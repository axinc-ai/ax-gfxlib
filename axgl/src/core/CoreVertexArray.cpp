// CoreVertexArray.cpp
// VertexArrayクラスの実装
#include "CoreVertexArray.h"
#include "CoreBuffer.h"
#include "CoreUtility.h"
#include "CoreContext.h"
#include "../backend/BackendVertexArray.h"
#include "../backend/BackendContext.h"

namespace axgl {

CoreVertexArray::CoreVertexArray()
{
	m_objectType = TYPE_VERTEX_ARRAY;
}

CoreVertexArray::~CoreVertexArray()
{
}

void CoreVertexArray::setVertexAttribPointer(CoreContext* context, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer, CoreBuffer* buffer)
{
	if (index >= AXGL_MAX_VERTEX_ATTRIBS) {
		setErrorCode(GL_INVALID_VALUE);
		return;
	}
	m_vertexAttribs[index].m_size = size;
	m_vertexAttribs[index].m_type = type;
	m_vertexAttribs[index].m_normalized = normalized;
	m_vertexAttribs[index].m_stride = stride;
	m_vertexAttribs[index].m_pointer = pointer;
	if (m_vertexAttribs[index].m_pBuffer != nullptr) {
		m_vertexAttribs[index].m_pBuffer->release(context);
	}
	m_vertexAttribs[index].m_pBuffer = buffer;
	if (m_vertexAttribs[index].m_pBuffer != nullptr) {
		m_vertexAttribs[index].m_pBuffer->addRef();
	}
	if (m_pBackendVertexArray != nullptr) {
		BackendBuffer* backend_buffer = nullptr;
		if (buffer != nullptr) {
			backend_buffer = buffer->getBackendBuffer();
		}
		// NOTE: pointer is casted offset value
		m_pBackendVertexArray->setVertexAttrib(index, size, type, normalized, stride, (uint32_t)((intptr_t)pointer), backend_buffer);
	}
	return;
}

void CoreVertexArray::setVertexAttribIPointer(CoreContext* context, GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer, CoreBuffer* buffer)
{
	if (index >= AXGL_MAX_VERTEX_ATTRIBS) {
		setErrorCode(GL_INVALID_VALUE);
		return;
	}
	// TODO: integer type check
	m_vertexAttribs[index].m_size = size;
	m_vertexAttribs[index].m_type = type;
	m_vertexAttribs[index].m_normalized = GL_FALSE;
	m_vertexAttribs[index].m_stride = stride;
	m_vertexAttribs[index].m_pointer = pointer;
	if (m_vertexAttribs[index].m_pBuffer != nullptr) {
		m_vertexAttribs[index].m_pBuffer->release(context);
	}
	m_vertexAttribs[index].m_pBuffer = buffer;
	if (m_vertexAttribs[index].m_pBuffer != nullptr) {
		m_vertexAttribs[index].m_pBuffer->addRef();
	}
	if (m_pBackendVertexArray != nullptr) {
		BackendBuffer* backend_buffer = nullptr;
		if (buffer != nullptr) {
			backend_buffer = buffer->getBackendBuffer();
		}
		// NOTE: pointer is casted offset value
		m_pBackendVertexArray->setVertexAttrib(index, size, type, GL_FALSE, stride, (uint32_t)((intptr_t)pointer), backend_buffer);
	}
	return;
}

void CoreVertexArray::setEnableVertexAttrib(GLuint index, GLboolean enable)
{
	if (index >= AXGL_MAX_VERTEX_ATTRIBS) {
		setErrorCode(GL_INVALID_VALUE);
		return;
	}
	m_vertexAttribs[index].m_enable = enable;
	if (m_pBackendVertexArray != nullptr) {
		m_pBackendVertexArray->setEnable(index, (enable == GL_TRUE));
	}
	return;
}

void CoreVertexArray::setVertexAttribDivisor(GLuint index, GLuint divisor)
{
	if (index >= AXGL_MAX_VERTEX_ATTRIBS) {
		setErrorCode(GL_INVALID_VALUE);
		return;
	}
	m_vertexAttribs[index].m_divisor = divisor;
	if (m_pBackendVertexArray != nullptr) {
		m_pBackendVertexArray->setDivisor(index, divisor);
	}
	return;
}

void CoreVertexArray::setIndexBuffer(CoreContext* context, CoreBuffer* indexBuffer)
{
	if (m_pIndexBuffer != nullptr) {
		m_pIndexBuffer->release(context);
	}
	m_pIndexBuffer = indexBuffer;
	if (m_pIndexBuffer != nullptr) {
		m_pIndexBuffer->addRef();
	}
	if (m_pBackendVertexArray != nullptr) {
		BackendBuffer* backend_buffer = nullptr;
		if (indexBuffer != nullptr) {
			backend_buffer = indexBuffer->getBackendBuffer();
		}
		m_pBackendVertexArray->setIndexBuffer(backend_buffer);
	}
	return;
}

CoreBuffer* CoreVertexArray::getIndexBuffer() const
{
	return m_pIndexBuffer;
}

void CoreVertexArray::getVertexAttribfv(GLuint index, GLenum pname, GLfloat* params)
{
	if (index >= AXGL_MAX_VERTEX_ATTRIBS) {
		setErrorCode(GL_INVALID_VALUE);
		return;
	}
	const VertexAttrib& va = m_vertexAttribs[index];
	switch (pname) {
	case GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING:
		*params = (va.m_pBuffer != nullptr) ? static_cast<float>(va.m_pBuffer->getId()) : 0.0f;
		break;
	case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
		*params = static_cast<float>(va.m_enable);
		break;
	case GL_VERTEX_ATTRIB_ARRAY_SIZE:
		*params = static_cast<float>(va.m_size);
		break;
	case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
		*params = static_cast<float>(va.m_stride);
		break;
	case GL_VERTEX_ATTRIB_ARRAY_TYPE:
		*params = static_cast<float>(va.m_type);
		break;
	case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
		*params = static_cast<float>(va.m_normalized);
		break;
	case GL_VERTEX_ATTRIB_ARRAY_INTEGER:
		*params = static_cast<float>(isIntegerFromType(va.m_type));
		break;
	case GL_VERTEX_ATTRIB_ARRAY_DIVISOR:
		*params = static_cast<float>(va.m_divisor);
		break;
	// NOTE: GL_CURRENT_VERTEX_ATTRIB は VertexArrayObject に保持しない
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return;
}

void CoreVertexArray::getVertexAttribIiv(GLuint index, GLenum pname, GLint* params)
{
	if (index >= AXGL_MAX_VERTEX_ATTRIBS) {
		setErrorCode(GL_INVALID_VALUE);
		return;
	}
	const VertexAttrib& va = m_vertexAttribs[index];
	switch (pname) {
	case GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING:
		*params = (va.m_pBuffer != nullptr) ? va.m_pBuffer->getId() : 0;
		break;
	case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
		*params = va.m_enable;
		break;
	case GL_VERTEX_ATTRIB_ARRAY_SIZE:
		*params = va.m_size;
		break;
	case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
		*params = va.m_stride;
		break;
	case GL_VERTEX_ATTRIB_ARRAY_TYPE:
		*params = va.m_type;
		break;
	case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
		*params = va.m_normalized;
		break;
	case GL_VERTEX_ATTRIB_ARRAY_INTEGER:
		*params = isIntegerFromType(va.m_type);
		break;
	case GL_VERTEX_ATTRIB_ARRAY_DIVISOR:
		*params = va.m_divisor;
		break;
	// NOTE: GL_CURRENT_VERTEX_ATTRIB は VertexArrayObject に保持しない
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return;
}

void CoreVertexArray::getVertexAttribPointerv(GLuint index, GLenum pname, void** pointer)
{
	if (index >= AXGL_MAX_VERTEX_ATTRIBS) {
		setErrorCode(GL_INVALID_VALUE);
		return;
	}
	if (pname == GL_VERTEX_ATTRIB_ARRAY_POINTER) {
		*pointer = const_cast<void*>(m_vertexAttribs[index].m_pointer);
	} else {
		setErrorCode(GL_INVALID_ENUM);
	}
	return;
}

void CoreVertexArray::getVertexAttribIuiv(GLuint index, GLenum pname, GLuint* params)
{
	if (index >= AXGL_MAX_VERTEX_ATTRIBS) {
		setErrorCode(GL_INVALID_VALUE);
		return;
	}
	const VertexAttrib& va = m_vertexAttribs[index];
	switch (pname) {
	case GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING:
		*params = (va.m_pBuffer != nullptr) ? va.m_pBuffer->getId() : 0;
		break;
	case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
		*params = va.m_enable;
		break;
	case GL_VERTEX_ATTRIB_ARRAY_SIZE:
		*params = va.m_size;
		break;
	case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
		*params = va.m_stride;
		break;
	case GL_VERTEX_ATTRIB_ARRAY_TYPE:
		*params = va.m_type;
		break;
	case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
		*params = va.m_normalized;
		break;
	case GL_VERTEX_ATTRIB_ARRAY_INTEGER:
		*params = isIntegerFromType(va.m_type);
		break;
	case GL_VERTEX_ATTRIB_ARRAY_DIVISOR:
		*params = va.m_divisor;
		break;
	// NOTE: GL_CURRENT_VERTEX_ATTRIB は VertexArrayObject に保持しない
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return;
}

bool CoreVertexArray::initialize(CoreContext* context)
{
	AXGL_ASSERT(m_pBackendVertexArray == nullptr);
	m_pBackendVertexArray = BackendVertexArray::create();
	if (m_pBackendVertexArray == nullptr) {
		return false;
	}
	AXGL_ASSERT(context != nullptr);
	BackendContext* backend_context = context->getBackendContext();
	AXGL_ASSERT(backend_context != nullptr);
	return m_pBackendVertexArray->initialize(backend_context);
}

void CoreVertexArray::terminate(CoreContext* context)
{
	for (int32_t i = 0; i < AXGL_MAX_VERTEX_ATTRIBS; i++) {
		if (m_vertexAttribs[i].m_pBuffer != nullptr) {
			m_vertexAttribs[i].m_pBuffer->release(context);
			m_vertexAttribs[i].m_pBuffer = nullptr;
		}
	}
	if (m_pIndexBuffer != nullptr) {
		m_pIndexBuffer->release(context);
		m_pIndexBuffer = nullptr;
	}
	if (m_pBackendVertexArray != nullptr) {
		BackendContext* backend_context = nullptr;
		if (context != nullptr) {
			backend_context = context->getBackendContext();
		}
		if (backend_context != nullptr) {
			// 関連するキャッシュを破棄
			backend_context->discardCachesAssociatedWithVertexArray(m_pBackendVertexArray);
		}
		m_pBackendVertexArray->terminate(backend_context);
		BackendVertexArray::destroy(m_pBackendVertexArray);
		m_pBackendVertexArray = nullptr;
	}
	return;
}

} // namespace axgl
