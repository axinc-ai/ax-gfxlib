// VertexArrayMetal.mm
#include "VertexArrayMetal.h"
#include "ContextMetal.h"
#include "BufferMetal.h"
#include "../../AXGLAllocatorImpl.h"

namespace axgl {

// BackendVertexArrayクラスの実装 --------
BackendVertexArray* BackendVertexArray::create()
{
	VertexArrayMetal* vertex_array = AXGL_NEW(VertexArrayMetal);
	return vertex_array;
}
	
void BackendVertexArray::destroy(BackendVertexArray* vertexArray)
{
	if (vertexArray == nullptr) {
		return;
	}
	AXGL_DELETE(vertexArray);
	return;
}

// VertexArrayMetalクラスの実装 --------
VertexArrayMetal::VertexArrayMetal()
{
}

VertexArrayMetal::~VertexArrayMetal()
{
}

bool VertexArrayMetal::initialize(BackendContext* context)
{
	AXGL_UNUSED(context);
	return true;
}

bool VertexArrayMetal::terminate(BackendContext* context)
{
	AXGL_UNUSED(context);
	return true;
}

void VertexArrayMetal::setEnable(uint32_t index, bool enable)
{
	if (index >= AXGL_MAX_VERTEX_ATTRIBS) {
		return;
	}
	m_attribParams[index].enabled = enable;
	return;
}

void VertexArrayMetal::setVertexAttrib(uint32_t index, int32_t size, int32_t type, int32_t normalized, uint32_t stride, uint32_t offset, BackendBuffer* buffer)
{
	if (index >= AXGL_MAX_VERTEX_ATTRIBS) {
		return;
	}
	AttribParamMetal* dst = &m_attribParams[index];
	MTLVertexFormat format_metal = convert_vertex_format(type, size, (normalized == GL_TRUE));
	dst->format = format_metal;
	dst->offset = offset;
	if (stride == 0) {
		stride = get_size_from_gltype(type) * size;
	}
	dst->stride = stride;
	dst->buffer = static_cast<BufferMetal*>(buffer);
	m_vertexBufferDirty = true;
	return;
}

void VertexArrayMetal::setDivisor(uint32_t index, uint32_t divisor)
{
	if (index >= AXGL_MAX_VERTEX_ATTRIBS) {
		return;
	}
	m_attribParams[index].divisor = divisor;
	return;
}

void VertexArrayMetal::setIndexBuffer(BackendBuffer* buffer)
{
	m_indexBuffer = static_cast<BufferMetal*>(buffer);
	return;
}

bool VertexArrayMetal::updateDirtyBuffers(ContextMetal* context)
{
	bool result = true;
	for (int i = 0; i < AXGL_MAX_VERTEX_ATTRIBS; i++) {
		if (m_attribParams[i].buffer != nullptr) {
			if (!m_attribParams[i].buffer->setupBufferInDraw(context)) {
				result = false;
			}
		}
	}
	return result;
}

const VertexArrayMetal::AttribParamMetal* VertexArrayMetal::getAttribParams() const
{
	return m_attribParams;
}

bool VertexArrayMetal::isVertexBufferDirty() const
{
	return m_vertexBufferDirty;
}

void VertexArrayMetal::clearVertexBufferDirty()
{
	m_vertexBufferDirty = false;
	return;
}

BufferMetal* VertexArrayMetal::getIndexBuffer() const
{
	return m_indexBuffer;
}

} // namespace axgl
