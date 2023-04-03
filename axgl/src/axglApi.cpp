// axglApi.cpp
// API層の実装

#include "axglApi.h"
#include "AXGLAllocatorImpl.h"
#include "common/axglCommon.h"
#include "core/CoreContext.h"
#include "backend/Backend.h"

namespace axgl {

// コンテキストの作成
AXGLContext createContext(AXGLObjectsManager sharedObjectManager)
{
	axgl::CoreContext* context = AXGL_NEW(axgl::CoreContext());
	if (context != nullptr) {
		bool result = context->setSharedObjectManager(sharedObjectManager);
		AXGL_ASSERT(result);
	}
	if (!context->initialize()) {
		AXGL_DBGOUT("Context initialization failed.\n");
	}
	AXGL_DBGOUT("axgl::createContext:%p\n", context);
	return context;
}

// コンテキストの削除
void destroyContext(AXGLContext context)
{
	if (context == nullptr) {
		return;
	}
	context->releaseResources();
	context->terminate();
	AXGL_DELETE(context);
	return;
}

// カレントコンテキストの設定
void setCurrentContext(AXGLContext context)
{
	backendSetCurrentContext(context);
	return;
}

// カレントコンテキストの取得
AXGLContext getCurrentContext()
{
	return backendGetCurrentContext();
}

// ObjectsManagerをコンテキストから取得
AXGLObjectsManager getObjectsManager(AXGLContext context)
{
	if (context == nullptr) {
		return nullptr;
	}
	return context->getObjectsManager();
}

} // namespace axgl

//========================================================
// GLES 2.0 API
void GL_APIENTRY glActiveTexture(GLenum texture)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->activeTexture(texture);
	return;
}

void GL_APIENTRY glAttachShader(GLuint program, GLuint shader)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->attachShader(program, shader);
	return;
}

void GL_APIENTRY glBindAttribLocation(GLuint program, GLuint index, const GLchar* name)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->bindAttribLocation(program, index, name);
	return;
}

void GL_APIENTRY glBindBuffer(GLenum target, GLuint buffer)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->bindBuffer(target, buffer);
	return;
}

void GL_APIENTRY glBindFramebuffer(GLenum target, GLuint framebuffer)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->bindFramebuffer(target, framebuffer);
	return;
}

void GL_APIENTRY glBindRenderbuffer(GLenum target, GLuint renderbuffer)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->bindRenderbuffer(target, renderbuffer);
	return;
}

void GL_APIENTRY glBindTexture(GLenum target, GLuint texture)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->bindTexture(target, texture);
	return;
}

void GL_APIENTRY glBlendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->blendColor(red, green, blue, alpha);
	return;
}

void GL_APIENTRY glBlendEquation(GLenum mode)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->blendEquation(mode);
	return;
}

void GL_APIENTRY glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->blendEquationSeparate(modeRGB, modeAlpha);
	return;
}

void GL_APIENTRY glBlendFunc(GLenum sfactor, GLenum dfactor)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->blendFunc(sfactor, dfactor);
	return;
}

void GL_APIENTRY glBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->blendFuncSeparate(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
	return;
}

void GL_APIENTRY glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->bufferData(target, size, data, usage);
	return;
}

void GL_APIENTRY glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->bufferSubData(target, offset, size, data);
	return;
}

GLenum GL_APIENTRY glCheckFramebufferStatus(GLenum target)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return GL_FRAMEBUFFER_COMPLETE;
	}
	return context->checkFramebufferStatus(target);
}

void GL_APIENTRY glClear(GLbitfield mask)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->clear(mask);
	return;
}

void GL_APIENTRY glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->clearColor(red, green, blue, alpha);
	return;
}

void GL_APIENTRY glClearDepthf(GLfloat d)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->clearDepthf(d);
	return;
}

void GL_APIENTRY glClearStencil(GLint s)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->clearStencil(s);
	return;
}

void GL_APIENTRY glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->colorMask(red, green, blue, alpha);
	return;
}

void GL_APIENTRY glCompileShader(GLuint shader)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->compileShader(shader);
	return;
}

void GL_APIENTRY glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->compressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
	return;
}

void GL_APIENTRY glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->compressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
	return;
}

