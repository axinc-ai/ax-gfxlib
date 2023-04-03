// CoreSync.cpp
// Syncクラスの実装
#include "CoreSync.h"
#include "CoreContext.h"
#include "../backend/BackendSync.h"

namespace axgl {

CoreSync::CoreSync()
{
	m_objectType = TYPE_SYNC;
}

CoreSync::CoreSync(GLenum condition, GLbitfield flags)
{
	m_objectType = TYPE_SYNC;
	m_condition = condition;
	m_flags = flags;
}

CoreSync::~CoreSync()
{
}

void CoreSync::getSynciv(GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values)
{
	GLsizei len = 0;
	switch (pname) {
	case GL_OBJECT_TYPE:
		if (bufSize >= 1) {
			*values = GL_SYNC_FENCE;
			len = 1;
		}
		break;
	case GL_SYNC_STATUS:
		if (bufSize >= 1) {
			if (m_pBackendSync != nullptr) {
				*values = m_pBackendSync->getStatus();
			} else {
				*values = GL_UNSIGNALED;
			}
			len = 1;
		}
		break;
	case GL_SYNC_CONDITION:
		if (bufSize >= 1) {
			*values = m_condition;
			len = 1;
		}
		break;
	case GL_SYNC_FLAGS:
		if (bufSize >= 1) {
			*values = m_flags;
			len = 1;
		}
		break;
	default:
		// GL_INVALID_ENUM
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	if (length != nullptr) {
		*length = len;
	}
	return;
}

bool CoreSync::initialize(CoreContext* context)
{
	AXGL_ASSERT(m_pBackendSync == nullptr);
	m_pBackendSync = BackendSync::create();
	if (m_pBackendSync == nullptr) {
		return false;
	}
	AXGL_ASSERT(context != nullptr);
	BackendContext* backend_context = context->getBackendContext();
	AXGL_ASSERT(backend_context != nullptr);
	return m_pBackendSync->initialize(backend_context);
}

void CoreSync::terminate(CoreContext* context)
{
	if (m_pBackendSync != nullptr) {
		BackendContext* backend_context = nullptr;
		if (context != nullptr) {
			backend_context = context->getBackendContext();
		}
		m_pBackendSync->terminate(backend_context);

		BackendSync::destroy(m_pBackendSync);
		m_pBackendSync = nullptr;
	}
	return;
}

} // namespace axgl
