// CoreBuffer.cpp
// Bufferクラスの実装

#include "CoreBuffer.h"
#include "CoreContext.h"
#include "../backend/BackendBuffer.h"

namespace axgl {

// コンストラクタ
CoreBuffer::CoreBuffer()
{
	m_objectType = TYPE_BUFFER;
}

// デストラクタ
CoreBuffer::~CoreBuffer()
{
}

// ターゲットを設定
void CoreBuffer::setTarget(GLenum target)
{
	m_target = target;
	return;
}

// ターゲットを取得
GLenum CoreBuffer::getTarget() const
{
	return m_target;
}

// バッファにデータを設定
void CoreBuffer::setData(CoreContext* context, GLsizeiptr size, const void* data, GLenum usage)
{
	if ((m_pBackendBuffer == nullptr) || (context == nullptr)) {
		return;
	}
	BackendContext* backend_context = context->getBackendContext();
	AXGL_ASSERT(backend_context != nullptr);
	if (!m_pBackendBuffer->setData(backend_context, size, data, usage)) {
		// internal error
		AXGL_DBGOUT("BackendBuffer::setData() failed\n");
	}
	return;
}

// バッファにサブデータを設定
void CoreBuffer::setSubData(CoreContext* context, GLintptr offset, GLsizeiptr size, const void* data)
{
	if ((m_pBackendBuffer == nullptr) || (context == nullptr)) {
		return;
	}
	BackendContext* backend_context = context->getBackendContext();
	AXGL_ASSERT(backend_context != nullptr);
	if (!m_pBackendBuffer->setSubData(backend_context, offset, size, data)) {
		// internal error
		AXGL_DBGOUT("BackendBuffer::setSubData() failed\n");
	}
	return;
}

// 領域を指定して、バッファをメモリにマップ
void* CoreBuffer::mapBufferRange(CoreContext* context, GLintptr offset, GLsizeiptr size, GLbitfield access)
{
	if ((m_pBackendBuffer == nullptr) || (context == nullptr)) {
		return nullptr;
	}
	BackendContext* backend_context = context->getBackendContext();
	AXGL_ASSERT(backend_context != nullptr);
	if (!m_pBackendBuffer->mapRange(backend_context, offset, size, access, &m_pMapPointer)) {
		// internal error
		AXGL_DBGOUT("BackendBuffer::mapRange() failed\n");
		m_pMapPointer = nullptr;
	} else {
		m_accessFlags = access;
	}
	return m_pMapPointer;
}

// バッファをアンマップ
GLboolean CoreBuffer::unmapBuffer(CoreContext* context)
{
	if ((m_pBackendBuffer == nullptr) || (context == nullptr)) {
		return GL_FALSE;
	}
	BackendContext* backend_context = context->getBackendContext();
	AXGL_ASSERT(backend_context != nullptr);
	GLboolean result = GL_TRUE;
	if (!m_pBackendBuffer->unmap(backend_context)) {
		AXGL_DBGOUT("BackendBuffer::unmap() failed\n");
		result = GL_FALSE;
	}
	m_pMapPointer = nullptr;
	return result;
}

// ポインタを取得
void CoreBuffer::getBufferPointerv(CoreContext* context, GLenum pname, void** params)
{
	AXGL_UNUSED(context);
	switch (pname) {
	case GL_BUFFER_MAP_POINTER:
		*params = m_pMapPointer;
		break;
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return;
}

// 領域を指定して、バッファをflush
void CoreBuffer::flushMappedBufferRange(CoreContext* context, GLintptr offset, GLsizeiptr length)
{
	if ((m_pBackendBuffer == nullptr) || (context == nullptr)) {
		return;
	}
	if ((m_pMapPointer == nullptr) || ((m_accessFlags & GL_MAP_FLUSH_EXPLICIT_BIT) == 0)) {
		// マップされていない、もしくは明示的なFLUSHを指定していない
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	BackendContext* backend_context = context->getBackendContext();
	AXGL_ASSERT(backend_context != nullptr);
	if (!m_pBackendBuffer->flushMappedRange(backend_context, offset, length)) {
		AXGL_DBGOUT("BackendBuffer::flushMappedRange() failed\n");
	}
	return;
}

// バッファのサブデータをコピー
void CoreBuffer::copyBufferSubData(CoreContext* context, CoreBuffer* src, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
	AXGL_UNUSED(context);
	AXGL_UNUSED(src);
	AXGL_UNUSED(readOffset);
	AXGL_UNUSED(writeOffset);
	AXGL_UNUSED(size);
	return;
}

// バッファのパラメータを取得(int32)
void CoreBuffer::getBufferParameteriv(CoreContext* context, GLenum pname, GLint* params)
{
	if (params == nullptr) {
		return;
	}
	AXGL_UNUSED(context);
	switch (pname) {
	case GL_BUFFER_ACCESS_FLAGS:
		*params = m_accessFlags;
		break;
	case GL_BUFFER_MAPPED:
		*params = m_mapped ? GL_TRUE : GL_FALSE;
		break;
	case GL_BUFFER_SIZE:
		*params = static_cast<GLint>(m_size);
		break;
	case GL_BUFFER_USAGE:
		*params = m_usage;
		break;
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return;
}

// バッファのパラメータを取得(int64)
void CoreBuffer::getBufferParameteri64v(CoreContext* context, GLenum pname, GLint64* params)
{
	if (params == nullptr) {
		return;
	}
	AXGL_UNUSED(context);
	switch (pname) {
	case GL_BUFFER_MAP_LENGTH:
		*params = static_cast<GLint64>(m_mapLength);
		break;
	case GL_BUFFER_MAP_OFFSET:
		*params = static_cast<GLint64>(m_mapOffset);
		break;
	case GL_BUFFER_SIZE:
		*params = static_cast<GLint64>(m_size);
		break;
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return;
}

// 初期化
bool CoreBuffer::initialize(CoreContext* context)
{
	if (context == nullptr) {
		return false;
	}
	AXGL_ASSERT(m_pBackendBuffer == nullptr);
	m_pBackendBuffer = BackendBuffer::create();
	if (m_pBackendBuffer == nullptr) {
		return false;
	}
	BackendContext* backend_context = context->getBackendContext();
	AXGL_ASSERT(backend_context != nullptr);
	return m_pBackendBuffer->initialize(backend_context);
}

// 終了
void CoreBuffer::terminate(CoreContext* context)
{
	if (m_pBackendBuffer != nullptr) {
		BackendContext* backend_context = nullptr;
		if (context != nullptr) {
			backend_context = context->getBackendContext();
		}
		m_pBackendBuffer->terminate(backend_context);
		BackendBuffer::destroy(m_pBackendBuffer);
		m_pBackendBuffer = nullptr;
	}
	return;
}

} // namespace axgl