void GL_APIENTRY glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->copyTexImage2D(target, level, internalformat, x, y, width, height, border);
	return;
}

void GL_APIENTRY glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->copyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
	return;
}

GLuint GL_APIENTRY glCreateProgram(void)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return 0;
	}
	return context->createProgram();
}

GLuint GL_APIENTRY glCreateShader(GLenum type)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return 0;
	}
	return context->createShader(type);
}

void GL_APIENTRY glCullFace(GLenum mode)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->cullFace(mode);
	return;
}

void GL_APIENTRY glDeleteBuffers(GLsizei n, const GLuint* buffers)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->deleteBuffers(n, buffers);
	return;
}

void GL_APIENTRY glDeleteFramebuffers(GLsizei n, const GLuint* framebuffers)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->deleteFramebuffers(n, framebuffers);
	return;
}

void GL_APIENTRY glDeleteProgram(GLuint program)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->deleteProgram(program);
	return;
}

void GL_APIENTRY glDeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->deleteRenderbuffers(n, renderbuffers);
	return;
}

void GL_APIENTRY glDeleteShader(GLuint shader)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->deleteShader(shader);
	return;
}

void GL_APIENTRY glDeleteTextures(GLsizei n, const GLuint* textures)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->deleteTextures(n, textures);
	return;
}

void GL_APIENTRY glDepthFunc(GLenum func)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->depthFunc(func);
	return;
}

void GL_APIENTRY glDepthMask(GLboolean flag)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->depthMask(flag);
	return;
}

void GL_APIENTRY glDepthRangef(GLfloat n, GLfloat f)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->depthRangef(n, f);
	return;
}

void GL_APIENTRY glDetachShader(GLuint program, GLuint shader)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->detachShader(program, shader);
	return;
}

void GL_APIENTRY glDisable(GLenum cap)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->disable(cap);
	return;
}

void GL_APIENTRY glDisableVertexAttribArray(GLuint index)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->disableVertexAttribArray(index);
	return;
}

void GL_APIENTRY glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->drawArrays(mode, first, count);
	return;
}

void GL_APIENTRY glDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->drawElements(mode, count, type, indices);
	return;
}

void GL_APIENTRY glEnable(GLenum cap)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->enable(cap);
	return;
}

void GL_APIENTRY glEnableVertexAttribArray(GLuint index)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->enableVertexAttribArray(index);
	return;
}

void GL_APIENTRY glFinish(void)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->finish();
	return;
}

void GL_APIENTRY glFlush(void)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->flush();
	return;
}

void GL_APIENTRY glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->framebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
	return;
}

void GL_APIENTRY glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->framebufferTexture2D(target, attachment, textarget, texture, level);
	return;
}

void GL_APIENTRY glFrontFace(GLenum mode)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->frontFace(mode);
	return;
}

void GL_APIENTRY glGenBuffers(GLsizei n, GLuint* buffers)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->genBuffers(n, buffers);
	return;
}

void GL_APIENTRY glGenerateMipmap(GLenum target)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->generateMipmap(target);
	return;
}

void GL_APIENTRY glGenFramebuffers(GLsizei n, GLuint* framebuffers)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->genFramebuffers(n, framebuffers);
	return;
}

void GL_APIENTRY glGenRenderbuffers(GLsizei n, GLuint* renderbuffers)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->genRenderbuffers(n, renderbuffers);
	return;
}

void GL_APIENTRY glGenTextures(GLsizei n, GLuint* textures)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->genTextures(n, textures);
	return;
}

void GL_APIENTRY glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getActiveAttrib(program, index, bufSize, length, size, type, name);
	return;
}

void GL_APIENTRY glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getActiveUniform(program, index, bufSize, length, size, type, name);
	return;
}

void GL_APIENTRY glGetAttachedShaders(GLuint program, GLsizei maxCount, GLsizei* count, GLuint* shaders)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getAttachedShaders(program, maxCount, count, shaders);
	return;
}

GLint GL_APIENTRY glGetAttribLocation(GLuint program, const GLchar* name)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return -1;
	}
	return context->getAttribLocation(program, name);
}

void GL_APIENTRY glGetBooleanv(GLenum pname, GLboolean* data)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getBooleanv(pname, data);
	return;
}

