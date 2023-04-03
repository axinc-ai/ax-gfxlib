// CoreRenderbuffer.cpp
// Renderbufferクラスの実装
#include "CoreRenderbuffer.h"
#include "CoreContext.h"
#include "CoreUtility.h"
#include "../backend/BackendRenderbuffer.h"

namespace axgl {

CoreRenderbuffer::CoreRenderbuffer()
{
	m_objectType = TYPE_RENDERBUFFER;
}

CoreRenderbuffer::~CoreRenderbuffer()
{
}

void CoreRenderbuffer::createStorage(CoreContext* context, GLenum internalformat, GLsizei width, GLsizei height)
{
	m_internalformat = internalformat;
	m_width = width;
	m_height = height;
	m_samples = 0;
	if (m_pBackendRenderbuffer != nullptr) {
		BackendContext* backend_context = context->getBackendContext();
		AXGL_ASSERT(backend_context != nullptr);
		bool result = m_pBackendRenderbuffer->createStorage(backend_context, internalformat, width, height);
		if (!result) {
			AXGL_DBGOUT("BackendRenderbuffer::createStorage() failed\n");
		}
	}
	return;
}

void CoreRenderbuffer::createStorageMultisample(CoreContext* context, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
	m_internalformat = internalformat;
	m_width = width;
	m_height = height;
	m_samples = samples;
	if (m_pBackendRenderbuffer != nullptr) {
		BackendContext* backend_context = context->getBackendContext();
		AXGL_ASSERT(backend_context != nullptr);
		bool result = m_pBackendRenderbuffer->createStorageMultisample(backend_context, samples, internalformat, width, height);
		if (!result) {
			AXGL_DBGOUT("BackendRenderbuffer::createStorageMultisample() failed\n");
		}
	}
	return;
}

void CoreRenderbuffer::updateStorageInformationFromBackend()
{
	if (m_pBackendRenderbuffer == nullptr) {
		return;
	}
	m_pBackendRenderbuffer->getStorageInformation(&m_internalformat, &m_width, &m_height, &m_samples);
	return;
}

void CoreRenderbuffer::getParameteriv(GLenum pname, GLint* params)
{
	AXGL_ASSERT(params != nullptr);
	switch (pname) {
	case GL_RENDERBUFFER_WIDTH:
		*params = m_width;
		break;
	case GL_RENDERBUFFER_HEIGHT:
		*params = m_height;
		break;
	case GL_RENDERBUFFER_INTERNAL_FORMAT:
		*params = m_internalformat;
		break;
	case GL_RENDERBUFFER_RED_SIZE:
		*params = getAlphaBitsFromFormat(m_internalformat);
		break;
	case GL_RENDERBUFFER_GREEN_SIZE:
		*params = getGreenBitsFromFormat(m_internalformat);
		break;
	case GL_RENDERBUFFER_BLUE_SIZE:
		*params = getBlueBitsFromFormat(m_internalformat);
		break;
	case GL_RENDERBUFFER_ALPHA_SIZE:
		*params = getAlphaBitsFromFormat(m_internalformat);
		break;
	case GL_RENDERBUFFER_DEPTH_SIZE:
		*params = getDepthBitsFromFormat(m_internalformat);
		break;
	case GL_RENDERBUFFER_STENCIL_SIZE:
		*params = getStencilBitsFromFormat(m_internalformat);
		break;
	case GL_RENDERBUFFER_SAMPLES:
		*params = m_samples;
		break;
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}

	return;
}

bool CoreRenderbuffer::initialize(CoreContext* context)
{
	AXGL_ASSERT(m_pBackendRenderbuffer == nullptr);
	m_pBackendRenderbuffer = BackendRenderbuffer::create();
	if (m_pBackendRenderbuffer == nullptr) {
		return false;
	}
	AXGL_ASSERT(context != nullptr);
	BackendContext* backend_context = context->getBackendContext();
	AXGL_ASSERT(backend_context != nullptr);
	return m_pBackendRenderbuffer->initialize(backend_context);
}

void CoreRenderbuffer::terminate(CoreContext* context)
{
	if (m_pBackendRenderbuffer != nullptr) {
		BackendContext* backend_context = nullptr;
		if (context != nullptr) {
			backend_context = context->getBackendContext();
		}
		m_pBackendRenderbuffer->terminate(backend_context);
		BackendRenderbuffer::destroy(m_pBackendRenderbuffer);
		m_pBackendRenderbuffer = nullptr;
	}
	return;
}

} // namespace axgl
