// CoreQuery.cpp
// Queryクラスの実装
#include "CoreQuery.h"
#include "CoreContext.h"
#include "../backend/BackendQuery.h"

namespace axgl {

CoreQuery::CoreQuery()
{
	m_objectType = TYPE_QUERY;
}

CoreQuery::~CoreQuery()
{
}

void CoreQuery::begin(CoreContext* context, GLenum target)
{
	if ((context == nullptr) || (m_pBackendQuery == nullptr)) {
		return;
	}
	m_target = target;
	BackendContext* backend_context = context->getBackendContext();
	AXGL_ASSERT(backend_context != nullptr);
	bool result = m_pBackendQuery->begin(backend_context);
	if (!result) {
		AXGL_DBGOUT("BackendQuery::begin() failed\n");
	}
	return;
}

void CoreQuery::end(CoreContext* context, GLenum target)
{
	if ((context == nullptr) || (m_pBackendQuery == nullptr)) {
		return;
	}
	AXGL_ASSERT(m_target == target);
	BackendContext* backend_context = context->getBackendContext();
	AXGL_ASSERT(backend_context != nullptr);
	bool result = m_pBackendQuery->end(backend_context);
	if (!result) {
		AXGL_DBGOUT("BackendQuery::end() failed\n");
	}
	return;
}

void CoreQuery::getQueryObjectuiv(GLenum pname, GLuint* params)
{
	if (m_pBackendQuery == nullptr) {
		// internal error
		return;
	}
	AXGL_ASSERT(params != nullptr);
	m_pBackendQuery->getQueryuiv(pname, params);
	return;
}

bool CoreQuery::initialize(CoreContext* context)
{
	AXGL_ASSERT(m_pBackendQuery == nullptr);
	m_pBackendQuery = BackendQuery::create();
	if (m_pBackendQuery == nullptr) {
		return false;
	}
	AXGL_ASSERT(context != nullptr);
	BackendContext* backend_context = context->getBackendContext();
	AXGL_ASSERT(backend_context != nullptr);
	return m_pBackendQuery->initialize(backend_context);
}

void CoreQuery::terminate(CoreContext* context)
{
	if (m_pBackendQuery != nullptr) {
		BackendContext* backend_context = nullptr;
		if (context != nullptr) {
			backend_context = context->getBackendContext();
		}
		m_pBackendQuery->terminate(backend_context);
		BackendQuery::destroy(m_pBackendQuery);
		m_pBackendQuery = nullptr;
	}
	return;
}

} // namespace axgl