void GL_APIENTRY glGetBufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getBufferParameteriv(target, pname, params);
	return;
}

GLenum GL_APIENTRY glGetError(void)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return GL_NO_ERROR;
	}
	return context->getError();
}

void GL_APIENTRY glGetFloatv(GLenum pname, GLfloat* data)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getFloatv(pname, data);
	return;
}

void GL_APIENTRY glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint* params)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getFramebufferAttachmentParameteriv(target, attachment, pname, params);
	return;
}

void GL_APIENTRY glGetIntegerv(GLenum pname, GLint* data)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getIntegerv(pname, data);
	return;
}

void GL_APIENTRY glGetProgramiv(GLuint program, GLenum pname, GLint* params)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getProgramiv(program, pname, params);
	return;
}

void GL_APIENTRY glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getProgramInfoLog(program, bufSize, length, infoLog);
	return;
}

void GL_APIENTRY glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getRenderbufferParameteriv(target, pname, params);
	return;
}

void GL_APIENTRY glGetShaderiv(GLuint shader, GLenum pname, GLint* params)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getShaderiv(shader, pname, params);
	return;
}

void GL_APIENTRY glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getShaderInfoLog(shader, bufSize, length, infoLog);
	return;
}

void GL_APIENTRY glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getShaderPrecisionFormat(shadertype, precisiontype, range, precision);
	return;
}

void GL_APIENTRY glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* source)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getShaderSource(shader, bufSize, length, source);
	return;
}

const GLubyte* GL_APIENTRY glGetString(GLenum name)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return nullptr;
	}
	return context->getString(name);
}

void GL_APIENTRY glGetTexParameterfv(GLenum target, GLenum pname, GLfloat* params)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getTexParameterfv(target, pname, params);
	return;
}

void GL_APIENTRY glGetTexParameteriv(GLenum target, GLenum pname, GLint* params)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getTexParameteriv(target, pname, params);
	return;
}

void GL_APIENTRY glGetUniformfv(GLuint program, GLint location, GLfloat* params)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getUniformfv(program, location, params);
	return;
}

void GL_APIENTRY glGetUniformiv(GLuint program, GLint location, GLint* params)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getUniformiv(program, location, params);
	return;
}

GLint GL_APIENTRY glGetUniformLocation(GLuint program, const GLchar* name)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return -1;
	}
	return context->getUniformLocation(program, name);
}

void GL_APIENTRY glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat* params)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getVertexAttribfv(index, pname, params);
	return;
}

void GL_APIENTRY glGetVertexAttribiv(GLuint index, GLenum pname, GLint* params)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getVertexAttribiv(index, pname, params);
	return;
}

void GL_APIENTRY glGetVertexAttribPointerv(GLuint index, GLenum pname, void** pointer)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getVertexAttribPointerv(index, pname, pointer);
	return;
}

void GL_APIENTRY glHint(GLenum target, GLenum mode)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->hint(target, mode);
	return;
}

GLboolean GL_APIENTRY glIsBuffer(GLuint buffer)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return GL_FALSE;
	}
	return context->isBuffer(buffer);
}

GLboolean GL_APIENTRY glIsEnabled(GLenum cap)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return GL_FALSE;
	}
	return context->isEnabled(cap);
}

GLboolean GL_APIENTRY glIsFramebuffer(GLuint framebuffer)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return GL_FALSE;
	}
	return context->isFramebuffer(framebuffer);
}

GLboolean GL_APIENTRY glIsProgram(GLuint program)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return GL_FALSE;
	}
	return context->isProgram(program);
}

GLboolean GL_APIENTRY glIsRenderbuffer(GLuint renderbuffer)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return GL_FALSE;
	}
	return context->isRenderbuffer(renderbuffer);
}

GLboolean GL_APIENTRY glIsShader(GLuint shader)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return GL_FALSE;
	}
	return context->isShader(shader);
}

GLboolean GL_APIENTRY glIsTexture(GLuint texture)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return GL_FALSE;
	}
	return context->isTexture(texture);
}

void GL_APIENTRY glLineWidth(GLfloat width)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->lineWidth(width);
	return;
}

void GL_APIENTRY glLinkProgram(GLuint program)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->linkProgram(program);
	return;
}

