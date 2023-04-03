// BackendBuffer.h
#ifndef __BackendBuffer_h_
#define __BackendBuffer_h_

#include "../common/axglCommon.h"

namespace axgl {

class BackendContext;

class BackendBuffer
{
public:
	enum ConversionMode {
		ConversionModeNone = 0,
		ConversionModeTriFanIndices8 = 1,
		ConversionModeTriFanIndices16 = 2,
		ConversionModeTriFanIndices32 = 3,
		ConversionModeTriFanVertices = 4
	};

public:
	virtual ~BackendBuffer() {}
	virtual bool initialize(BackendContext* context) = 0;
	virtual void terminate(BackendContext* context) = 0;
	virtual bool setData(BackendContext* context, GLsizeiptr size, const void* data, GLenum usage) = 0;
	virtual bool setSubData(BackendContext* context, GLintptr offset, GLsizeiptr size, const void* data) = 0;
	virtual bool mapRange(BackendContext* context, GLintptr offset, GLsizeiptr length, GLenum access, void** mapPointer) = 0;
	virtual bool unmap(BackendContext* context) = 0;
	virtual bool flushMappedRange(BackendContext* context, GLintptr offset, GLsizeiptr length) = 0;

public:
	static BackendBuffer* create();
	static void destroy(BackendBuffer* buffer);
};

} // namespace axgl

#endif // __BackendBuffer_h_
