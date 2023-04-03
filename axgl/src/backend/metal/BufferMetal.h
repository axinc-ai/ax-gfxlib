// BufferMetal.h
#ifndef __BufferMetal_h_
#define __BufferMetal_h_
#include "BackendMetal.h"
#include "../BackendBuffer.h"
#include "../../common/MemoryBuffer.h"

namespace axgl {

class ContextMetal;

class BufferMetal : public BackendBuffer
{
public:
	BufferMetal();
	virtual ~BufferMetal();
	virtual bool initialize(BackendContext* context) override;
	virtual void terminate(BackendContext* context) override;
	virtual bool setData(BackendContext* context, GLsizeiptr size, const void* data, GLenum usage) override;
	virtual bool setSubData(BackendContext* context, GLintptr offset, GLsizeiptr size, const void* data) override;
	virtual bool mapRange(BackendContext* context, GLintptr offset, GLsizeiptr length, GLenum access, void** mapPointer) override;
	virtual bool unmap(BackendContext* context) override;
	virtual bool flushMappedRange(BackendContext* context, GLintptr offset, GLsizeiptr length) override;

public:
	enum {
		NoUpdateRequired = 0,
		UpdateRequiredWithBlitCommand = 1,
		UpdateRequiredWithoutBlitCommand = 2
	};

public:
	id<MTLBuffer> getMtlBuffer() const;
	void setU8U16ConversionMode();
	bool setupBufferInDraw(BackendContext* context,
		ConversionMode conversion = ConversionModeNone, intptr_t offset = 0, intptr_t size = 0);
	bool needUpdateWithStrideConversion(uint32_t stride, uint32_t convertedStride) const;
	bool needUpdateWithStrideAndDataConversion(uint32_t stride, uint32_t convertedStride, ConversionMode conversion, GLint first, GLsizei count) const;
	bool needUpdateWithoutConversion() const;
	int needUpdateWithIndexConversion(ConversionMode conversion, intptr_t iboOffset, intptr_t iboSize, bool isUbyte) const;
	bool setupBufferWithStrideConversion(ContextMetal* context, uint32_t stride, uint32_t convertedStride);
	bool setupWithStrideAndDataConversion(ContextMetal* context,
		uint32_t stride, uint32_t convertedStride, ConversionMode conversion, GLint first, GLsizei count);
	bool isMTLBufferDirty() const;
	void clearMTLBufferDirty();
	bool isDynamicBuffer() const;
	size_t getBufferDataSize() const;
	void copyToDynamicBuffer(id<MTLBuffer> dynamicBuffer, size_t offset) const;

private:
	bool setupMTLBuffer(ContextMetal* context, size_t size, const uint8_t* data);
	bool setupShadowBuffer(size_t size, const uint8_t* data);
	bool setupShadowBufferForReserved();
	bool setupWithDataConversion(ContextMetal* context,
		ConversionMode conversion, intptr_t offset, intptr_t size);
	bool convertTriFanIndices8(ContextMetal* context, intptr_t offset, intptr_t size);
	bool convertTriFanIndices16(ContextMetal* context, intptr_t offset, intptr_t size);
	bool convertTriFanIndices32(ContextMetal* context, intptr_t offset, intptr_t size);
	

private:
	enum {
		SHADOW_BUFFER_STATE_INITIAL = 0,
		SHADOW_BUFFER_STATE_RESERVED = 1,
		SHADOW_BUFFER_STATE_CREATED = 2
	};
	MemoryBuffer m_shadowBuffer;
	uint32_t m_mapAccessFlags = 0;
	intptr_t m_mapOffset = 0;
	intptr_t m_mapLength = 0;
	id<MTLBuffer> m_srcBuffer = nil;
	id<MTLBuffer> m_mtlBuffer = nil;
	bool m_mtlBufferDirty = false;
	intptr_t m_dirtyStart = 0;
	intptr_t m_dirtyEnd = 0;
	bool m_u8u16ConversionMode = false;
	intptr_t m_setDataSize = 0;
	uint32_t m_convertedStride = UINT32_MAX;
	ConversionMode m_convertedMode = ConversionModeNone;
	intptr_t m_convertedOffset = 0;
	intptr_t m_convertedSize = 0;
	GLint m_convertedFirst = 0;
	GLsizei m_convertedCount = 0;
	int m_shadowBufferState = SHADOW_BUFFER_STATE_INITIAL;
	GLenum m_usage = 0;
};

} // namespace axgl

#endif // __BufferMetal_h_