void GL_APIENTRY glPixelStorei(GLenum pname, GLint param)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->pixelStorei(pname, param);
	return;
}

void GL_APIENTRY glPolygonOffset(GLfloat factor, GLfloat units)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->polygonOffset(factor, units);
	return;
}

void GL_APIENTRY glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->readPixels(x, y, width, height, format, type, pixels);
	return;
}

void GL_APIENTRY glReleaseShaderCompiler(void)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->releaseShaderCompiler();
	return;
}

void GL_APIENTRY glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->renderbufferStorage(target, internalformat, width, height);
	return;
}

void GL_APIENTRY glSampleCoverage(GLfloat value, GLboolean invert)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->sampleCoverage(value, invert);
	return;
}

void GL_APIENTRY glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->scissor(x, y, width, height);
	return;
}

void GL_APIENTRY glShaderBinary(GLsizei count, const GLuint* shaders, GLenum binaryformat, const void* binary, GLsizei length)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->shaderBinary(count, shaders, binaryformat, binary, length);
	return;
}

void GL_APIENTRY glShaderSource(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->shaderSource(shader, count, string, length);
	return;
}

void GL_APIENTRY glStencilFunc(GLenum func, GLint ref, GLuint mask)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->stencilFunc(func, ref, mask);
	return;
}

void GL_APIENTRY glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->stencilFuncSeparate(face, func, ref, mask);
	return;
}

void GL_APIENTRY glStencilMask(GLuint mask)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->stencilMask(mask);
	return;
}

void GL_APIENTRY glStencilMaskSeparate(GLenum face, GLuint mask)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->stencilMaskSeparate(face, mask);
	return;
}

void GL_APIENTRY glStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->stencilOp(fail, zfail, zpass);
	return;
}

void GL_APIENTRY glStencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->stencilOpSeparate(face, sfail, dpfail, dppass);
	return;
}

void GL_APIENTRY glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->texImage2D(target, level, internalformat, width, height, border, format, type, pixels);
	return;
}

void GL_APIENTRY glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->texParameterf(target, pname, param);
	return;
}

void GL_APIENTRY glTexParameterfv(GLenum target, GLenum pname, const GLfloat* params)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->texParameterfv(target, pname, params);
	return;
}

void GL_APIENTRY glTexParameteri(GLenum target, GLenum pname, GLint param)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->texParameteri(target, pname, param);
	return;
}

void GL_APIENTRY glTexParameteriv(GLenum target, GLenum pname, const GLint* params)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->texParameteriv(target, pname, params);
	return;
}

void GL_APIENTRY glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->texSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
	return;
}

void GL_APIENTRY glUniform1f(GLint location, GLfloat v0)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->uniform1f(location, v0);
	return;
}

void GL_APIENTRY glUniform1fv(GLint location, GLsizei count, const GLfloat* value)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->uniform1fv(location, count, value);
	return;
}

void GL_APIENTRY glUniform1i(GLint location, GLint v0)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->uniform1i(location, v0);
	return;
}

void GL_APIENTRY glUniform1iv(GLint location, GLsizei count, const GLint* value)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->uniform1iv(location, count, value);
	return;
}

void GL_APIENTRY glUniform2f(GLint location, GLfloat v0, GLfloat v1)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->uniform2f(location, v0, v1);
	return;
}

void GL_APIENTRY glUniform2fv(GLint location, GLsizei count, const GLfloat* value)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->uniform2fv(location, count, value);
	return;
}

void GL_APIENTRY glUniform2i(GLint location, GLint v0, GLint v1)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->uniform2i(location, v0, v1);
	return;
}

void GL_APIENTRY glUniform2iv(GLint location, GLsizei count, const GLint* value)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->uniform2iv(location, count, value);
	return;
}

void GL_APIENTRY glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->uniform3f(location, v0, v1, v2);
	return;
}

void GL_APIENTRY glUniform3fv(GLint location, GLsizei count, const GLfloat* value)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->uniform3fv(location, count, value);
	return;
}

void GL_APIENTRY glUniform3i(GLint location, GLint v0, GLint v1, GLint v2)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->uniform3i(location, v0, v1, v2);
	return;
}

void GL_APIENTRY glUniform3iv(GLint location, GLsizei count, const GLint* value)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->uniform3iv(location, count, value);
	return;
}

