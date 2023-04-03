// CoreSampler.cpp
// Samplerクラスの実装
#include "CoreSampler.h"
#include "CoreContext.h"

namespace axgl {

CoreSampler::CoreSampler()
{
	m_objectType = TYPE_SAMPLER;
}

CoreSampler::~CoreSampler()
{
}

void CoreSampler::terminate(CoreContext* context)
{
	if (m_pBackendSampler != nullptr) {
		BackendContext* backend_context = nullptr;
		if (context != nullptr) {
			backend_context = context->getBackendContext();
		}
		m_pBackendSampler->terminate(backend_context);
		BackendSampler::destroy(m_pBackendSampler);
		m_pBackendSampler = nullptr;
	}
	return;
}

void CoreSampler::setSamplerParameteri(CoreContext* context, GLenum pname, GLint param)
{
	// TODO: param check
	switch (pname) {
	case GL_TEXTURE_MAG_FILTER:
		m_samplerParameters.magFilter = param;
		break;
	case GL_TEXTURE_MIN_FILTER:
		m_samplerParameters.minFilter = param;
		break;
	case GL_TEXTURE_MIN_LOD:
		m_samplerParameters.minLod = static_cast<float>(param);
		break;
	case GL_TEXTURE_MAX_LOD:
		m_samplerParameters.maxLod = static_cast<float>(param);
		break;
	case GL_TEXTURE_WRAP_S:
		m_samplerParameters.wrapS = param;
		break;
	case GL_TEXTURE_WRAP_T:
		m_samplerParameters.wrapT = param;
		break;
	case GL_TEXTURE_WRAP_R:
		m_samplerParameters.wrapR = param;
		break;
	case GL_TEXTURE_COMPARE_MODE:
		m_samplerParameters.compareMode = param;
		break;
	case GL_TEXTURE_COMPARE_FUNC:
		m_samplerParameters.compareFunc = param;
		break;
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	m_dirty = true;
	return;
}

void CoreSampler::setSamplerParameteriv(CoreContext* context, GLenum pname, const GLint* param)
{
	setSamplerParameteri(context, pname, *param);
	return;
}

void CoreSampler::setSamplerParameterf(CoreContext* context, GLenum pname, GLfloat param)
{
	// TODO: param check
	switch (pname) {
	case GL_TEXTURE_MAG_FILTER:
		m_samplerParameters.magFilter = static_cast<GLint>(param);
		break;
	case GL_TEXTURE_MIN_FILTER:
		m_samplerParameters.minFilter = static_cast<GLint>(param);
		break;
	case GL_TEXTURE_MIN_LOD:
		m_samplerParameters.minLod = param;
		break;
	case GL_TEXTURE_MAX_LOD:
		m_samplerParameters.maxLod = param;
		break;
	case GL_TEXTURE_WRAP_S:
		m_samplerParameters.wrapS = static_cast<GLint>(param);
		break;
	case GL_TEXTURE_WRAP_T:
		m_samplerParameters.wrapT = static_cast<GLint>(param);
		break;
	case GL_TEXTURE_WRAP_R:
		m_samplerParameters.wrapR = static_cast<GLint>(param);
		break;
	case GL_TEXTURE_COMPARE_MODE:
		m_samplerParameters.compareMode = static_cast<GLint>(param);
		break;
	case GL_TEXTURE_COMPARE_FUNC:
		m_samplerParameters.compareFunc = static_cast<GLint>(param);
		break;
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	m_dirty = true;
	return;
}

void CoreSampler::setSamplerParameterfv(CoreContext* context, GLenum pname, const GLfloat* param)
{
	setSamplerParameterf(context, pname, *param);
	return;
}

void CoreSampler::getSamplerParameteriv(CoreContext* context, GLenum pname, GLint* params)
{
	AXGL_ASSERT(params != nullptr);
	switch (pname) {
	case GL_TEXTURE_MAG_FILTER:
		*params = m_samplerParameters.magFilter;
		break;
	case GL_TEXTURE_MIN_FILTER:
		*params = m_samplerParameters.minFilter;
		break;
	case GL_TEXTURE_MIN_LOD:
		*params = static_cast<GLint>(m_samplerParameters.minLod);
		break;
	case GL_TEXTURE_MAX_LOD:
		*params = static_cast<GLint>(m_samplerParameters.maxLod);
		break;
	case GL_TEXTURE_WRAP_S:
		*params = m_samplerParameters.wrapS;
		break;
	case GL_TEXTURE_WRAP_T:
		*params = m_samplerParameters.wrapT;
		break;
	case GL_TEXTURE_WRAP_R:
		*params = m_samplerParameters.wrapR;
		break;
	case GL_TEXTURE_COMPARE_MODE:
		*params = m_samplerParameters.compareMode;
		break;
	case GL_TEXTURE_COMPARE_FUNC:
		*params = m_samplerParameters.compareFunc;
		break;
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return;
}

void CoreSampler::getSamplerParameterfv(CoreContext* context, GLenum pname, GLfloat* params)
{
	AXGL_ASSERT(params != nullptr);
	switch (pname) {
	case GL_TEXTURE_MAG_FILTER:
		*params = static_cast<GLfloat>(m_samplerParameters.magFilter);
		break;
	case GL_TEXTURE_MIN_FILTER:
		*params = static_cast<GLfloat>(m_samplerParameters.minFilter);
		break;
	case GL_TEXTURE_MIN_LOD:
		*params = m_samplerParameters.minLod;
		break;
	case GL_TEXTURE_MAX_LOD:
		*params = m_samplerParameters.maxLod;
		break;
	case GL_TEXTURE_WRAP_S:
		*params = static_cast<GLfloat>(m_samplerParameters.wrapS);
		break;
	case GL_TEXTURE_WRAP_T:
		*params = static_cast<GLfloat>(m_samplerParameters.wrapT);
		break;
	case GL_TEXTURE_WRAP_R:
		*params = static_cast<GLfloat>(m_samplerParameters.wrapR);
		break;
	case GL_TEXTURE_COMPARE_MODE:
		*params = static_cast<GLfloat>(m_samplerParameters.compareMode);
		break;
	case GL_TEXTURE_COMPARE_FUNC:
		*params = static_cast<GLfloat>(m_samplerParameters.compareFunc);
		break;
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return;
}

bool CoreSampler::setup(CoreContext* context)
{
	if (m_pBackendSampler == nullptr) {
		m_pBackendSampler = BackendSampler::create();
		if (m_pBackendSampler == nullptr) {
			AXGL_DBGOUT("BackendSampler::create() failed\n");
			return false;
		}
		BackendContext* backend_context = context->getBackendContext();
		if (!m_pBackendSampler->initialize(backend_context)) {
			AXGL_DBGOUT("BaskendSampler::initialize() failed\n");
		}
		m_dirty = true;
	}
	bool result = true;
	if (m_dirty) {
		AXGL_ASSERT(context != nullptr);
		result = m_pBackendSampler->setupSampler(context->getBackendContext(), m_samplerParameters);
		m_dirty = false;
	}
	return result;
}

} // namespace axgl
