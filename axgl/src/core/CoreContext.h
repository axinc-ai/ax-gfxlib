// CoreContext.h
// コンテキストクラスの宣言
#ifndef __CoreContext_h_
#define __CoreContext_h_

#include "../common/axglCommon.h"
#include "CoreState.h"

#include <memory>

namespace axgl {

class BackendContext;
class CoreObjectsManager;
class CoreRenderbuffer;
struct BackendRenderbufferFormat;

// コンテキストクラス
class CoreContext
{
public:
	CoreContext();
	~CoreContext();

public:
	// shared object manager
	bool setSharedObjectManager(CoreObjectsManager* sharedObjectManager);
	// release internal resources
	void releaseResources();
	// set error code
	void setErrorCode(GLenum code);
	// initialize
	bool initialize();
	// terminate
	void terminate();
	// get backend context
	BackendContext* getBackendContext() const
	{
		return m_pBackendContext;
	}
	// get objects manager
	CoreObjectsManager* getObjectsManager() const
	{
		return m_pObjectsManager;
	}
	// setup initial viewport
	void setupInitialViewport(const CoreRenderbuffer* renderbuffer);

public:
	// GLES 2.0 API
	void activeTexture(GLenum texture);
	void attachShader(GLuint program, GLuint shader);
	void bindAttribLocation(GLuint program, GLuint index, const GLchar *name);
	void bindBuffer(GLenum target, GLuint buffer);
	void bindFramebuffer(GLenum target, GLuint framebuffer);
	void bindRenderbuffer(GLenum target, GLuint renderbuffer);
	void bindTexture(GLenum target, GLuint texture);
	void blendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
	void blendEquation(GLenum mode);
	void blendEquationSeparate (GLenum modeRGB, GLenum modeAlpha);
	void blendFunc(GLenum sfactor, GLenum dfactor);
	void blendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
	void bufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
	void bufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
	GLenum checkFramebufferStatus(GLenum target);
	void clear(GLbitfield mask);
	void clearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
	void clearDepthf(GLfloat d);
	void clearStencil(GLint s);
	void colorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
	void compileShader(GLuint shader);
	void compressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
	void compressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
	void copyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
	void copyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
	GLuint createProgram(void);
	GLuint createShader(GLenum type);
	void cullFace(GLenum mode);
	void deleteBuffers(GLsizei n, const GLuint *buffers);
	void deleteFramebuffers(GLsizei n, const GLuint *framebuffers);
	void deleteProgram(GLuint program);
	void deleteRenderbuffers(GLsizei n, const GLuint *renderbuffers);
	void deleteShader(GLuint shader);
	void deleteTextures(GLsizei n, const GLuint *textures);
	void depthFunc(GLenum func);
	void depthMask(GLboolean flag);
	void depthRangef(GLfloat n, GLfloat f);
	void detachShader(GLuint program, GLuint shader);
	void disable(GLenum cap);
	void disableVertexAttribArray(GLuint index);
	void drawArrays(GLenum mode, GLint first, GLsizei count);
	void drawElements(GLenum mode, GLsizei count, GLenum type, const void *indices);
	void enable(GLenum cap);
	void enableVertexAttribArray(GLuint index);
	void finish(void);
	void flush(void);
	void framebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
	void framebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
	void frontFace(GLenum mode);
	void genBuffers(GLsizei n, GLuint *buffers);
	void generateMipmap(GLenum target);
	void genFramebuffers(GLsizei n, GLuint *framebuffers);
	void genRenderbuffers(GLsizei n, GLuint *renderbuffers);
	void genTextures(GLsizei n, GLuint *textures);
	void getActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
	void getActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
	void getAttachedShaders(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders);
	GLint getAttribLocation(GLuint program, const GLchar *name);
	void getBooleanv(GLenum pname, GLboolean *data);
	void getBufferParameteriv(GLenum target, GLenum pname, GLint *params);
	GLenum getError(void);
	void getFloatv(GLenum pname, GLfloat *data);
	void getFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint *params);
	void getIntegerv(GLenum pname, GLint *data);
	void getProgramiv(GLuint program, GLenum pname, GLint *params);
	void getProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
	void getRenderbufferParameteriv(GLenum target, GLenum pname, GLint *params);
	void getShaderiv(GLuint shader, GLenum pname, GLint *params);
	void getShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
	void getShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision);
	void getShaderSource(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
	const GLubyte* getString(GLenum name);
	void getTexParameterfv(GLenum target, GLenum pname, GLfloat *params);
	void getTexParameteriv(GLenum target, GLenum pname, GLint *params);
	void getUniformfv(GLuint program, GLint location, GLfloat *params);
	void getUniformiv(GLuint program, GLint location, GLint *params);
	GLint getUniformLocation(GLuint program, const GLchar *name);
	void getVertexAttribfv(GLuint index, GLenum pname, GLfloat *params);
	void getVertexAttribiv(GLuint index, GLenum pname, GLint *params);
	void getVertexAttribPointerv(GLuint index, GLenum pname, void **pointer);
	void hint(GLenum target, GLenum mode);
	GLboolean isBuffer(GLuint buffer);
	GLboolean isEnabled(GLenum cap);
	GLboolean isFramebuffer(GLuint framebuffer);
	GLboolean isProgram(GLuint program);
	GLboolean isRenderbuffer(GLuint renderbuffer);
	GLboolean isShader(GLuint shader);
	GLboolean isTexture(GLuint texture);
	void lineWidth(GLfloat width);
	void linkProgram(GLuint program);
	void pixelStorei(GLenum pname, GLint param);
	void polygonOffset(GLfloat factor, GLfloat units);
	void readPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels);
	void releaseShaderCompiler(void);
	void renderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
	void sampleCoverage(GLfloat value, GLboolean invert);
	void scissor(GLint x, GLint y, GLsizei width, GLsizei height);
	void shaderBinary(GLsizei count, const GLuint *shaders, GLenum binaryformat, const void *binary, GLsizei length);
	void shaderSource(GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
	void stencilFunc(GLenum func, GLint ref, GLuint mask);
	void stencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask);
	void stencilMask(GLuint mask);
	void stencilMaskSeparate(GLenum face, GLuint mask);
	void stencilOp(GLenum fail, GLenum zfail, GLenum zpass);
	void stencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
	void texImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
	void texParameterf(GLenum target, GLenum pname, GLfloat param);
	void texParameterfv(GLenum target, GLenum pname, const GLfloat *params);
	void texParameteri(GLenum target, GLenum pname, GLint param);
	void texParameteriv(GLenum target, GLenum pname, const GLint *params);
	void texSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
	void uniform1f(GLint location, GLfloat v0);
	void uniform1fv(GLint location, GLsizei count, const GLfloat *value);
	void uniform1i(GLint location, GLint v0);
	void uniform1iv(GLint location, GLsizei count, const GLint *value);
	void uniform2f(GLint location, GLfloat v0, GLfloat v1);
	void uniform2fv(GLint location, GLsizei count, const GLfloat *value);
	void uniform2i(GLint location, GLint v0, GLint v1);
	void uniform2iv(GLint location, GLsizei count, const GLint *value);
	void uniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
	void uniform3fv(GLint location, GLsizei count, const GLfloat *value);
	void uniform3i(GLint location, GLint v0, GLint v1, GLint v2);
	void uniform3iv(GLint location, GLsizei count, const GLint *value);
	void uniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
	void uniform4fv(GLint location, GLsizei count, const GLfloat *value);
	void uniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
	void uniform4iv(GLint location, GLsizei count, const GLint *value);
	void uniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	void uniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	void uniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	void useProgram(GLuint program);
	void validateProgram(GLuint program);
	void vertexAttrib1f(GLuint index, GLfloat x);
	void vertexAttrib1fv(GLuint index, const GLfloat *v);
	void vertexAttrib2f(GLuint index, GLfloat x, GLfloat y);
	void vertexAttrib2fv(GLuint index, const GLfloat *v);
	void vertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z);
	void vertexAttrib3fv(GLuint index, const GLfloat *v);
	void vertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
	void vertexAttrib4fv(GLuint index, const GLfloat *v);
	void vertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
	void viewport(GLint x, GLint y, GLsizei width, GLsizei height);
	// GLES 3.0 API
	void readBuffer(GLenum src);
	void drawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices);
	void texImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
	void texSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
	void copyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
	void compressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data);
	void compressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
	void genQueries(GLsizei n, GLuint *ids);
	void deleteQueries(GLsizei n, const GLuint *ids);
	GLboolean isQuery(GLuint id);
	void beginQuery(GLenum target, GLuint id);
	void endQuery(GLenum target);
	void getQueryiv(GLenum target, GLenum pname, GLint *params);
	void getQueryObjectuiv(GLuint id, GLenum pname, GLuint *params);
	GLboolean unmapBuffer(GLenum target);
	void getBufferPointerv(GLenum target, GLenum pname, void **params);
	void drawBuffers(GLsizei n, const GLenum *bufs);
	void uniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	void uniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	void uniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	void uniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	void uniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	void uniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	void blitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
	void renderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
	void framebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
	void* mapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
	void flushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length);
	void bindVertexArray(GLuint array);
	void deleteVertexArrays(GLsizei n, const GLuint *arrays);
	void genVertexArrays(GLsizei n, GLuint *arrays);
	GLboolean isVertexArray(GLuint array);
	void getIntegeri_v(GLenum target, GLuint index, GLint *data);
	void beginTransformFeedback(GLenum primitiveMode);
	void endTransformFeedback(void);
	void bindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
	void bindBufferBase(GLenum target, GLuint index, GLuint buffer);
	void transformFeedbackVaryings(GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode);
	void getTransformFeedbackVarying(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name);
	void vertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
	void getVertexAttribIiv(GLuint index, GLenum pname, GLint *params);
	void getVertexAttribIuiv(GLuint index, GLenum pname, GLuint *params);
	void vertexAttribI4i(GLuint index, GLint x, GLint y, GLint z, GLint w);
	void vertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
	void vertexAttribI4iv(GLuint index, const GLint *v);
	void vertexAttribI4uiv(GLuint index, const GLuint *v);
	void getUniformuiv(GLuint program, GLint location, GLuint *params);
	GLint getFragDataLocation(GLuint program, const GLchar *name);
	void uniform1ui(GLint location, GLuint v0);
	void uniform2ui(GLint location, GLuint v0, GLuint v1);
	void uniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2);
	void uniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
	void uniform1uiv(GLint location, GLsizei count, const GLuint *value);
	void uniform2uiv(GLint location, GLsizei count, const GLuint *value);
	void uniform3uiv(GLint location, GLsizei count, const GLuint *value);
	void uniform4uiv(GLint location, GLsizei count, const GLuint *value);
	void clearBufferiv(GLenum buffer, GLint drawbuffer, const GLint *value);
	void clearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint *value);
	void clearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat *value);
	void clearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
	const GLubyte* getStringi(GLenum name, GLuint index);
	void copyBufferSubData(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
	void getUniformIndices(GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices);
	void getActiveUniformsiv(GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params);
	GLuint getUniformBlockIndex(GLuint program, const GLchar *uniformBlockName);
	void getActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params);
	void getActiveUniformBlockName(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName);
	void uniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
	void drawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
	void drawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount);
	GLsync fenceSync(GLenum condition, GLbitfield flags);
	GLboolean isSync(GLsync sync);
	void deleteSync(GLsync sync);
	GLenum clientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout);
	void waitSync(GLsync sync, GLbitfield flags, GLuint64 timeout);
	void getInteger64v(GLenum pname, GLint64 *data);
	void getSynciv(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values);
	void getInteger64i_v(GLenum target, GLuint index, GLint64 *data);
	void getBufferParameteri64v(GLenum target, GLenum pname, GLint64 *params);
	void genSamplers(GLsizei count, GLuint *samplers);
	void deleteSamplers(GLsizei count, const GLuint *samplers);
	GLboolean isSampler(GLuint sampler);
	void bindSampler(GLuint unit, GLuint sampler);
	void samplerParameteri(GLuint sampler, GLenum pname, GLint param);
	void samplerParameteriv(GLuint sampler, GLenum pname, const GLint *param);
	void samplerParameterf(GLuint sampler, GLenum pname, GLfloat param);
	void samplerParameterfv(GLuint sampler, GLenum pname, const GLfloat *param);
	void getSamplerParameteriv(GLuint sampler, GLenum pname, GLint *params);
	void getSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat *params);
	void vertexAttribDivisor(GLuint index, GLuint divisor);
	void bindTransformFeedback(GLenum target, GLuint id);
	void deleteTransformFeedbacks(GLsizei n, const GLuint *ids);
	void genTransformFeedbacks(GLsizei n, GLuint *ids);
	GLboolean isTransformFeedback(GLuint id);
	void pauseTransformFeedback(void);
	void resumeTransformFeedback(void);
	void getProgramBinary(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);
	void programBinary(GLuint program, GLenum binaryFormat, const void *binary, GLsizei length);
	void programParameteri(GLuint program, GLenum pname, GLint value);
	void invalidateFramebuffer(GLenum target, GLsizei numAttachments, const GLenum *attachments);
	void invalidateSubFramebuffer(GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height);
	void texStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
	void texStorage3D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
	void getInternalformativ(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params);
	// other interface methods
	CoreRenderbuffer* getCurrentRenderbuffer();
	void invalidateCache(GLbitfield flags);

private:
	GLenum getDrawFramebufferFormat(int colorIndex);
	GLenum getDrawDepthbufferFormat();
	GLenum getDrawStencilbufferFormat();
	static const BackendRenderbufferFormat* getRenderbufferFormat(GLuint count, const BackendRenderbufferFormat* formats, GLenum format);

private:
	CoreState m_state;
	CoreObjectsManager* m_pObjectsManager = nullptr;
	GLenum m_errorCode = GL_NO_ERROR;
	BackendContext* m_pBackendContext = nullptr;
	bool m_isViewportInitialized = false;
};

} // namespace legm

#endif // __CoreContext_h_