void GL_APIENTRY glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->uniform4f(location, v0, v1, v2, v3);
	return;
}

void GL_APIENTRY glUniform4fv(GLint location, GLsizei count, const GLfloat* value)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->uniform4fv(location, count, value);
	return;
}

void GL_APIENTRY glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->uniform4i(location, v0, v1, v2, v3);
	return;
}

void GL_APIENTRY glUniform4iv(GLint location, GLsizei count, const GLint* value)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->uniform4iv(location, count, value);
	return;
}

void GL_APIENTRY glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->uniformMatrix2fv(location, count, transpose, value);
	return;
}

void GL_APIENTRY glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->uniformMatrix3fv(location, count, transpose, value);
	return;
}

void GL_APIENTRY glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->uniformMatrix4fv(location, count, transpose, value);
	return;
}

void GL_APIENTRY glUseProgram(GLuint program)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->useProgram(program);
	return;
}

void GL_APIENTRY glValidateProgram(GLuint program)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->validateProgram(program);
	return;
}

void GL_APIENTRY glVertexAttrib1f(GLuint index, GLfloat x)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->vertexAttrib1f(index, x);
	return;
}

void GL_APIENTRY glVertexAttrib1fv(GLuint index, const GLfloat* v)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->vertexAttrib1fv(index, v);
	return;
}

void GL_APIENTRY glVertexAttrib2f(GLuint index, GLfloat x, GLfloat y)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->vertexAttrib2f(index, x, y);
	return;
}

void GL_APIENTRY glVertexAttrib2fv(GLuint index, const GLfloat* v)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->vertexAttrib2fv(index, v);
	return;
}

void GL_APIENTRY glVertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->vertexAttrib3f(index, x, y, z);
	return;
}

void GL_APIENTRY glVertexAttrib3fv(GLuint index, const GLfloat* v)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->vertexAttrib3fv(index, v);
	return;
}

void GL_APIENTRY glVertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->vertexAttrib4f(index, x, y, z, w);
	return;
}

void GL_APIENTRY glVertexAttrib4fv(GLuint index, const GLfloat* v)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->vertexAttrib4fv(index, v);
	return;
}

void GL_APIENTRY glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->vertexAttribPointer(index, size, type, normalized, stride, pointer);
	return;
}

void GL_APIENTRY glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->viewport(x, y, width, height);
	return;
}

//================================================================
// GLES 3.0 API

void GL_APIENTRY glReadBuffer(GLenum src)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->readBuffer(src);
	return;
}

void GL_APIENTRY glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->drawRangeElements(mode, start, end, count, type, indices);
	return;
}

void GL_APIENTRY glTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->texImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels);
	return;
}

void GL_APIENTRY glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->texSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
	return;
}

void GL_APIENTRY glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->copyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
	return;
}

void GL_APIENTRY glCompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* data)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->compressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, data);
	return;
}

void GL_APIENTRY glCompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->compressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
	return;
}

void GL_APIENTRY glGenQueries(GLsizei n, GLuint* ids)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->genQueries(n, ids);
	return;
}

void GL_APIENTRY glDeleteQueries(GLsizei n, const GLuint* ids)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->deleteQueries(n, ids);
	return;
}

GLboolean GL_APIENTRY glIsQuery(GLuint id)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return GL_FALSE;
	}
	return context->isQuery(id);
}

void GL_APIENTRY glBeginQuery(GLenum target, GLuint id)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->beginQuery(target, id);
	return;
}

void GL_APIENTRY glEndQuery(GLenum target)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->endQuery(target);
	return;
}

void GL_APIENTRY glGetQueryiv(GLenum target, GLenum pname, GLint* params)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getQueryiv(target, pname, params);
	return;
}

void GL_APIENTRY glGetQueryObjectuiv(GLuint id, GLenum pname, GLuint* params)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getQueryObjectuiv(id, pname, params);
	return;
}

GLboolean GL_APIENTRY glUnmapBuffer(GLenum target)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return GL_FALSE;
	}
	return context->unmapBuffer(target);
}

void GL_APIENTRY glGetBufferPointerv(GLenum target, GLenum pname, void** params)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getBufferPointerv(target, pname, params);
	return;
}

