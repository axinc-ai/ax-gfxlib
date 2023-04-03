// VertexArrayMetal.h
#ifndef __VertexArrayMetal_h_
#define __VertexArrayMetal_h_
#include "BackendMetal.h"
#include "../BackendVertexArray.h"

namespace axgl {

class ContextMetal;
class BufferMetal;

// Vertex array objectクラス
class VertexArrayMetal : public BackendVertexArray
{
public:
	// Coreインタフェース
	VertexArrayMetal();
	virtual ~VertexArrayMetal();
	virtual bool initialize(BackendContext* context) override;
	virtual bool terminate(BackendContext* context) override;
	virtual void setEnable(uint32_t index, bool enable) override;
	virtual void setVertexAttrib(uint32_t index, int32_t size, int32_t type, int32_t normalized, uint32_t stride, uint32_t offset, BackendBuffer* buffer) override;
	virtual void setDivisor(uint32_t index, uint32_t divisor) override;
	virtual void setIndexBuffer(BackendBuffer* buffer) override;

public:
	// Attributeパラメータ構造体
	struct AttribParamMetal {
		MTLVertexFormat format = MTLVertexFormatInvalid;
		uint32_t offset = 0;
		uint32_t stride = 0;
		uint32_t divisor = 0;
		BufferMetal* buffer = nullptr;
		bool enabled = false;
	};

public:
	// Metalバックエンドインタフェース
	bool updateDirtyBuffers(ContextMetal* context);
	const AttribParamMetal* getAttribParams() const;
	bool isVertexBufferDirty() const;
	void clearVertexBufferDirty();
	BufferMetal* getIndexBuffer() const;

private:
	AttribParamMetal m_attribParams[AXGL_MAX_VERTEX_ATTRIBS];
	BufferMetal* m_indexBuffer = nullptr;
	bool m_vertexBufferDirty = false;
};

} // namespace axgl

#endif // __VertexArrayMetal_h_

