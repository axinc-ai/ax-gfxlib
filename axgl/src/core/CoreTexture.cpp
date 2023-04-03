// CoreTexture.cpp
// Textureクラスの実装
#include "CoreTexture.h"
#include "CoreContext.h"

namespace axgl {

CoreTexture::CoreTexture()
{
	m_objectType = TYPE_TEXTURE;
}

CoreTexture::~CoreTexture()
{
}

void CoreTexture::terminate(CoreContext* context)
{
	BackendContext* backend_context = nullptr;
	if (context != nullptr) {
		backend_context = context->getBackendContext();
	}
	if (m_pBackendTexture != nullptr) {
		m_pBackendTexture->terminate(backend_context);
		BackendTexture::destroy(m_pBackendTexture);
		m_pBackendTexture = nullptr;
	}
	if (m_pBackendSampler != nullptr) {
		m_pBackendSampler->terminate(backend_context);
		BackendSampler::destroy(m_pBackendSampler);
		m_pBackendSampler = nullptr;
	}
	m_samplerDirty = true;
	return;
}

void CoreTexture::setTarget(GLenum target)
{
	if (m_target == TARGET_UNKNOWN) {
		// NOTE: 最初のglBindTextureで確定
		m_target = target;
	}
	return;
}

GLenum CoreTexture::getTarget() const
{
	return m_target;
}

void CoreTexture::compressedTexImage2d(CoreContext* context, GLenum target, GLint level, GLenum internalformat,
	GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data)
{
	AXGL_ASSERT(context != nullptr);
	if (!setupBackendTexture(context)) {
		AXGL_DBGOUT("CoreTexture::setupBackendTexture() failed\n");
		return;
	}
	AXGL_UNUSED(border);
	BackendContext* backend_context = context->getBackendContext();
	bool result = false;
	AXGL_ASSERT(m_pBackendTexture != nullptr);
	if (m_target == GL_TEXTURE_CUBE_MAP) {
		result = m_pBackendTexture->setCompressedImageCube(backend_context, target, level, internalformat,
			width, height, imageSize, data, &m_textureParameters);
	} else if(m_target == GL_TEXTURE_2D) {
		result = m_pBackendTexture->setCompressedImage2D(backend_context, level, internalformat,
			width, height, imageSize, data, &m_textureParameters);
	} else {
		AXGL_ASSERT(0);
	}
	m_internalformat = internalformat;
	if (!result) {
		AXGL_DBGOUT("BackendTexture::setCompressedImage*() failed\n");
	}
	return;
}

void CoreTexture::compressedTexSubImage2d(CoreContext* context, GLenum target, GLint level, GLint xoffset, GLint yoffset,
	GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data)
{
	// TODO:
	AXGL_UNUSED(context);
	AXGL_UNUSED(target);
	AXGL_UNUSED(level);
	AXGL_UNUSED(xoffset);
	AXGL_UNUSED(yoffset);
	AXGL_UNUSED(width);
	AXGL_UNUSED(height);
	AXGL_UNUSED(format);
	AXGL_UNUSED(imageSize);
	AXGL_UNUSED(data);
	AXGL_DBGOUT("CoreTexture::compressedTexSubImage2d() called\n");
	return;
}

void CoreTexture::copyTexImage2d(GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
	// TODO:
	AXGL_UNUSED(level);
	AXGL_UNUSED(internalformat);
	AXGL_UNUSED(x);
	AXGL_UNUSED(y);
	AXGL_UNUSED(width);
	AXGL_UNUSED(height);
	AXGL_UNUSED(border);
	AXGL_DBGOUT("CoreTexture::copyTexImage2d() called\n");
	return;
}

void CoreTexture::copyTexSubImage2d(GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	// TODO:
	AXGL_UNUSED(level);
	AXGL_UNUSED(xoffset);
	AXGL_UNUSED(yoffset);
	AXGL_UNUSED(x);
	AXGL_UNUSED(y);
	AXGL_UNUSED(width);
	AXGL_UNUSED(height);
	AXGL_DBGOUT("CoreTexture::copyTexSubImage2d() called\n");
	return;
}

void CoreTexture::texImage2d(CoreContext* context, GLenum target, GLint level, GLint internalformat,
	GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels)
{
	AXGL_ASSERT(context != nullptr);
	if (!setupBackendTexture(context)) {
		AXGL_DBGOUT("CoreTexture::setupBackendTexture() failed\n");
		return;
	}
	AXGL_UNUSED(border);
	BackendContext* backend_context = context->getBackendContext();
	bool result = false;
	AXGL_ASSERT(m_pBackendTexture != nullptr);
	if (m_target == GL_TEXTURE_CUBE_MAP) {
		result = m_pBackendTexture->setImageCube(backend_context, target, level, internalformat,
			width, height, format, type, pixels, &m_textureParameters);
	} else if(m_target == GL_TEXTURE_2D) {
		result = m_pBackendTexture->setImage2D(backend_context, level, internalformat,
			width, height, format, type, pixels, &m_textureParameters);
	} else {
		AXGL_ASSERT(0);
	}
	m_internalformat = internalformat;
	if (!result) {
		AXGL_DBGOUT("BackendTexture::texImage2d() failed\n");
	}
	return;
}

void CoreTexture::texSubImage2d(CoreContext* context, GLenum target, GLint level, GLint xoffset, GLint yoffset,
	GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels)
{
	if (m_pBackendTexture == nullptr) {
		return;
	}
	AXGL_ASSERT(context != nullptr);
	BackendContext* backend_context = context->getBackendContext();
	bool result = false;
	AXGL_ASSERT(m_pBackendTexture != nullptr);
	if (m_target == GL_TEXTURE_CUBE_MAP) {
		result = m_pBackendTexture->setSubImageCube(backend_context, target, level, xoffset, yoffset,
			width, height, format, type, pixels);
	} else if(m_target == GL_TEXTURE_2D) {
		result = m_pBackendTexture->setSubImage2D(backend_context, level, xoffset, yoffset,
			width, height, format, type, pixels);
	} else {
		AXGL_ASSERT(0);
	}
	if (!result) {
		AXGL_DBGOUT("BackendTexture::texSubImage2d() failed\n");
	}
	return;
}

void CoreTexture::texParameterf(GLenum pname, GLfloat param)
{
	// TODO: param チェックによる GL_INVALID_ENUM
	switch (pname) {
	case GL_TEXTURE_BASE_LEVEL:
		m_textureParameters.baseLevel = static_cast<GLint>(param);
		break;
	case GL_TEXTURE_COMPARE_FUNC:
		m_samplerParameters.compareFunc = static_cast<GLenum>(param);
		m_samplerDirty = true;
		break;
	case GL_TEXTURE_COMPARE_MODE:
		m_samplerParameters.compareMode = static_cast<GLenum>(param);
		m_samplerDirty = true;
		break;
	case GL_TEXTURE_MIN_FILTER:
		m_samplerParameters.minFilter = static_cast<GLenum>(param);
		m_samplerDirty = true;
		break;
	case GL_TEXTURE_MAG_FILTER:
		m_samplerParameters.magFilter = static_cast<GLenum>(param);
		m_samplerDirty = true;
		break;
	case GL_TEXTURE_MIN_LOD:
		m_samplerParameters.minLod = param;
		m_samplerDirty = true;
		break;
	case GL_TEXTURE_MAX_LOD:
		m_samplerParameters.maxLod = param;
		m_samplerDirty = true;
		break;
	case GL_TEXTURE_MAX_LEVEL:
		m_textureParameters.maxLevel = static_cast<GLint>(param);
		break;
	case GL_TEXTURE_SWIZZLE_R:
		m_textureParameters.swizzleR = static_cast<GLenum>(param);
		break;
	case GL_TEXTURE_SWIZZLE_G:
		m_textureParameters.swizzleG = static_cast<GLenum>(param);
		break;
	case GL_TEXTURE_SWIZZLE_B:
		m_textureParameters.swizzleB = static_cast<GLenum>(param);
		break;
	case GL_TEXTURE_SWIZZLE_A:
		m_textureParameters.swizzleA = static_cast<GLenum>(param);
		break;
	case GL_TEXTURE_WRAP_S:
		m_samplerParameters.wrapS = static_cast<GLenum>(param);
		m_samplerDirty = true;
		break;
	case GL_TEXTURE_WRAP_T:
		m_samplerParameters.wrapT = static_cast<GLenum>(param);
		m_samplerDirty = true;
		break;
	case GL_TEXTURE_WRAP_R:
		m_samplerParameters.wrapR = static_cast<GLenum>(param);
		m_samplerDirty = true;
		break;
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}

	return;
}

void CoreTexture::texParameterfv(GLenum pname, const GLfloat *params)
{
	// NOTE: GLES 3.0 仕様では、複数要素設定のパラメータが存在しない
	texParameterf(pname, *params);
	return;
}

void CoreTexture::texParameteri(GLenum pname, GLint param)
{
	// TODO: param チェックによる GL_INVALID_ENUM
	switch (pname) {
	case GL_TEXTURE_BASE_LEVEL:
		m_textureParameters.baseLevel = param;
		break;
	case GL_TEXTURE_COMPARE_FUNC:
		m_samplerParameters.compareFunc = param;
		m_samplerDirty = true;
		break;
	case GL_TEXTURE_COMPARE_MODE:
		m_samplerParameters.compareMode = param;
		m_samplerDirty = true;
		break;
	case GL_TEXTURE_MIN_FILTER:
		m_samplerParameters.minFilter = param;
		m_samplerDirty = true;
		break;
	case GL_TEXTURE_MAG_FILTER:
		m_samplerParameters.magFilter = param;
		m_samplerDirty = true;
		break;
	case GL_TEXTURE_MIN_LOD:
		m_samplerParameters.minLod = static_cast<float>(param);
		m_samplerDirty = true;
		break;
	case GL_TEXTURE_MAX_LOD:
		m_samplerParameters.maxLod = static_cast<float>(param);
		m_samplerDirty = true;
		break;
	case GL_TEXTURE_MAX_LEVEL:
		m_textureParameters.maxLevel = param;
		break;
	case GL_TEXTURE_SWIZZLE_R:
		m_textureParameters.swizzleR = param;
		break;
	case GL_TEXTURE_SWIZZLE_G:
		m_textureParameters.swizzleG = param;
		break;
	case GL_TEXTURE_SWIZZLE_B:
		m_textureParameters.swizzleB = param;
		break;
	case GL_TEXTURE_SWIZZLE_A:
		m_textureParameters.swizzleA = param;
		break;
	case GL_TEXTURE_WRAP_S:
		m_samplerParameters.wrapS = param;
		m_samplerDirty = true;
		break;
	case GL_TEXTURE_WRAP_T:
		m_samplerParameters.wrapT = param;
		m_samplerDirty = true;
		break;
	case GL_TEXTURE_WRAP_R:
		m_samplerParameters.wrapR = param;
		m_samplerDirty = true;
		break;
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return;
}

void CoreTexture::texParameteriv(GLenum pname, const GLint* params)
{
	// NOTE: GLES 3.0 仕様では、複数要素設定のパラメータが存在しない
	texParameteri(pname, *params);
	return;
}

void CoreTexture::texImage3d(CoreContext* context, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels)
{
	AXGL_ASSERT(context != nullptr);
	if (!setupBackendTexture(context)) {
		AXGL_DBGOUT("CoreTexture::setupBackendTexture() failed\n");
		return;
	}
	AXGL_UNUSED(target);
	AXGL_UNUSED(border);
	BackendContext* backend_context = context->getBackendContext();
	bool result = false;
	AXGL_ASSERT(m_pBackendTexture != nullptr);
	if (m_target == GL_TEXTURE_2D_ARRAY) {
		result = m_pBackendTexture->setImage2DArray(backend_context, level, internalformat,
			width, height, depth, format, type, pixels, &m_textureParameters);
	} else if(m_target == GL_TEXTURE_3D) {
		result = m_pBackendTexture->setImage3D(backend_context, level, internalformat,
			width, height, depth, format, type, pixels, &m_textureParameters);
	} else {
		AXGL_ASSERT(0);
	}
	m_internalformat = internalformat;
	if (!result) {
		AXGL_DBGOUT("BackendTexture::setImage*() failed\n");
	}
	return;
}

void CoreTexture::texSubImage3d(CoreContext* context, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
	GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels)
{
	if (m_pBackendTexture == nullptr) {
		return;
	}
	AXGL_UNUSED(target);
	AXGL_ASSERT(context != nullptr);
	BackendContext* backend_context = context->getBackendContext();
	bool result = false;
	AXGL_ASSERT(m_pBackendTexture != nullptr);
	if (m_target == GL_TEXTURE_2D_ARRAY) {
		result = m_pBackendTexture->setSubImage2DArray(backend_context, level, xoffset, yoffset, zoffset,
			width, height, depth, format, type, pixels);
	} else if(m_target == GL_TEXTURE_3D) {
		result = m_pBackendTexture->setSubImage3D(backend_context, level, xoffset, yoffset, zoffset,
			width, height, depth, format, type, pixels);
	} else {
		AXGL_ASSERT(0);
	}
	if (!result) {
		AXGL_DBGOUT("BackendTexture::setImage*() failed\n");
	}
	return;
}

void CoreTexture::copyTexSubImage3d(GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	// TODO:
	AXGL_UNUSED(level);
	AXGL_UNUSED(xoffset);
	AXGL_UNUSED(yoffset);
	AXGL_UNUSED(zoffset);
	AXGL_UNUSED(x);
	AXGL_UNUSED(y);
	AXGL_UNUSED(width);
	AXGL_UNUSED(height);
	return;
}

void CoreTexture::compressedTexImage3d(CoreContext* context, GLenum target, GLint level, GLenum internalformat,
	GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data)
{
	AXGL_ASSERT(context != nullptr);
	if (!setupBackendTexture(context)) {
		AXGL_DBGOUT("CoreTexture::setupBackendTexture() failed\n");
		return;
	}
	AXGL_UNUSED(target);
	AXGL_UNUSED(border);
	BackendContext* backend_context = context->getBackendContext();
	bool result = false;
	AXGL_ASSERT(m_pBackendTexture != nullptr);
	if (m_target == GL_TEXTURE_2D_ARRAY) {
		result = m_pBackendTexture->setCompressedImage2DArray(backend_context, level, internalformat,
			width, height, depth, imageSize, data, &m_textureParameters);
	} else if(m_target == GL_TEXTURE_2D) {
		result = m_pBackendTexture->setCompressedImage3D(backend_context, level, internalformat,
			width, height, depth, imageSize, data, &m_textureParameters);
	} else {
		AXGL_ASSERT(0);
	}
	m_internalformat = internalformat;
	if (!result) {
		AXGL_DBGOUT("BackendTexture::setCompressedImage*() failed\n");
	}
	return;
}

void CoreTexture::compressedTexSubImage3d(CoreContext* context, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
	GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data)
{
	// TODO:
	AXGL_UNUSED(context);
	AXGL_UNUSED(target);
	AXGL_UNUSED(level);
	AXGL_UNUSED(xoffset);
	AXGL_UNUSED(yoffset);
	AXGL_UNUSED(zoffset);
	AXGL_UNUSED(width);
	AXGL_UNUSED(height);
	AXGL_UNUSED(depth);
	AXGL_UNUSED(format);
	AXGL_UNUSED(imageSize);
	AXGL_UNUSED(data);
	return;
}

void CoreTexture::texStorage2d(CoreContext* context, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
	AXGL_ASSERT(context != nullptr);
	if (!setupBackendTexture(context)) {
		AXGL_DBGOUT("CoreTexture::setupBackendTexture() failed\n");
		return;
	}
	BackendContext* backend_context = context->getBackendContext();
	bool result = false;
	AXGL_ASSERT(m_pBackendTexture != nullptr);
	if (m_target == GL_TEXTURE_CUBE_MAP) {
		result = m_pBackendTexture->createStorageCube(backend_context, levels, internalformat, width, height, &m_textureParameters);
	} else if(m_target == GL_TEXTURE_2D) {
		result = m_pBackendTexture->createStorage2D(backend_context, levels, internalformat, width, height, &m_textureParameters);
	} else {
		AXGL_ASSERT(0);
	}
	m_immutableFormat = GL_TRUE;
	m_internalformat = internalformat;
	if (!result) {
		AXGL_DBGOUT("BackendTexture::createStorage*() failed\n");
	}
	return;
}

void CoreTexture::texStorage3d(CoreContext* context, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
	AXGL_ASSERT(context != nullptr);
	if (!setupBackendTexture(context)) {
		AXGL_DBGOUT("CoreTexture::setupBackendTexture() failed\n");
		return;
	}
	BackendContext* backend_context = context->getBackendContext();
	bool result = false;
	AXGL_ASSERT(m_pBackendTexture != nullptr);
	if (m_target == GL_TEXTURE_2D_ARRAY) {
		result = m_pBackendTexture->createStorage2DArray(backend_context, levels, internalformat, width, height, depth, &m_textureParameters);
	} else if(m_target == GL_TEXTURE_3D) {
		result = m_pBackendTexture->createStorage3D(backend_context, levels, internalformat, width, height, depth, &m_textureParameters);
	} else {
		AXGL_ASSERT(0);
	}
	m_immutableFormat = GL_TRUE;
	m_internalformat = internalformat; 
	if (!result) {
		AXGL_DBGOUT("BackendTexture::createStorage*() failed\n");
	}
	return;
}

void CoreTexture::generateMipmap(CoreContext* context)
{
	if (m_pBackendTexture == nullptr) {
		return;
	}
	AXGL_ASSERT(context != nullptr);
	bool result = m_pBackendTexture->generateMipmap(context->getBackendContext(), &m_textureParameters);
	if (!result) {
		AXGL_DBGOUT("BackendTexture::generateMipmap() failed\n");
	}
	return;
}

void CoreTexture::getParameterfv(GLenum pname, GLfloat* params)
{
	AXGL_ASSERT(params != nullptr);
	switch (pname) {
	case GL_TEXTURE_BASE_LEVEL:
		*params = static_cast<GLfloat>(m_textureParameters.baseLevel);
		break;
	case GL_TEXTURE_COMPARE_FUNC:
		*params = static_cast<GLfloat>(m_samplerParameters.compareFunc);
		break;
	case GL_TEXTURE_COMPARE_MODE:
		*params = static_cast<GLfloat>(m_samplerParameters.compareMode);
		break;
	case GL_TEXTURE_IMMUTABLE_FORMAT:
		*params = static_cast<GLfloat>(m_immutableFormat);
		break;
	case GL_TEXTURE_MAG_FILTER:
		*params = static_cast<GLfloat>(m_samplerParameters.magFilter);
		break;
	case GL_TEXTURE_MAX_LEVEL:
		*params = static_cast<GLfloat>(m_textureParameters.maxLevel);
		break;
	case GL_TEXTURE_MAX_LOD:
		*params = m_samplerParameters.maxLod;
		break;
	case GL_TEXTURE_MIN_FILTER:
		*params = static_cast<GLfloat>(m_samplerParameters.minFilter);
		break;
	case GL_TEXTURE_MIN_LOD:
		*params = m_samplerParameters.minLod;
		break;
	case GL_TEXTURE_SWIZZLE_R:
		*params = static_cast<GLfloat>(m_textureParameters.swizzleR);
		break;
	case GL_TEXTURE_SWIZZLE_G:
		*params = static_cast<GLfloat>(m_textureParameters.swizzleG);
		break;
	case GL_TEXTURE_SWIZZLE_B:
		*params = static_cast<GLfloat>(m_textureParameters.swizzleB);
		break;
	case GL_TEXTURE_SWIZZLE_A:
		*params = static_cast<GLfloat>(m_textureParameters.swizzleA);
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
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return;
}

void CoreTexture::getParameteriv(GLenum pname, GLint* params)
{
	AXGL_ASSERT(params != nullptr);
	switch (pname) {
	case GL_TEXTURE_BASE_LEVEL:
		*params = m_textureParameters.baseLevel;
		break;
	case GL_TEXTURE_COMPARE_FUNC:
		*params = m_samplerParameters.compareFunc;
		break;
	case GL_TEXTURE_COMPARE_MODE:
		*params = m_samplerParameters.compareMode;
		break;
	case GL_TEXTURE_IMMUTABLE_FORMAT:
		*params = m_immutableFormat;
		break;
	case GL_TEXTURE_MAG_FILTER:
		*params = m_samplerParameters.magFilter;
		break;
	case GL_TEXTURE_MAX_LEVEL:
		*params = m_textureParameters.maxLevel;
		break;
	case GL_TEXTURE_MAX_LOD:
		*params = static_cast<GLint>(m_samplerParameters.maxLod);
		break;
	case GL_TEXTURE_MIN_FILTER:
		*params = m_samplerParameters.minFilter;
		break;
	case GL_TEXTURE_MIN_LOD:
		*params = static_cast<GLint>(m_samplerParameters.minLod);
		break;
	case GL_TEXTURE_SWIZZLE_R:
		*params = m_textureParameters.swizzleR;
		break;
	case GL_TEXTURE_SWIZZLE_G:
		*params = m_textureParameters.swizzleG;
		break;
	case GL_TEXTURE_SWIZZLE_B:
		*params = m_textureParameters.swizzleB;
		break;
	case GL_TEXTURE_SWIZZLE_A:
		*params = m_textureParameters.swizzleA;
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
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return;
}

bool CoreTexture::setupSampler(CoreContext* context)
{
	if (m_pBackendSampler == nullptr) {
		m_pBackendSampler = BackendSampler::create();
		if (m_pBackendSampler == nullptr) {
			AXGL_DBGOUT("BackendSampler::create() failed\n");
			return false;
		}
		BackendContext* backend_context = context->getBackendContext();
		AXGL_ASSERT(backend_context != nullptr);
		m_pBackendSampler->initialize(backend_context);
		m_samplerDirty = true;
	}
	bool result = true;
	if (m_samplerDirty) {
		AXGL_ASSERT(context != nullptr);
		result = m_pBackendSampler->setupSampler(context->getBackendContext(), m_samplerParameters);
		m_samplerDirty = false;
	}
	return result;
}

bool CoreTexture::setupBackendTexture(CoreContext* context)
{
	bool result = true;
	if (m_pBackendTexture == nullptr) {
		result = false;
		m_pBackendTexture = BackendTexture::create();
		if(m_pBackendTexture != nullptr) {
			result = m_pBackendTexture->initialize(context->getBackendContext());
		}
	}
	return result;
}

} // namespace axgl