void GL_APIENTRY glDrawBuffers(GLsizei n, const GLenum* bufs)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->drawBuffers(n, bufs);
	return;
}

void GL_APIENTRY glUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->uniformMatrix2x3fv(location, count, transpose, value);
	return;
}

void GL_APIENTRY glUniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->uniformMatrix3x2fv(location, count, transpose, value);
	return;
}

void GL_APIENTRY glUniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->uniformMatrix2x4fv(location, count, transpose, value);
	return;
}

void GL_APIENTRY glUniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->uniformMatrix4x2fv(location, count, transpose, value);
	return;
}

void GL_APIENTRY glUniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->uniformMatrix3x4fv(location, count, transpose, value);
	return;
}

void GL_APIENTRY glUniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->uniformMatrix4x3fv(location, count, transpose, value);
	return;
}

void GL_APIENTRY glBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->blitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
	return;
}

void GL_APIENTRY glRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->renderbufferStorageMultisample(target, samples, internalformat, width, height);
	return;
}

void GL_APIENTRY glFramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->framebufferTextureLayer(target, attachment, texture, level, layer);
	return;
}

void* GL_APIENTRY glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return nullptr;
	}
	return context->mapBufferRange(target, offset, length, access);
}

void GL_APIENTRY glFlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->flushMappedBufferRange(target, offset, length);
	return;
}

void GL_APIENTRY glBindVertexArray(GLuint array)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->bindVertexArray(array);
	return;
}

void GL_APIENTRY glDeleteVertexArrays(GLsizei n, const GLuint* arrays)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->deleteVertexArrays(n, arrays);
	return;
}

void GL_APIENTRY glGenVertexArrays(GLsizei n, GLuint* arrays)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->genVertexArrays(n, arrays);
	return;
}

GLboolean GL_APIENTRY glIsVertexArray(GLuint array)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return GL_FALSE;
	}
	return context->isVertexArray(array);
}

void GL_APIENTRY glGetIntegeri_v(GLenum target, GLuint index, GLint* data)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getIntegeri_v(target, index, data);
	return;
}

void GL_APIENTRY glBeginTransformFeedback(GLenum primitiveMode)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->beginTransformFeedback(primitiveMode);
	return;
}

void GL_APIENTRY glEndTransformFeedback(void)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->endTransformFeedback();
	return;
}

void GL_APIENTRY glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->bindBufferRange(target, index, buffer, offset, size);
	return;
}

void GL_APIENTRY glBindBufferBase(GLenum target, GLuint index, GLuint buffer)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->bindBufferBase(target, index, buffer);
	return;
}

void GL_APIENTRY glTransformFeedbackVaryings(GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->transformFeedbackVaryings(program, count, varyings, bufferMode);
	return;
}

void GL_APIENTRY glGetTransformFeedbackVarying(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getTransformFeedbackVarying(program, index, bufSize, length, size, type, name);
	return;
}

void GL_APIENTRY glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->vertexAttribIPointer(index, size, type, stride, pointer);
	return;
}

void GL_APIENTRY glGetVertexAttribIiv(GLuint index, GLenum pname, GLint* params)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getVertexAttribIiv(index, pname, params);
	return;
}

void GL_APIENTRY glGetVertexAttribIuiv(GLuint index, GLenum pname, GLuint* params)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getVertexAttribIuiv(index, pname, params);
	return;
}

void GL_APIENTRY glVertexAttribI4i(GLuint index, GLint x, GLint y, GLint z, GLint w)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->vertexAttribI4i(index, x, y, z, w);
	return;
}

void GL_APIENTRY glVertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->vertexAttribI4ui(index, x, y, z, w);
	return;
}

void GL_APIENTRY glVertexAttribI4iv(GLuint index, const GLint* v)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->vertexAttribI4iv(index, v);
	return;
}

void GL_APIENTRY glVertexAttribI4uiv(GLuint index, const GLuint* v)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->vertexAttribI4uiv(index, v);
	return;
}

void GL_APIENTRY glGetUniformuiv(GLuint program, GLint location, GLuint* params)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getUniformuiv(program, location, params);
	return;
}

GLint GL_APIENTRY glGetFragDataLocation(GLuint program, const GLchar* name)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return -1;
	}
	return context->getFragDataLocation(program, name);
}

void GL_APIENTRY glUniform1ui(GLint location, GLuint v0)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->uniform1ui(location, v0);
	return;
}

void GL_APIENTRY glUniform2ui(GLint location, GLuint v0, GLuint v1)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->uniform2ui(location, v0, v1);
	return;
}

void GL_APIENTRY glUniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->uniform3ui(location, v0, v1, v2);
	return;
}

void GL_APIENTRY glUniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->uniform4ui(location, v0, v1, v2, v3);
	return;
}

void GL_APIENTRY glUniform1uiv(GLint location, GLsizei count, const GLuint* value)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->uniform1uiv(location, count, value);
	return;
}

void GL_APIENTRY glUniform2uiv(GLint location, GLsizei count, const GLuint* value)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->uniform2uiv(location, count, value);
	return;
}

void GL_APIENTRY glUniform3uiv(GLint location, GLsizei count, const GLuint* value)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->uniform3uiv(location, count, value);
	return;
}

void GL_APIENTRY glUniform4uiv(GLint location, GLsizei count, const GLuint* value)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->uniform4uiv(location, count, value);
	return;
}

void GL_APIENTRY glClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint* value)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->clearBufferiv(buffer, drawbuffer, value);
	return;
}

void GL_APIENTRY glClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint* value)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->clearBufferuiv(buffer, drawbuffer, value);
	return;
}

void GL_APIENTRY glClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat* value)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->clearBufferfv(buffer, drawbuffer, value);
	return;
}

void GL_APIENTRY glClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->clearBufferfi(buffer, drawbuffer, depth, stencil);
	return;
}

const GLubyte* GL_APIENTRY glGetStringi(GLenum name, GLuint index)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return nullptr;
	}
	return context->getStringi(name, index);
}

void GL_APIENTRY glCopyBufferSubData(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->copyBufferSubData(readTarget, writeTarget, readOffset, writeOffset, size);
	return;
}

void GL_APIENTRY glGetUniformIndices(GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getUniformIndices(program, uniformCount, uniformNames, uniformIndices);
	return;
}

void GL_APIENTRY glGetActiveUniformsiv(GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getActiveUniformsiv(program, uniformCount, uniformIndices, pname, params);
	return;
}

GLuint GL_APIENTRY glGetUniformBlockIndex(GLuint program, const GLchar* uniformBlockName)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return ~0U;
	}
	return context->getUniformBlockIndex(program, uniformBlockName);
}

void GL_APIENTRY glGetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getActiveUniformBlockiv(program, uniformBlockIndex, pname, params);
	return;
}

void GL_APIENTRY glGetActiveUniformBlockName(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getActiveUniformBlockName(program, uniformBlockIndex, bufSize, length, uniformBlockName);
	return;
}

void GL_APIENTRY glUniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->uniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
	return;
}

void GL_APIENTRY glDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->drawArraysInstanced(mode, first, count, instancecount);
	return;
}

void GL_APIENTRY glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->drawElementsInstanced(mode, count, type, indices, instancecount);
	return;
}

GLsync GL_APIENTRY glFenceSync(GLenum condition, GLbitfield flags)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		// if glFenceSync fails, it will return zero.
		return static_cast<GLsync>(0);
	}
	return context->fenceSync(condition, flags);
}

GLboolean GL_APIENTRY glIsSync(GLsync sync)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return GL_FALSE;
	}
	return context->isSync(sync);
}

void GL_APIENTRY glDeleteSync(GLsync sync)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->deleteSync(sync);
	return;
}

GLenum GL_APIENTRY glClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return GL_WAIT_FAILED;
	}
	return context->clientWaitSync(sync, flags, timeout);
}

void GL_APIENTRY glWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->waitSync(sync, flags, timeout);
	return;
}

void GL_APIENTRY glGetInteger64v(GLenum pname, GLint64* data)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getInteger64v(pname, data);
	return;
}

void GL_APIENTRY glGetSynciv(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getSynciv(sync, pname, bufSize, length, values);
	return;
}

void GL_APIENTRY glGetInteger64i_v(GLenum target, GLuint index, GLint64* data)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getInteger64i_v(target, index, data);
	return;
}

void GL_APIENTRY glGetBufferParameteri64v(GLenum target, GLenum pname, GLint64* params)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getBufferParameteri64v(target, pname, params);
	return;
}

void GL_APIENTRY glGenSamplers(GLsizei count, GLuint* samplers)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->genSamplers(count, samplers);
	return;
}

void GL_APIENTRY glDeleteSamplers(GLsizei count, const GLuint* samplers)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->deleteSamplers(count, samplers);
	return;
}

GLboolean GL_APIENTRY glIsSampler(GLuint sampler)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return GL_FALSE;
	}
	return context->isSampler(sampler);
}

void GL_APIENTRY glBindSampler(GLuint unit, GLuint sampler)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->bindSampler(unit, sampler);
	return;
}

void GL_APIENTRY glSamplerParameteri(GLuint sampler, GLenum pname, GLint param)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->samplerParameteri(sampler, pname, param);
	return;
}

void GL_APIENTRY glSamplerParameteriv(GLuint sampler, GLenum pname, const GLint* param)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->samplerParameteriv(sampler, pname, param);
	return;
}

void GL_APIENTRY glSamplerParameterf(GLuint sampler, GLenum pname, GLfloat param)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->samplerParameterf(sampler, pname, param);
	return;
}

void GL_APIENTRY glSamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat* param)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->samplerParameterfv(sampler, pname, param);
	return;
}

void GL_APIENTRY glGetSamplerParameteriv(GLuint sampler, GLenum pname, GLint* params)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->samplerParameteriv(sampler, pname, params);
	return;
}

void GL_APIENTRY glGetSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat* params)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getSamplerParameterfv(sampler, pname, params);
	return;
}

void GL_APIENTRY glVertexAttribDivisor(GLuint index, GLuint divisor)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->vertexAttribDivisor(index, divisor);
	return;
}

void GL_APIENTRY glBindTransformFeedback(GLenum target, GLuint id)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->bindTransformFeedback(target, id);
	return;
}

void GL_APIENTRY glDeleteTransformFeedbacks(GLsizei n, const GLuint* ids)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->deleteTransformFeedbacks(n, ids);
	return;
}

void GL_APIENTRY glGenTransformFeedbacks(GLsizei n, GLuint* ids)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->genTransformFeedbacks(n, ids);
	return;
}

GLboolean GL_APIENTRY glIsTransformFeedback(GLuint id)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return GL_FALSE;
	}
	return context->isTransformFeedback(id);
}

void GL_APIENTRY glPauseTransformFeedback(void)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->pauseTransformFeedback();
	return;
}

void GL_APIENTRY glResumeTransformFeedback(void)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->resumeTransformFeedback();
	return;
}

void GL_APIENTRY glGetProgramBinary(GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, void* binary)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getProgramBinary(program, bufSize, length, binaryFormat, binary);
	return;
}

void GL_APIENTRY glProgramBinary(GLuint program, GLenum binaryFormat, const void* binary, GLsizei length)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->programBinary(program, binaryFormat, binary, length);
	return;
}

void GL_APIENTRY glProgramParameteri(GLuint program, GLenum pname, GLint value)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->programParameteri(program, pname, value);
	return;
}

void GL_APIENTRY glInvalidateFramebuffer(GLenum target, GLsizei numAttachments, const GLenum* attachments)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->invalidateFramebuffer(target, numAttachments, attachments);
	return;
}

void GL_APIENTRY glInvalidateSubFramebuffer(GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->invalidateSubFramebuffer(target, numAttachments, attachments, x, y, width, height);
	return;
}

void GL_APIENTRY glTexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->texStorage2D(target, levels, internalformat, width, height);
	return;
}

void GL_APIENTRY glTexStorage3D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->texStorage3D(target, levels, internalformat, width, height, depth);
	return;
}

void GL_APIENTRY glGetInternalformativ(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->getInternalformativ(target, internalformat, pname, bufSize, params);
	return;
}

//======================================================================
// Extension API

// キャッシュのInvalidateを実行する
void GL_APIENTRY glInvalidateCacheAXGL(GLbitfield flags)
{
	axgl::CoreContext* context = axgl::getCurrentContext();
	if (context == nullptr) {
		return;
	}
	context->invalidateCache(flags);
	return;
}
