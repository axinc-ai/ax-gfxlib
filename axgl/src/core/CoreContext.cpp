// CoreContext.cpp
// コンテキストクラスの実装
#include "CoreContext.h"
#include "CoreObjectsManager.h"
#include "CoreBuffer.h"
#include "CoreProgram.h"
#include "CoreFramebuffer.h"
#include "CoreShader.h"
#include "CoreTexture.h"
#include "CoreRenderbuffer.h"
#include "CoreQuery.h"
#include "CoreSync.h"
#include "CoreSampler.h"
#include "CoreTransformFeedback.h"
#include "CoreVertexArray.h"
#include "CoreUtility.h"

#include "../backend/BackendContext.h"
#include "../backend/BackendRenderbuffer.h"

// GLの型変換に使用するマクロ
#define GLINT_TO_GLBOOLEAN(a) (((a) == 0) ? GL_FALSE : GL_TRUE)
#define GLFLOAT_TO_GLBOOLEAN(a) (((a) == 0.0f) ? GL_FALSE : GL_TRUE)
#define FLOAT_TO_INT_COLOR(a) (((a)>1.0f) ? 0x7fffffff : ((a)<-1.0f) ? 0x80000000 : (GLint)((double)(a)*0x7fffffff))
#define FLOAT_TO_INT64_COLOR(a) (((a)>1.0f) ? 0x7fffffffffffffffULL : ((a)<-1.0f) ? 0x8000000000000000ULL : (GLint)((double)(a)*0x7fffffffffffffffULL))

namespace axgl {

// glGetStringで返す文字列
static const char* c_extension_string = "";
static const char* c_vendor_string = "Vendor string";
static const char* c_renderer_string = "Renderer string";
static const char* c_version_string = "3.0";
static const char* c_shader_language_version = "3.0.0";

// コンストラクタ
CoreContext::CoreContext()
{
}

// デストラクタ
CoreContext::~CoreContext()
{
}

// ObjectsManagerを設定
// NOTE: shared contextを実現するための機能
bool CoreContext::setSharedObjectManager(CoreObjectsManager* sharedObjectManager)
{
	if (sharedObjectManager != nullptr) {
		// リファレンスカウンタを増加
		sharedObjectManager->addRef();
		m_pObjectsManager = sharedObjectManager;
	} else {
		// 新規にObjectsManagerを作成
		CoreObjectsManager* objectsManager = AXGL_NEW(CoreObjectsManager);
		if (objectsManager != nullptr) {
			objectsManager->addRef();
			m_pObjectsManager = objectsManager;
		}
	}
	return true;
}

// 内部リソースを解放
void CoreContext::releaseResources()
{
	m_state.releaseResources(this);
	if (m_pObjectsManager != nullptr) {
		m_pObjectsManager->release(this);
		m_pObjectsManager = nullptr;
	}
	return;
}

// エラーコードを設定
void CoreContext::setErrorCode(GLenum code)
{
	// NOTE: エラーなしの場合のみ設定、GLの仕様
	if (m_errorCode == GL_NO_ERROR) {
		m_errorCode = code;
	}
	return;
}

// 初期化
bool CoreContext::initialize()
{
	// NOTE: 多重呼び出しの保護なし
	AXGL_ASSERT(m_pBackendContext == nullptr);
	m_pBackendContext = BackendContext::create();
	if (m_pBackendContext == nullptr) {
		AXGL_DBGOUT("BackendContext::create() failed.\n");
		return false;
	}
	if (!m_pBackendContext->initialize()) {
		AXGL_DBGOUT("BackendContext::initialize() failed.\n");
		return false;
	}
	return true;
}

// 終了
void CoreContext::terminate()
{
	if (m_pBackendContext == nullptr) {
		return;
	}
	m_pBackendContext->terminate();
	BackendContext::destroy(m_pBackendContext);
	m_pBackendContext = nullptr;
	return;
}

// Renderbuferから初期Viewportを設定する
void CoreContext::setupInitialViewport(const CoreRenderbuffer* renderbuffer)
{
	if (m_isViewportInitialized || (renderbuffer == nullptr)) {
		return;
	}
	GLsizei rbo_width = renderbuffer->getWidth();
	GLsizei rbo_height = renderbuffer->getHeight();
	m_state.setViewport(0, 0, rbo_width, rbo_height);
	m_isViewportInitialized = true;
	return;
}

// GLES 2.0 API ---------------------------------------------------------------
void CoreContext::activeTexture(GLenum texture)
{
	m_state.setActiveTexture(texture);
	return;
}

void CoreContext::attachShader(GLuint program, GLuint shader)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreProgram* core_program = m_pObjectsManager->getProgram(program);
	if (core_program == nullptr) {
		// GL_INVALID_OPERATION is generated if program is not a program object.
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	CoreShader* core_shader = m_pObjectsManager->getShader(shader);
	if (core_program == nullptr) {
		// GL_INVALID_OPERATION is generated if shader is not a shader object.
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_program->attachShader(core_shader);
	return;
}

void CoreContext::bindAttribLocation(GLuint program, GLuint index, const GLchar* name)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreProgram* core_program = m_pObjectsManager->getProgram(program);
	if (core_program == nullptr) {
		// GL_INVALID_OPERATION is generated if program is not a program object.
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_program->bindAttribLocation(index, name);
	return;
}

void CoreContext::bindBuffer(GLenum target, GLuint buffer)
{
	CoreBuffer* buffer_object = nullptr;
	if (buffer != 0) {
		buffer_object = m_pObjectsManager->getBuffer(buffer);
		if (buffer_object != nullptr) {
			buffer_object->setTarget(target);
		}
	}
	CoreVertexArray* core_vertex_array = m_state.getVertexArray();
	if (core_vertex_array != nullptr && target == GL_ELEMENT_ARRAY_BUFFER) {
		// set to vertex array object
		core_vertex_array->setIndexBuffer(this, buffer_object);
	}
	// Even if it is a VAO IndexBuffer, it is also used in glBufferData and so on, so it is always stored in global information.
	m_state.setBuffer(this, target, buffer_object);
	return;
}

void CoreContext::bindFramebuffer(GLenum target, GLuint framebuffer)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreFramebuffer* core_framebuffer = m_pObjectsManager->getFramebuffer(framebuffer);
	m_state.setFramebuffer(this, target, core_framebuffer);
	return;
}

void CoreContext::bindRenderbuffer(GLenum target, GLuint renderbuffer)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreRenderbuffer* core_renderbuffer = m_pObjectsManager->getRenderbuffer(renderbuffer);
	m_state.setRenderbuffer(this, target, core_renderbuffer);
	return;
}

void CoreContext::bindTexture(GLenum target, GLuint texture)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreTexture* core_texture = m_pObjectsManager->getTexture(texture);
	m_state.setTexture(this, target, core_texture);
	return;
}

void CoreContext::blendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	m_state.setBlendColor(red, green, blue, alpha);
	return;
}

void CoreContext::blendEquation(GLenum mode)
{
	m_state.setBlendEquation(mode, mode);
	return;
}

void CoreContext::blendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
{
	m_state.setBlendEquation(modeRGB, modeAlpha);
	return;
}

void CoreContext::blendFunc(GLenum sfactor, GLenum dfactor)
{
	m_state.setBlendFunc(sfactor, dfactor, sfactor, dfactor);
	return;
}

void CoreContext::blendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
{
	m_state.setBlendFunc(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
	return;
}

void CoreContext::bufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
{
	CoreBuffer* buffer_object = m_state.getBuffer(target);
	if (buffer_object != nullptr) {
		buffer_object->setData(this, size, data, usage);
	}
	return;
}

void CoreContext::bufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data)
{
	CoreBuffer* buffer_object = m_state.getBuffer(target);
	if (buffer_object != nullptr) {
		buffer_object->setSubData(this, offset, size, data);
	}
	return;
}

GLenum CoreContext::checkFramebufferStatus(GLenum target)
{
	GLenum rval = GL_FRAMEBUFFER_COMPLETE;
	CoreFramebuffer* framebuffer_object = m_state.getFramebuffer(target);
	if (framebuffer_object != nullptr) {
		rval = framebuffer_object->checkStatus();
	} else {
		// TODO: default framebuffer
	}
	return rval;
}

void CoreContext::clear(GLbitfield mask)
{
	if (m_pBackendContext == nullptr) {
		return;
	}
	// clear
	m_pBackendContext->clear(mask, &m_state.getDrawParameters(), &m_state.getClearParams());
	return;
}

void CoreContext::clearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	m_state.setColorClearValue(red, green, blue, alpha);
	return;
}

void CoreContext::clearDepthf(GLfloat d)
{
	m_state.setDepthClearValue(d);
	return;
}

void CoreContext::clearStencil(GLint s)
{
	m_state.setStencilClearValue(s);
	return;
}

void CoreContext::colorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
	m_state.setColorWriteMask(red, green, blue, alpha);
	return;
}

void CoreContext::compileShader(GLuint shader)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreShader* core_shader = m_pObjectsManager->getShader(shader);
	if (core_shader == nullptr) {
		// GL_INVALID_OPERATION is generated if shader is not a shader object
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_shader->compile(this);
	return;
}

void CoreContext::compressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data)
{
	// TODO: target check for 2D or CubeMap face
	CoreTexture* core_texture = m_state.getTexture(target);
	if (core_texture == nullptr) {
		return;
	}
	core_texture->compressedTexImage2d(this, target, level, internalformat, width, height, border, imageSize, data);
	return;
}

void CoreContext::compressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data)
{
	// TODO: target check for 2D or CubeMap face
	CoreTexture* core_texture = m_state.getTexture(target);
	if (core_texture == nullptr) {
		return;
	}
	core_texture->compressedTexSubImage2d(this, target, level, xoffset, yoffset, width, height, format, imageSize, data);
	return;
}

void CoreContext::copyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
	// TODO: target check for 2D or CubeMap face
	CoreTexture* core_texture = m_state.getTexture(target);
	if (core_texture == nullptr) {
		return;
	}
	core_texture->copyTexImage2d(level, internalformat, x, y, width, height, border);
	return;
}

void CoreContext::copyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	// TODO: target check for 2D or CubeMap face
	CoreTexture* core_texture = m_state.getTexture(target);
	if (core_texture == nullptr) {
		return;
	}
	core_texture->copyTexSubImage2d(level, xoffset, yoffset, x, y, width, height);
	return;
}

GLuint CoreContext::createProgram(void)
{
	if (m_pObjectsManager == nullptr) {
		return 0;
	}
	return m_pObjectsManager->createProgram(this);
}

GLuint CoreContext::createShader(GLenum type)
{
	if (m_pObjectsManager == nullptr) {
		return 0;
	}
	return m_pObjectsManager->createShader(this, type);
}

void CoreContext::cullFace(GLenum mode)
{
	m_state.setCullFaceMode(mode);
	return;
}

void CoreContext::deleteBuffers(GLsizei n, const GLuint* buffers)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	m_pObjectsManager->deleteBuffers(this, n, buffers);
	return;
}

void CoreContext::CoreContext::deleteFramebuffers(GLsizei n, const GLuint* framebuffers)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	m_pObjectsManager->deleteFramebuffers(this, n, framebuffers);
	return;
}

void CoreContext::deleteProgram(GLuint program)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	m_pObjectsManager->deleteProgram(this, program);
	return;
}

void CoreContext::deleteRenderbuffers(GLsizei n, const GLuint* renderbuffers)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	m_pObjectsManager->deleteRenderbuffers(this, n, renderbuffers);
	return;
}

void CoreContext::deleteShader(GLuint shader)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	m_pObjectsManager->deleteShader(this, shader);
	return;
}

void CoreContext::deleteTextures(GLsizei n, const GLuint* textures)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	m_pObjectsManager->deleteTextures(this, n, textures);
	return;
}

void CoreContext::depthFunc(GLenum func)
{
	m_state.setDepthFunc(func);
	return;
}

void CoreContext::depthMask(GLboolean flag)
{
	m_state.setDepthWritemask(flag);
	return;
}

void CoreContext::depthRangef(GLfloat n, GLfloat f)
{
	m_state.setDepthRange(n, f);
	return;
}

void CoreContext::detachShader(GLuint program, GLuint shader)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreProgram* core_program = m_pObjectsManager->getProgram(program);
	if (core_program == nullptr) {
		// GL_INVALID_OPERATION is generated if program is not a program object.
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	CoreShader* core_shader = m_pObjectsManager->getShader(shader);
	if (core_program == nullptr) {
		// GL_INVALID_OPERATION is generated if shader is not a shader object.
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_program->detachShader(this, core_shader);
	return;
}

void CoreContext::disable(GLenum cap)
{
	m_state.setEnable(cap, GL_FALSE);
	return;
}

void CoreContext::disableVertexAttribArray(GLuint index)
{
	CoreVertexArray* core_vertex_array = m_state.getVertexArray();
	if (core_vertex_array != nullptr) {
		core_vertex_array->setEnableVertexAttrib(index, GL_FALSE);
	} else {
		m_state.setEnableVertexAttrib(index, GL_FALSE);
	}
	return;
}

void CoreContext::drawArrays(GLenum mode, GLint first, GLsizei count)
{
	if (m_pBackendContext == nullptr) {
		return;
	}
	// サンプラとテクスチャを準備
	m_state.setupTextureSampler(this);
	// インスタンス描画を無効に設定
	m_state.setInstancedRendering(false);
	// 描画
	m_pBackendContext->drawArrays(mode, first, count, &m_state.getDrawParameters(), &m_state.getClearParams());
	// GLステートのダーティをクリア
	m_state.clearDrawParameterDirtyFlags();
	return;
}

void CoreContext::drawElements(GLenum mode, GLsizei count, GLenum type, const void* indices)
{
	if (m_pBackendContext == nullptr) {
		return;
	}
	if (!m_state.elementArrayBufferAvailable()) {
		// Element Array Buffer がバインドされていない（クライアントメモリを使用する）描画はサポートしない
		return;
	}
	// サンプラとテクスチャを準備
	m_state.setupTextureSampler(this);
	// インスタンス描画を無効に設定
	m_state.setInstancedRendering(false);
	// 描画
	m_pBackendContext->drawElements(mode, count, type, indices, &m_state.getDrawParameters(), &m_state.getClearParams());
	// GLステートのダーティをクリア
	m_state.clearDrawParameterDirtyFlags();
	return;
}

void CoreContext::enable(GLenum cap)
{
	m_state.setEnable(cap, GL_TRUE);
	return;
}

void CoreContext::enableVertexAttribArray(GLuint index)
{
	CoreVertexArray* core_vertex_array = m_state.getVertexArray();
	if (core_vertex_array != nullptr) {
		core_vertex_array->setEnableVertexAttrib(index, GL_TRUE);
	} else {
		m_state.setEnableVertexAttrib(index, GL_TRUE);
	}
	return;
}

void CoreContext::finish(void)
{
	if (m_pBackendContext != nullptr) {
		m_pBackendContext->finish();
	}
	return;
}

void CoreContext::flush(void)
{
	if (m_pBackendContext != nullptr) {
		m_pBackendContext->flush();
	}
	return;
}

void CoreContext::framebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	if (renderbuffertarget != GL_RENDERBUFFER) {
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	CoreFramebuffer* core_framebuffer = m_state.getFramebuffer(target);
	if (core_framebuffer == nullptr) {
		// GL_INVALID_OPERATION is generated if zero is bound to target.
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	CoreRenderbuffer* core_renderbuffer = m_pObjectsManager->getRenderbuffer(renderbuffer);
	if (core_renderbuffer == nullptr) {
		// GL_INVALID_ENUM is generated if renderbuffertarget is not GL_RENDERBUFFER.
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	core_framebuffer->setRenderbuffer(this, attachment, core_renderbuffer);
	// 事前に設定されていたattachment情報の反映
	m_state.setFramebuffer(this, target, core_framebuffer);
	return;
}

void CoreContext::framebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreFramebuffer* core_framebuffer = m_state.getFramebuffer(target);
	if (core_framebuffer == nullptr) {
		// GL_INVALID_OPERATION is generated if zero is bound to target.
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	CoreTexture* core_texture = m_pObjectsManager->getTexture(texture);
	if (core_texture == nullptr) {
		// GL_INVALID_OPERATION is generated if textarget and texture are not compatible.
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_framebuffer->setTexture2d(this, attachment, core_texture, textarget, level);
	// 事前に設定されていたattachment情報の反映
	m_state.setFramebuffer(this, target, core_framebuffer);
	return;
}

void CoreContext::frontFace(GLenum mode)
{
	m_state.setFrontFace(mode);
	return;
}

void CoreContext::genBuffers(GLsizei n, GLuint* buffers)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	m_pObjectsManager->genBuffers(this, n, buffers);
	return;
}

void CoreContext::generateMipmap(GLenum target)
{
	CoreTexture* core_texture = m_state.getTexture(target);
	if (core_texture == nullptr) {
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_texture->generateMipmap(this);
	return;
}

void CoreContext::genFramebuffers(GLsizei n, GLuint* framebuffers)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	m_pObjectsManager->genFramebuffers(this, n, framebuffers);
	return;
}

void CoreContext::genRenderbuffers(GLsizei n, GLuint* renderbuffers)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	m_pObjectsManager->genRenderbuffers(this, n, renderbuffers);
	return;
}

void CoreContext::genTextures(GLsizei n, GLuint* textures)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	m_pObjectsManager->genTextures(n, textures);
	return;
}

void CoreContext::getActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreProgram* core_program = m_pObjectsManager->getProgram(program);
	if (core_program == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_program->getActiveAttrib(index, bufSize, length, size, type, name);
	return;
}

void CoreContext::getActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreProgram* core_program = m_pObjectsManager->getProgram(program);
	if (core_program == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_program->getActiveUniform(index, bufSize, length, size, type, name);
	return;
}

void CoreContext::getAttachedShaders(GLuint program, GLsizei maxCount, GLsizei* count, GLuint* shaders)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreProgram* core_program = m_pObjectsManager->getProgram(program);
	if (core_program == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_program->getAttachedShaders(maxCount, count, shaders);
	return;
}

GLint CoreContext::getAttribLocation(GLuint program, const GLchar* name)
{
	if (m_pObjectsManager == nullptr) {
		return -1;
	}
	CoreProgram* core_program = m_pObjectsManager->getProgram(program);
	if (core_program == nullptr) {
		return -1;
	}
	return core_program->getAttribLocation(name);
}

void CoreContext::getBooleanv(GLenum pname, GLboolean* data)
{
	AXGL_ASSERT(data != nullptr);
	AXGL_ASSERT(m_pBackendContext != nullptr);
	// MOTE: 全ステートが、GL_TRUE or GL_FALSE に変換して取得される仕様
	if ((pname >= GL_DRAW_BUFFER0) && (pname < (GL_DRAW_BUFFER0 + AXGL_MAX_DRAW_BUFFERS))) {
		// GL_DRAW_BUFFERx
		GLenum buf = m_state.getDrawBuffer(pname);
		*data = GLINT_TO_GLBOOLEAN(buf);
	} else {
		// general
		switch (pname) {
		case GL_ACTIVE_TEXTURE:
			*data = GLINT_TO_GLBOOLEAN(m_state.getActiveTexture());
			break;
		case GL_ALIASED_LINE_WIDTH_RANGE:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			data[0] = GLFLOAT_TO_GLBOOLEAN(params.aliasedLineWidthRange[0]);
			data[1] = GLFLOAT_TO_GLBOOLEAN(params.aliasedLineWidthRange[1]);
		}
		break;
		case GL_ALIASED_POINT_SIZE_RANGE:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			data[0] = GLFLOAT_TO_GLBOOLEAN(params.aliasedPointSizeRange[0]);
			data[1] = GLFLOAT_TO_GLBOOLEAN(params.aliasedPointSizeRange[1]);
		}
		break;
		case GL_ALPHA_BITS:
		{
			GLenum format = getDrawFramebufferFormat(0);
			int alpha_bits = getAlphaBitsFromFormat(format);
			*data = GLINT_TO_GLBOOLEAN(alpha_bits);
		}
		break;
		case GL_ARRAY_BUFFER_BINDING:
		{
			CoreBuffer* core_buffer = m_state.getBuffer(GL_ARRAY_BUFFER);
			if (core_buffer != nullptr) {
				*data = GL_TRUE; // GL仕様上、非0なのでGL_TRUE
			} else {
				*data = GL_FALSE;
			}
		}
		break;
		case GL_BLEND:
			*data = m_state.getEnable(GL_BLEND);
			break;
		case GL_BLEND_COLOR:
		{
			const BlendParams& params = m_state.getBlendParams();
			data[0] = GLFLOAT_TO_GLBOOLEAN(params.blendColor[0]);
			data[1] = GLFLOAT_TO_GLBOOLEAN(params.blendColor[1]);
			data[2] = GLFLOAT_TO_GLBOOLEAN(params.blendColor[2]);
			data[3] = GLFLOAT_TO_GLBOOLEAN(params.blendColor[3]);
		}
		break;
		case GL_BLEND_DST_ALPHA:
		{
			const BlendParams& params = m_state.getBlendParams();
			*data = GLINT_TO_GLBOOLEAN(params.blendDst[1]);
		}
		break;
		case GL_BLEND_DST_RGB:
		{
			const BlendParams& params = m_state.getBlendParams();
			*data = GLINT_TO_GLBOOLEAN(params.blendDst[0]);
		}
		break;
		case GL_BLEND_EQUATION_ALPHA:
		{
			const BlendParams& params = m_state.getBlendParams();
			*data = GLINT_TO_GLBOOLEAN(params.blendEquation[1]);
		}
		break;
		case GL_BLEND_EQUATION_RGB:
		{
			const BlendParams& params = m_state.getBlendParams();
			*data = GLINT_TO_GLBOOLEAN(params.blendEquation[0]);
		}
		break;
		case GL_BLEND_SRC_ALPHA:
		{
			const BlendParams& params = m_state.getBlendParams();
			*data = GLINT_TO_GLBOOLEAN(params.blendSrc[1]);
		}
		break;
		case GL_BLEND_SRC_RGB:
		{
			const BlendParams& params = m_state.getBlendParams();
			*data = GLINT_TO_GLBOOLEAN(params.blendSrc[0]);
		}
		break;
		case GL_BLUE_BITS:
		{
			GLenum format = getDrawFramebufferFormat(0);
			int blue_bits = getBlueBitsFromFormat(format);
			*data = GLINT_TO_GLBOOLEAN(blue_bits);
		}
		break;
		case GL_COLOR_CLEAR_VALUE:
		{
			const ClearParameters& params = m_state.getClearParams();
			data[0] = GLFLOAT_TO_GLBOOLEAN(params.colorClearValue[0][0]);
			data[1] = GLFLOAT_TO_GLBOOLEAN(params.colorClearValue[0][1]);
			data[2] = GLFLOAT_TO_GLBOOLEAN(params.colorClearValue[0][2]);
			data[3] = GLFLOAT_TO_GLBOOLEAN(params.colorClearValue[0][3]);
		}
		break;
		case GL_COLOR_WRITEMASK:
		{
			const ColorWritemaskParams& params = m_state.getColorWritemaskParams();
			data[0] = params.colorWritemask[0];
			data[1] = params.colorWritemask[1];
			data[2] = params.colorWritemask[2];
			data[3] = params.colorWritemask[3];
		}
		break;
		case GL_COMPRESSED_TEXTURE_FORMATS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			for (int i = 0; i < params.numCompressedTextureFormats; i++) {
				data[i] = GLINT_TO_GLBOOLEAN(params.compressedTextureFormats[i]);
			}
		}
		break;
		case GL_COPY_READ_BUFFER_BINDING:
		{
			CoreBuffer* core_buffer = m_state.getBuffer(GL_COPY_READ_BUFFER);
			if (core_buffer != nullptr) {
				*data = GL_TRUE; // GL仕様上、非0なのでGL_TRUE
			} else {
				*data = GL_FALSE;
			}
		}
		break;
		case GL_COPY_WRITE_BUFFER_BINDING:
		{
			CoreBuffer* core_buffer = m_state.getBuffer(GL_COPY_WRITE_BUFFER);
			if (core_buffer != nullptr) {
				*data = GL_TRUE; // GL仕様上、非0なのでGL_TRUE
			} else {
				*data = GL_FALSE;
			}
		}
		break;
		case GL_CULL_FACE:
		{
			const CullFaceParams& params = m_state.getCullFaceParams();
			*data = params.cullFaceEnable;
		}
		break;
		case GL_CULL_FACE_MODE:
		{
			const CullFaceParams& params = m_state.getCullFaceParams();
			*data = GLINT_TO_GLBOOLEAN(params.cullFaceMode);
		}
		break;
		case GL_CURRENT_PROGRAM:
		{
			CoreProgram* core_program = m_state.getProgram();
			if (core_program != nullptr) {
				*data = GL_TRUE; // GL仕様上、非0なのでGL_TRUE
			} else {
				*data = GL_FALSE;
			}
		}
		break;
		case GL_DEPTH_BITS:
		{
			GLenum format = getDrawDepthbufferFormat();
			int depth_bits = getDepthBitsFromFormat(format);
			*data = GLINT_TO_GLBOOLEAN(depth_bits);
		}
		break;
		case GL_DEPTH_CLEAR_VALUE:
		{
			const ClearParameters& params = m_state.getClearParams();
			*data = GLFLOAT_TO_GLBOOLEAN(params.depthClearValue);
		}
		break;
		case GL_DEPTH_FUNC:
		{
			const DepthTestParams& params = m_state.getDepthTestParams();
			*data = GLINT_TO_GLBOOLEAN(params.depthFunc);
		}
		break;
		case GL_DEPTH_RANGE:
		{
			const ViewportParams& params = m_state.getViewportParams();
			data[0] = GLFLOAT_TO_GLBOOLEAN(params.depthRange[0]);
			data[1] = GLFLOAT_TO_GLBOOLEAN(params.depthRange[1]);
		}
		break;
		case GL_DEPTH_TEST:
		{
			const DepthTestParams& params = m_state.getDepthTestParams();
			*data = params.depthTestEnable;
		}
		break;
		case GL_DEPTH_WRITEMASK:
		{
			const DepthStencilWritemaskParams& params = m_state.getDepthStencilWritemaskParams();
			*data = params.depthWritemask;
		}
		break;
		case GL_DITHER:
			*data = m_state.getEnable(GL_DITHER);
			break;
		case GL_DRAW_FRAMEBUFFER_BINDING:
		{
			CoreFramebuffer* core_framebuffer = m_state.getFramebuffer(GL_DRAW_FRAMEBUFFER);
			if (core_framebuffer != nullptr) {
				*data = GL_TRUE; // GL仕様上、非0なのでGL_TRUE
			} else {
				*data = GL_FALSE;
			}
		}
		break;
		case GL_ELEMENT_ARRAY_BUFFER_BINDING:
		{
			CoreBuffer* core_buffer = m_state.getBuffer(GL_ELEMENT_ARRAY_BUFFER);
			if (core_buffer != nullptr) {
				*data = GL_TRUE; // GL仕様上、非0なのでGL_TRUE
			} else {
				*data = GL_FALSE;
			}
		}
		break;
		case GL_FRAGMENT_SHADER_DERIVATIVE_HINT:
			*data = static_cast<GLboolean>(m_state.getHint(GL_FRAGMENT_SHADER_DERIVATIVE_HINT));
			break;
		case GL_FRONT_FACE:
		{
			const CullFaceParams& params = m_state.getCullFaceParams();
			*data = GLINT_TO_GLBOOLEAN(params.frontFace);
		}
		break;
		case GL_GENERATE_MIPMAP_HINT:
			*data = static_cast<GLboolean>(m_state.getHint(GL_GENERATE_MIPMAP_HINT));
			break;
		case GL_GREEN_BITS:
		{
			GLenum format = getDrawFramebufferFormat(0);
			int green_bits = getGreenBitsFromFormat(format);
			*data = GLINT_TO_GLBOOLEAN(green_bits);
		}
		break;
		case GL_IMPLEMENTATION_COLOR_READ_FORMAT:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.implementationColorReadFormat);
		}
		break;
		case GL_IMPLEMENTATION_COLOR_READ_TYPE:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.implementationColorReadType);
		}
		break;
		case GL_LINE_WIDTH:
		{
			float line_width = m_state.getLineWidth();
			*data = GLFLOAT_TO_GLBOOLEAN(line_width);
		}
		break;
		case GL_MAJOR_VERSION:
			*data = 3;
			break;
		case GL_MAX_3D_TEXTURE_SIZE:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.max3dTextureSize);
		}
		break;
		case GL_MAX_ARRAY_TEXTURE_LAYERS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.maxArrayTextureLayers);
		}
		break;
		case GL_MAX_COLOR_ATTACHMENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.maxColorAttachments);
		}
		break;
		case GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.maxCombinedFragmentUniformComponents);
		}
		break;
		case GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.maxCombinedTextureImageUnits);
		}
		break;
		case GL_MAX_COMBINED_UNIFORM_BLOCKS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.maxCombinedUniformBlocks);
		}
		break;
		case GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.maxCombinedVertexUniformComponents);
		}
		break;
		case GL_MAX_CUBE_MAP_TEXTURE_SIZE:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.maxCubeMapTextureSize);
		}
		break;
		case GL_MAX_DRAW_BUFFERS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.maxDrawBuffers);
		}
		break;
		case GL_MAX_ELEMENT_INDEX:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.maxElementIndex);
		}
		break;
		case GL_MAX_ELEMENTS_INDICES:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.maxElementsIndices);
		}
		break;
		case GL_MAX_ELEMENTS_VERTICES:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.maxElementsVertices);
		}
		break;
		case GL_MAX_FRAGMENT_INPUT_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.maxFragmentInputComponents);
		}
		break;
		case GL_MAX_FRAGMENT_UNIFORM_BLOCKS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.maxFragmentUniformBlocks);
		}
		break;
		case GL_MAX_FRAGMENT_UNIFORM_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.maxFragmentUniformComponents);
		}
		break;
		case GL_MAX_FRAGMENT_UNIFORM_VECTORS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.maxFragmentUniformVectors);
		}
		break;
		case GL_MAX_PROGRAM_TEXEL_OFFSET:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.maxProgramTexelOffset);
		}
		break;
		case GL_MAX_RENDERBUFFER_SIZE:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.maxRenderbufferSize);
		}
		break;
		case GL_MAX_SAMPLES:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.maxSamples);
		}
		break;
		case GL_MAX_SERVER_WAIT_TIMEOUT:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.maxServerWaitTimeout);
		}
		break;
		case GL_MAX_TEXTURE_IMAGE_UNITS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.maxTextureImageUnits);
		}
		break;
		case GL_MAX_TEXTURE_LOD_BIAS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLFLOAT_TO_GLBOOLEAN(params.maxTextureLodBias);
		}
		break;
		case GL_MAX_TEXTURE_SIZE:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.maxTextureSize);
		}
		break;
		case GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.maxTransformFeedbackInterleavedComponents);
		}
		break;
		case GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.maxTransformFeedbackSeparateAttribs);
		}
		break;
		case GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.maxTransformFeedbackSeparateComponents);
		}
		break;
		case GL_MAX_UNIFORM_BLOCK_SIZE:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.maxUniformBlockSize);
		}
		break;
		case GL_MAX_UNIFORM_BUFFER_BINDINGS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.maxUniformBufferBindings);
		}
		break;
		case GL_MAX_VARYING_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.maxVaryingComponents);
		}
		break;
		case GL_MAX_VARYING_VECTORS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.maxVaryingVectors);
		}
		break;
		case GL_MAX_VERTEX_ATTRIBS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.maxVertexAttribs);
		}
		break;
		case GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.maxVertexTextureImageUnits);
		}
		break;
		case GL_MAX_VERTEX_OUTPUT_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.maxVertexOutputComponents);
		}
		break;
		case GL_MAX_VERTEX_UNIFORM_BLOCKS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.maxVertexUniformBlocks);
		}
		break;
		case GL_MAX_VERTEX_UNIFORM_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.maxVertexUniformComponents);
		}
		break;
		case GL_MAX_VERTEX_UNIFORM_VECTORS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.maxVertexUniformVectors);
		}
		break;
		case GL_MAX_VIEWPORT_DIMS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			data[0] = GLINT_TO_GLBOOLEAN(params.maxViewportDims[0]);
			data[1] = GLINT_TO_GLBOOLEAN(params.maxViewportDims[1]);
		}
		break;
		case GL_MIN_PROGRAM_TEXEL_OFFSET:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.minProgramTexelOffset);
		}
		break;
		case GL_MINOR_VERSION:
			*data = 0;
			break;
		case GL_NUM_COMPRESSED_TEXTURE_FORMATS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.numCompressedTextureFormats);
		}
		break;
		case GL_NUM_EXTENSIONS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.numExtensions);
		}
		break;
		case GL_NUM_PROGRAM_BINARY_FORMATS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.numProgramBinaryFormats);
		}
		break;
		case GL_NUM_SHADER_BINARY_FORMATS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.numShaderBinaryFormats);
		}
		break;
		case GL_PACK_ALIGNMENT:
		{
			const CoreState::PackParams& params = m_state.getPackParams();
			*data = GLINT_TO_GLBOOLEAN(params.packAlignment);
		}
		break;
		case GL_PACK_ROW_LENGTH:
		{
			const CoreState::PackParams& params = m_state.getPackParams();
			*data = GLINT_TO_GLBOOLEAN(params.packRowLength);
		}
		break;
		case GL_PACK_SKIP_PIXELS:
		{
			const CoreState::PackParams& params = m_state.getPackParams();
			*data = GLINT_TO_GLBOOLEAN(params.packSkipPixels);
		}
		break;
		case GL_PACK_SKIP_ROWS:
		{
			const CoreState::PackParams& params = m_state.getPackParams();
			*data = GLINT_TO_GLBOOLEAN(params.packSkipRows);
		}
		break;
		case GL_PIXEL_PACK_BUFFER_BINDING:
		{
			CoreBuffer* core_buffer = m_state.getBuffer(GL_PIXEL_PACK_BUFFER);
			if (core_buffer != nullptr) {
				*data = GL_TRUE; // GL仕様上、非0なのでGL_TRUE
			} else {
				*data = GL_FALSE;
			}
		}
		break;
		case GL_PIXEL_UNPACK_BUFFER_BINDING:
		{
			CoreBuffer* core_buffer = m_state.getBuffer(GL_PIXEL_UNPACK_BUFFER);
			if (core_buffer != nullptr) {
				*data = GL_TRUE; // GL仕様上、非0なのでGL_TRUE
			} else {
				*data = GL_FALSE;
			}
		}
		break;
		case GL_POLYGON_OFFSET_FACTOR:
		{
			const PolygonOffsetParams& params = m_state.getPolygonOffsetParams();
			*data = GLFLOAT_TO_GLBOOLEAN(params.polygonOffsetFactor);
		}
		break;
		case GL_POLYGON_OFFSET_FILL:
		{
			const PolygonOffsetParams& params = m_state.getPolygonOffsetParams();
			*data = params.polygonOffsetFillEnable;
		}
		break;
		case GL_POLYGON_OFFSET_UNITS:
		{
			const PolygonOffsetParams& params = m_state.getPolygonOffsetParams();
			*data = GLFLOAT_TO_GLBOOLEAN(params.polygonOffsetUnits);
		}
		break;
		case GL_PRIMITIVE_RESTART_FIXED_INDEX:
			*data = m_state.getEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
			break;
		case GL_PROGRAM_BINARY_FORMATS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			for (int i = 0; i < params.numProgramBinaryFormats; i++) {
				data[i] = GLINT_TO_GLBOOLEAN(params.programBinaryFormats[i]);
			}
		}
		break;
		case GL_RASTERIZER_DISCARD:
			*data = m_state.getEnable(GL_RASTERIZER_DISCARD);
			break;
		case GL_READ_BUFFER:
		{
			GLenum read_buffer = m_state.getReadBuffer();
			*data = GLINT_TO_GLBOOLEAN(read_buffer);
		}
		break;
		case GL_READ_FRAMEBUFFER_BINDING:
		{
			CoreFramebuffer* core_framebuffer = m_state.getFramebuffer(GL_READ_FRAMEBUFFER);
			if (core_framebuffer != nullptr) {
				*data = GL_TRUE; // GL仕様上、非0なのでGL_TRUE
			} else {
				*data = GL_FALSE;
			}
		}
		break;
		case GL_RED_BITS:
		{
			GLenum format = getDrawFramebufferFormat(0);
			int red_bits = getRedBitsFromFormat(format);
			*data = GLINT_TO_GLBOOLEAN(red_bits);
		}
		break;
		case GL_RENDERBUFFER_BINDING:
		{
			CoreRenderbuffer* core_renderbuffer = m_state.getRenderbuffer(GL_RENDERBUFFER);
			if (core_renderbuffer != nullptr) {
				*data = GL_TRUE; // GL仕様上、非0なのでGL_TRUE
			} else {
				*data = GL_FALSE;
			}
		}
		break;
		case GL_SAMPLE_ALPHA_TO_COVERAGE:
		{
			const SampleCoverageParams& params = m_state.getSampleCoverageParams();
			*data = params.sampleAlphaToCoverageEnable;
		}
		break;
		case GL_SAMPLE_BUFFERS:
		{
			// TODO: Framebuffer のマルチサンプル数
		}
		break;
		case GL_SAMPLE_COVERAGE:
		{
			const SampleCoverageParams& params = m_state.getSampleCoverageParams();
			*data = params.sampleCoverageEnable;
		}
		break;
		case GL_SAMPLE_COVERAGE_INVERT:
		{
			const SampleCoverageParams& params = m_state.getSampleCoverageParams();
			*data = params.sampleCoverageInvert;
		}
		break;
		case GL_SAMPLE_COVERAGE_VALUE:
		{
			const SampleCoverageParams& params = m_state.getSampleCoverageParams();
			*data = GLFLOAT_TO_GLBOOLEAN(params.sampleCoverageValue);
		}
		break;
		case GL_SAMPLER_BINDING:
		{
			GLenum active_texture = m_state.getActiveTexture();
			GLuint unit_index = active_texture - GL_TEXTURE0;
			CoreSampler* core_sampler = m_state.getSampler(unit_index);
			if (core_sampler != nullptr) {
				*data = GL_TRUE; // GL仕様上、非0なのでGL_TRUE
			} else {
				*data = GL_FALSE;
			}
		}
		break;
		case GL_SAMPLES:
			// TODO: framebuffer のサンプル数
			break;
		case GL_SCISSOR_BOX:
		{
			const ScissorParams& params = m_state.getScissorParams();
			data[0] = GLINT_TO_GLBOOLEAN(params.scissorBox[0]);
			data[1] = GLINT_TO_GLBOOLEAN(params.scissorBox[1]);
			data[2] = GLINT_TO_GLBOOLEAN(params.scissorBox[2]);
			data[3] = GLINT_TO_GLBOOLEAN(params.scissorBox[3]);
		}
		break;
		case GL_SCISSOR_TEST:
		{
			const ScissorParams& params = m_state.getScissorParams();
			*data = params.scissorTestEnable;
		}
		break;
		case GL_SHADER_BINARY_FORMATS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			for (int i = 0; i < params.numShaderBinaryFormats; i++) {
				data[i] = GLINT_TO_GLBOOLEAN(params.shaderBinaryFormats[i]);
			}
		}
		break;
		case GL_SHADER_COMPILER:
			*data = GL_TRUE; // 常にGL_TRUE
			break;
		case GL_STENCIL_BACK_FAIL:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = GLINT_TO_GLBOOLEAN(params.stencilBackFail);
		}
		break;
		case GL_STENCIL_BACK_FUNC:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = GLINT_TO_GLBOOLEAN(params.stencilBackFunc);
		}
		break;
		case GL_STENCIL_BACK_PASS_DEPTH_FAIL:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = GLINT_TO_GLBOOLEAN(params.stencilBackPassDepthFail);
		}
		break;
		case GL_STENCIL_BACK_PASS_DEPTH_PASS:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = GLINT_TO_GLBOOLEAN(params.stencilBackPassDepthPass);
		}
		break;
		case GL_STENCIL_BACK_REF:
		{
			const StencilReference& st_ref = m_state.getStencilReference();
			*data = GLINT_TO_GLBOOLEAN(st_ref.stencilBackRef);
		}
		break;
		case GL_STENCIL_BACK_VALUE_MASK:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = GLINT_TO_GLBOOLEAN(params.stencilBackValueMask);
		}
		break;
		case GL_STENCIL_BACK_WRITEMASK:
		{
			const DepthStencilWritemaskParams& params = m_state.getDepthStencilWritemaskParams();
			*data = GLINT_TO_GLBOOLEAN(params.stencilBackWritemask);
		}
		break;
		case GL_STENCIL_BITS:
		{
			GLenum format = getDrawFramebufferFormat(0);
			int green_bits = getGreenBitsFromFormat(format);
			*data = GLINT_TO_GLBOOLEAN(green_bits);
		}
		break;
		case GL_STENCIL_CLEAR_VALUE:
		{
			const ClearParameters& params = m_state.getClearParams();
			*data = GLINT_TO_GLBOOLEAN(params.stencilClearValue);
		}
		break;
		case GL_STENCIL_FAIL:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = GLINT_TO_GLBOOLEAN(params.stencilFail);
		}
		break;
		case GL_STENCIL_FUNC:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = GLINT_TO_GLBOOLEAN(params.stencilFunc);
		}
		break;
		case GL_STENCIL_PASS_DEPTH_FAIL:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = GLINT_TO_GLBOOLEAN(params.stencilPassDepthFail);
		}
		break;
		case GL_STENCIL_PASS_DEPTH_PASS:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = GLINT_TO_GLBOOLEAN(params.stencilPassDepthPass);
		}
		break;
		case GL_STENCIL_REF:
		{
			const StencilReference& st_ref = m_state.getStencilReference();
			*data = GLINT_TO_GLBOOLEAN(st_ref.stencilRef);
		}
		break;
		case GL_STENCIL_TEST:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = params.stencilTestEnable;
		}
		break;
		case GL_STENCIL_VALUE_MASK:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = GLINT_TO_GLBOOLEAN(params.stencilValueMask);
		}
		break;
		case GL_STENCIL_WRITEMASK:
		{
			const DepthStencilWritemaskParams& params = m_state.getDepthStencilWritemaskParams();
			*data = GLINT_TO_GLBOOLEAN(params.stencilWritemask);
		}
		break;
		case GL_SUBPIXEL_BITS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.subpixelBits);
		}
		break;
		case GL_TEXTURE_BINDING_2D:
		{
			CoreTexture* core_texture = m_state.getTexture(GL_TEXTURE_2D);
			if (core_texture != nullptr) {
				*data = GL_TRUE; // GL仕様上、非0なのでGL_TRUE
			} else {
				*data = GL_FALSE;
			}
		}
		break;
		case GL_TEXTURE_BINDING_2D_ARRAY:
		{
			CoreTexture* core_texture = m_state.getTexture(GL_TEXTURE_2D_ARRAY);
			if (core_texture != nullptr) {
				*data = GL_TRUE; // GL仕様上、非0なのでGL_TRUE
			} else {
				*data = GL_FALSE;
			}
		}
		break;
		case GL_TEXTURE_BINDING_3D:
		{
			CoreTexture* core_texture = m_state.getTexture(GL_TEXTURE_3D);
			if (core_texture != nullptr) {
				*data = GL_TRUE; // GL仕様上、非0なのでGL_TRUE
			} else {
				*data = GL_FALSE;
			}
		}
		break;
		case GL_TEXTURE_BINDING_CUBE_MAP:
		{
			CoreTexture* core_texture = m_state.getTexture(GL_TEXTURE_CUBE_MAP);
			if (core_texture != nullptr) {
				*data = GL_TRUE; // GL仕様上、非0なのでGL_TRUE
			} else {
				*data = GL_FALSE;
			}
		}
		break;
		case GL_TRANSFORM_FEEDBACK_BINDING:
		{
			CoreTransformFeedback* core_transform_feedback = m_state.getTransformFeedback(GL_TRANSFORM_FEEDBACK);
			if (core_transform_feedback != nullptr) {
				*data = GL_TRUE; // GL仕様上、非0なのでGL_TRUE
			} else {
				*data = GL_FALSE;
			}
		}
		break;
		case GL_TRANSFORM_FEEDBACK_ACTIVE:
		{
			const CoreState::TransformFeedbackParams& params = m_state.getTransformFeedbackParams();
			*data = params.transformFeedbackActive;
		}
		break;
		case GL_TRANSFORM_FEEDBACK_BUFFER_BINDING:
		{
			const IndexedBuffer* buffer = m_state.getIndexedBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);
			if ((buffer != nullptr) && (buffer->buffer != nullptr)) {
				*data = GL_TRUE; // GL仕様上、非0なのでGL_TRUE
			} else {
				*data = GL_FALSE;
			}
		}
		break;
		case GL_TRANSFORM_FEEDBACK_PAUSED:
		{
			const CoreState::TransformFeedbackParams& params = m_state.getTransformFeedbackParams();
			*data = params.transformFeedbackPaused;
		}
		break;
		case GL_TRANSFORM_FEEDBACK_BUFFER_SIZE:
		{
			const IndexedBuffer* buffer = m_state.getIndexedBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);
			if (buffer != nullptr) {
				*data = GLINT_TO_GLBOOLEAN(buffer->size);
			} else {
				*data = GL_FALSE;
			}
		}
		break;
		case GL_TRANSFORM_FEEDBACK_BUFFER_START:
		{
			const IndexedBuffer* buffer = m_state.getIndexedBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);
			if (buffer != nullptr) {
				*data = GLINT_TO_GLBOOLEAN(buffer->offset);
			} else {
				*data = GL_FALSE;
			}
		}
		break;
		case GL_UNIFORM_BUFFER_BINDING:
		{
			CoreBuffer* core_buffer = m_state.getBuffer(GL_UNIFORM_BUFFER);
			if (core_buffer != nullptr) {
				*data = GL_TRUE; // GL仕様上、非0なのでGL_TRUE
			} else {
				*data = GL_FALSE;
			}
		}
		break;
		case GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = GLINT_TO_GLBOOLEAN(params.uniformBufferOffsetAlignment);
		}
		break;
		case GL_UNIFORM_BUFFER_SIZE:
		{
			const IndexedBuffer* buffer = m_state.getIndexedBuffer(GL_UNIFORM_BUFFER, 0);
			if (buffer != nullptr) {
				*data = GLINT_TO_GLBOOLEAN(buffer->size);
			} else {
				*data = GL_FALSE;
			}
		}
		break;
		case GL_UNIFORM_BUFFER_START:
		{
			const IndexedBuffer* buffer = m_state.getIndexedBuffer(GL_UNIFORM_BUFFER, 0);
			if (buffer != nullptr) {
				*data = GLINT_TO_GLBOOLEAN(buffer->offset);
			} else {
				*data = GL_FALSE;
			}
		}
		break;
		case GL_UNPACK_ALIGNMENT:
		{
			const CoreState::UnpackParams& params = m_state.getUnpackParams();
			*data = GLINT_TO_GLBOOLEAN(params.unpackAlignment);
		}
		break;
		case GL_UNPACK_IMAGE_HEIGHT:
		{
			const CoreState::UnpackParams& params = m_state.getUnpackParams();
			*data = GLINT_TO_GLBOOLEAN(params.unpackImageHeight);
		}
		break;
		case GL_UNPACK_ROW_LENGTH:
		{
			const CoreState::UnpackParams& params = m_state.getUnpackParams();
			*data = GLINT_TO_GLBOOLEAN(params.unpackRowLength);
		}
		break;
		case GL_UNPACK_SKIP_IMAGES:
		{
			const CoreState::UnpackParams& params = m_state.getUnpackParams();
			*data = GLINT_TO_GLBOOLEAN(params.unpackSkipImages);
		}
		break;
		case GL_UNPACK_SKIP_PIXELS:
		{
			const CoreState::UnpackParams& params = m_state.getUnpackParams();
			*data = GLINT_TO_GLBOOLEAN(params.unpackSkipPixels);
		}
		break;
		case GL_UNPACK_SKIP_ROWS:
		{
			const CoreState::UnpackParams& params = m_state.getUnpackParams();
			*data = GLINT_TO_GLBOOLEAN(params.unpackSkipRows);
		}
		break;
		case GL_VERTEX_ARRAY_BINDING:
		{
			CoreVertexArray* core_vertex_array = m_state.getVertexArray();
			if (core_vertex_array != nullptr) {
				*data = GL_TRUE; // GL仕様上、非0なのでGL_TRUE
			} else {
				*data = GL_FALSE;
			}
		}
		break;
		case GL_VIEWPORT:
		{
			const ViewportParams& params = m_state.getViewportParams();
			data[0] = GLINT_TO_GLBOOLEAN(params.viewport[0]);
			data[1] = GLINT_TO_GLBOOLEAN(params.viewport[1]);
			data[2] = GLINT_TO_GLBOOLEAN(params.viewport[2]);
			data[3] = GLINT_TO_GLBOOLEAN(params.viewport[3]);
		}
		break;
		default:
			// GL_INVALID_ENUM
			setErrorCode(GL_INVALID_ENUM);
			break;
		}
	}
	return;
}

void CoreContext::getBufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
	CoreBuffer* core_buffer = m_state.getBuffer(target);
	if (core_buffer == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_buffer->getBufferParameteriv(this, pname, params);
	return;
}

GLenum CoreContext::getError(void)
{
	GLenum rval = m_errorCode;
	// NOTE: 取得でエラーはクリア、GLの仕様
	m_errorCode = GL_NO_ERROR;
	return rval;
}

void CoreContext::getFloatv(GLenum pname, GLfloat* data)
{
	AXGL_ASSERT(data != nullptr);
	AXGL_ASSERT(m_pBackendContext != nullptr);
	if ((pname >= GL_DRAW_BUFFER0) && (pname < (GL_DRAW_BUFFER0 + AXGL_MAX_DRAW_BUFFERS))) {
		// GL_DRAW_BUFFERx
		GLenum buf = m_state.getDrawBuffer(pname);
		*data = static_cast<GLfloat>(buf);
	} else {
		// general
		switch (pname) {
		case GL_ACTIVE_TEXTURE:
			*data = static_cast<GLfloat>(m_state.getActiveTexture());
			break;
		case GL_ALIASED_LINE_WIDTH_RANGE:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			data[0] = params.aliasedLineWidthRange[0];
			data[1] = params.aliasedLineWidthRange[1];
		}
		break;
		case GL_ALIASED_POINT_SIZE_RANGE:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			data[0] = params.aliasedPointSizeRange[0];
			data[1] = params.aliasedPointSizeRange[1];
		}
		break;
		case GL_ALPHA_BITS:
		{
			GLenum format = getDrawFramebufferFormat(0);
			int alpha_bits = getAlphaBitsFromFormat(format);
			*data = static_cast<GLfloat>(alpha_bits);
		}
		break;
		case GL_ARRAY_BUFFER_BINDING:
		{
			CoreBuffer* core_buffer = m_state.getBuffer(GL_ARRAY_BUFFER);
			if (core_buffer != nullptr) {
				*data = static_cast<GLfloat>(core_buffer->getId());
			} else {
				*data = 0.0f;
			}
		}
		break;
		case GL_BLEND:
			*data = static_cast<GLfloat>(m_state.getEnable(GL_BLEND));
			break;
		case GL_BLEND_COLOR:
		{
			const BlendParams& params = m_state.getBlendParams();
			data[0] = params.blendColor[0];
			data[1] = params.blendColor[1];
			data[2] = params.blendColor[2];
			data[3] = params.blendColor[3];
		}
		break;
		case GL_BLEND_DST_ALPHA:
		{
			const BlendParams& params = m_state.getBlendParams();
			*data = static_cast<GLfloat>(params.blendDst[1]);
		}
		break;
		case GL_BLEND_DST_RGB:
		{
			const BlendParams& params = m_state.getBlendParams();
			*data = static_cast<GLfloat>(params.blendDst[0]);
		}
		break;
		case GL_BLEND_EQUATION_ALPHA:
		{
			const BlendParams& params = m_state.getBlendParams();
			*data = static_cast<GLfloat>(params.blendEquation[1]);
		}
		break;
		case GL_BLEND_EQUATION_RGB:
		{
			const BlendParams& params = m_state.getBlendParams();
			*data = static_cast<GLfloat>(params.blendEquation[0]);
		}
		break;
		case GL_BLEND_SRC_ALPHA:
		{
			const BlendParams& params = m_state.getBlendParams();
			*data = static_cast<GLfloat>(params.blendSrc[1]);
		}
		break;
		case GL_BLEND_SRC_RGB:
		{
			const BlendParams& params = m_state.getBlendParams();
			*data = static_cast<GLfloat>(params.blendSrc[0]);
		}
		break;
		case GL_BLUE_BITS:
		{
			GLenum format = getDrawFramebufferFormat(0);
			int blue_bits = getBlueBitsFromFormat(format);
			*data = static_cast<GLfloat>(blue_bits);
		}
		break;
		case GL_COLOR_CLEAR_VALUE:
		{
			const ClearParameters& params = m_state.getClearParams();
			data[0] = params.colorClearValue[0][0];
			data[1] = params.colorClearValue[0][1];
			data[2] = params.colorClearValue[0][2];
			data[3] = params.colorClearValue[0][3];
		}
		break;
		case GL_COLOR_WRITEMASK:
		{
			const ColorWritemaskParams& params = m_state.getColorWritemaskParams();
			data[0] = static_cast<GLfloat>(params.colorWritemask[0]);
			data[1] = static_cast<GLfloat>(params.colorWritemask[1]);
			data[2] = static_cast<GLfloat>(params.colorWritemask[2]);
			data[3] = static_cast<GLfloat>(params.colorWritemask[3]);
		}
		break;
		case GL_COMPRESSED_TEXTURE_FORMATS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			for (int i = 0; i < params.numCompressedTextureFormats; i++) {
				data[i] = static_cast<GLfloat>(params.compressedTextureFormats[i]);
			}
		}
		break;
		case GL_COPY_READ_BUFFER_BINDING:
		{
			CoreBuffer* core_buffer = m_state.getBuffer(GL_COPY_READ_BUFFER);
			if (core_buffer != nullptr) {
				*data = static_cast<GLfloat>(core_buffer->getId());
			} else {
				*data = 0.0f;
			}
		}
		break;
		case GL_COPY_WRITE_BUFFER_BINDING:
		{
			CoreBuffer* core_buffer = m_state.getBuffer(GL_COPY_WRITE_BUFFER);
			if (core_buffer != nullptr) {
				*data = static_cast<GLfloat>(core_buffer->getId());
			} else {
				*data = 0.0f;
			}
		}
		break;
		case GL_CULL_FACE:
		{
			const CullFaceParams& params = m_state.getCullFaceParams();
			*data = static_cast<GLfloat>(params.cullFaceEnable);
		}
		break;
		case GL_CULL_FACE_MODE:
		{
			const CullFaceParams& params = m_state.getCullFaceParams();
			*data = static_cast<GLfloat>(params.cullFaceMode);
		}
		break;
		case GL_CURRENT_PROGRAM:
		{
			CoreProgram* core_program = m_state.getProgram();
			if (core_program != nullptr) {
				*data = static_cast<GLfloat>(core_program->getId());
			} else {
				*data = 0.0f;
			}
		}
		break;
		case GL_DEPTH_BITS:
		{
			GLenum format = getDrawDepthbufferFormat();
			int depth_bits = getDepthBitsFromFormat(format);
			*data = static_cast<GLfloat>(depth_bits);
		}
		break;
		case GL_DEPTH_CLEAR_VALUE:
		{
			const ClearParameters& params = m_state.getClearParams();
			*data = params.depthClearValue;
		}
		break;
		case GL_DEPTH_FUNC:
		{
			const DepthTestParams& params = m_state.getDepthTestParams();
			*data = static_cast<GLfloat>(params.depthFunc);
		}
		break;
		case GL_DEPTH_RANGE:
		{
			const ViewportParams& params = m_state.getViewportParams();
			data[0] = params.depthRange[0];
			data[1] = params.depthRange[1];
		}
		break;
		case GL_DEPTH_TEST:
		{
			const DepthTestParams& params = m_state.getDepthTestParams();
			*data = static_cast<GLfloat>(params.depthTestEnable);
		}
		break;
		case GL_DEPTH_WRITEMASK:
		{
			const DepthStencilWritemaskParams& params = m_state.getDepthStencilWritemaskParams();
			*data = static_cast<GLfloat>(params.depthWritemask);
		}
		break;
		case GL_DITHER:
			*data = static_cast<GLfloat>(m_state.getEnable(GL_DITHER));
			break;
		case GL_DRAW_FRAMEBUFFER_BINDING:
		{
			CoreFramebuffer* core_framebuffer = m_state.getFramebuffer(GL_DRAW_FRAMEBUFFER);
			if (core_framebuffer != nullptr) {
				*data = static_cast<GLfloat>(core_framebuffer->getId());
			} else {
				*data = 0.0f;
			}
		}
		break;
		case GL_ELEMENT_ARRAY_BUFFER_BINDING:
		{
			CoreBuffer* core_buffer = m_state.getBuffer(GL_ELEMENT_ARRAY_BUFFER);
			if (core_buffer != nullptr) {
				*data = static_cast<GLfloat>(core_buffer->getId());
			} else {
				*data = 0.0f;
			}
		}
		break;
		case GL_FRAGMENT_SHADER_DERIVATIVE_HINT:
			*data = static_cast<GLfloat>(m_state.getHint(GL_FRAGMENT_SHADER_DERIVATIVE_HINT));
			break;
		case GL_FRONT_FACE:
		{
			const CullFaceParams& params = m_state.getCullFaceParams();
			*data = static_cast<GLfloat>(params.frontFace);
		}
		break;
		case GL_GENERATE_MIPMAP_HINT:
			*data = static_cast<GLfloat>(m_state.getHint(GL_GENERATE_MIPMAP_HINT));
			break;
		case GL_GREEN_BITS:
		{
			GLenum format = getDrawFramebufferFormat(0);
			int green_bits = getGreenBitsFromFormat(format);
			*data = static_cast<GLfloat>(green_bits);
		}
		break;
		case GL_IMPLEMENTATION_COLOR_READ_FORMAT:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.implementationColorReadFormat);
		}
		break;
		case GL_IMPLEMENTATION_COLOR_READ_TYPE:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.implementationColorReadType);
		}
		break;
		case GL_LINE_WIDTH:
			*data = m_state.getLineWidth();
			break;
		case GL_MAJOR_VERSION:
			*data = 3.0f;
			break;
		case GL_MAX_3D_TEXTURE_SIZE:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.max3dTextureSize);
		}
		break;
		case GL_MAX_ARRAY_TEXTURE_LAYERS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.maxArrayTextureLayers);
		}
		break;
		case GL_MAX_COLOR_ATTACHMENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.maxColorAttachments);
		}
		break;
		case GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.maxCombinedFragmentUniformComponents);
		}
		break;
		case GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.maxCombinedTextureImageUnits);
		}
		break;
		case GL_MAX_COMBINED_UNIFORM_BLOCKS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.maxCombinedUniformBlocks);
		}
		break;
		case GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.maxCombinedVertexUniformComponents);
		}
		break;
		case GL_MAX_CUBE_MAP_TEXTURE_SIZE:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.maxCubeMapTextureSize);
		}
		break;
		case GL_MAX_DRAW_BUFFERS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.maxDrawBuffers);
		}
		break;
		case GL_MAX_ELEMENT_INDEX:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.maxElementIndex);
		}
		break;
		case GL_MAX_ELEMENTS_INDICES:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.maxElementsIndices);
		}
		break;
		case GL_MAX_ELEMENTS_VERTICES:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.maxElementsVertices);
		}
		break;
		case GL_MAX_FRAGMENT_INPUT_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.maxFragmentInputComponents);
		}
		break;
		case GL_MAX_FRAGMENT_UNIFORM_BLOCKS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.maxFragmentUniformBlocks);
		}
		break;
		case GL_MAX_FRAGMENT_UNIFORM_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.maxFragmentUniformComponents);
		}
		break;
		case GL_MAX_FRAGMENT_UNIFORM_VECTORS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.maxFragmentUniformVectors);
		}
		break;
		case GL_MAX_PROGRAM_TEXEL_OFFSET:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.maxProgramTexelOffset);
		}
		break;
		case GL_MAX_RENDERBUFFER_SIZE:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.maxRenderbufferSize);
		}
		break;
		case GL_MAX_SAMPLES:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.maxSamples);
		}
		break;
		case GL_MAX_SERVER_WAIT_TIMEOUT:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.maxServerWaitTimeout);
		}
		break;
		case GL_MAX_TEXTURE_IMAGE_UNITS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.maxTextureImageUnits);
		}
		break;
		case GL_MAX_TEXTURE_LOD_BIAS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxTextureLodBias;
		}
		break;
		case GL_MAX_TEXTURE_SIZE:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.maxTextureSize);
		}
		break;
		case GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.maxTransformFeedbackInterleavedComponents);
		}
		break;
		case GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.maxTransformFeedbackSeparateAttribs);
		}
		break;
		case GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.maxTransformFeedbackSeparateComponents);
		}
		break;
		case GL_MAX_UNIFORM_BLOCK_SIZE:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.maxUniformBlockSize);
		}
		break;
		case GL_MAX_UNIFORM_BUFFER_BINDINGS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.maxUniformBufferBindings);
		}
		break;
		case GL_MAX_VARYING_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.maxVaryingComponents);
		}
		break;
		case GL_MAX_VARYING_VECTORS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.maxVaryingVectors);
		}
		break;
		case GL_MAX_VERTEX_ATTRIBS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.maxVertexAttribs);
		}
		break;
		case GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.maxVertexTextureImageUnits);
		}
		break;
		case GL_MAX_VERTEX_OUTPUT_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.maxVertexOutputComponents);
		}
		break;
		case GL_MAX_VERTEX_UNIFORM_BLOCKS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.maxVertexUniformBlocks);
		}
		break;
		case GL_MAX_VERTEX_UNIFORM_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.maxVertexUniformComponents);
		}
		break;
		case GL_MAX_VERTEX_UNIFORM_VECTORS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.maxVertexUniformVectors);
		}
		break;
		case GL_MAX_VIEWPORT_DIMS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			data[0] = static_cast<GLfloat>(params.maxViewportDims[0]);
			data[1] = static_cast<GLfloat>(params.maxViewportDims[1]);
		}
		break;
		case GL_MIN_PROGRAM_TEXEL_OFFSET:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.minProgramTexelOffset);
		}
		break;
		case GL_MINOR_VERSION:
			*data = 0.0f;
			break;
		case GL_NUM_COMPRESSED_TEXTURE_FORMATS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.numCompressedTextureFormats);
		}
		break;
		case GL_NUM_EXTENSIONS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.numExtensions);
		}
		break;
		case GL_NUM_PROGRAM_BINARY_FORMATS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.numProgramBinaryFormats);
		}
		break;
		case GL_NUM_SHADER_BINARY_FORMATS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.numShaderBinaryFormats);
		}
		break;
		case GL_PACK_ALIGNMENT:
		{
			const CoreState::PackParams& params = m_state.getPackParams();
			*data = static_cast<GLfloat>(params.packAlignment);
		}
		break;
		case GL_PACK_ROW_LENGTH:
		{
			const CoreState::PackParams& params = m_state.getPackParams();
			*data = static_cast<GLfloat>(params.packRowLength);
		}
		break;
		case GL_PACK_SKIP_PIXELS:
		{
			const CoreState::PackParams& params = m_state.getPackParams();
			*data = static_cast<GLfloat>(params.packSkipPixels);
		}
		break;
		case GL_PACK_SKIP_ROWS:
		{
			const CoreState::PackParams& params = m_state.getPackParams();
			*data = static_cast<GLfloat>(params.packSkipRows);
		}
		break;
		case GL_PIXEL_PACK_BUFFER_BINDING:
		{
			CoreBuffer* core_buffer = m_state.getBuffer(GL_PIXEL_PACK_BUFFER);
			if (core_buffer != nullptr) {
				*data = static_cast<GLfloat>(core_buffer->getId());
			} else {
				*data = 0.0f;
			}
		}
		break;
		case GL_PIXEL_UNPACK_BUFFER_BINDING:
		{
			CoreBuffer* core_buffer = m_state.getBuffer(GL_PIXEL_UNPACK_BUFFER);
			if (core_buffer != nullptr) {
				*data = static_cast<GLfloat>(core_buffer->getId());
			} else {
				*data = 0.0f;
			}
		}
		break;
		case GL_POLYGON_OFFSET_FACTOR:
		{
			const PolygonOffsetParams& params = m_state.getPolygonOffsetParams();
			*data = params.polygonOffsetFactor;
		}
		break;
		case GL_POLYGON_OFFSET_FILL:
		{
			const PolygonOffsetParams& params = m_state.getPolygonOffsetParams();
			*data = static_cast<GLfloat>(params.polygonOffsetFillEnable);
		}
		break;
		case GL_POLYGON_OFFSET_UNITS:
		{
			const PolygonOffsetParams& params = m_state.getPolygonOffsetParams();
			*data = params.polygonOffsetUnits;
		}
		break;
		case GL_PRIMITIVE_RESTART_FIXED_INDEX:
			*data = static_cast<GLfloat>(m_state.getEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX));
			break;
		case GL_PROGRAM_BINARY_FORMATS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			for (int i = 0; i < params.numProgramBinaryFormats; i++) {
				data[i] = static_cast<GLfloat>(params.programBinaryFormats[i]);
			}
		}
		break;
		case GL_RASTERIZER_DISCARD:
			*data = static_cast<GLfloat>(m_state.getEnable(GL_RASTERIZER_DISCARD));
			break;
		case GL_READ_BUFFER:
		{
			GLenum read_buffer = m_state.getReadBuffer();
			*data = static_cast<GLfloat>(read_buffer);
		}
		break;
		case GL_READ_FRAMEBUFFER_BINDING:
		{
			CoreFramebuffer* core_framebuffer = m_state.getFramebuffer(GL_READ_FRAMEBUFFER);
			if (core_framebuffer != nullptr) {
				*data = static_cast<GLfloat>(core_framebuffer->getId());
			} else {
				*data = 0.0f;
			}
		}
		break;
		case GL_RED_BITS:
		{
			GLenum format = getDrawFramebufferFormat(0);
			int red_bits = getRedBitsFromFormat(format);
			*data = static_cast<GLfloat>(red_bits);
		}
		break;
		case GL_RENDERBUFFER_BINDING:
		{
			CoreRenderbuffer* core_renderbuffer = m_state.getRenderbuffer(GL_RENDERBUFFER);
			if (core_renderbuffer != nullptr) {
				*data = static_cast<GLfloat>(core_renderbuffer->getId());
			} else {
				*data = 0.0f;
			}
		}
		break;
		case GL_SAMPLE_ALPHA_TO_COVERAGE:
		{
			const SampleCoverageParams& params = m_state.getSampleCoverageParams();
			*data = static_cast<GLfloat>(params.sampleAlphaToCoverageEnable);
		}
		break;
		case GL_SAMPLE_BUFFERS:
		{
			// TODO: Framebuffer のマルチサンプル数
		}
		break;
		case GL_SAMPLE_COVERAGE:
		{
			const SampleCoverageParams& params = m_state.getSampleCoverageParams();
			*data = static_cast<GLfloat>(params.sampleCoverageEnable);
		}
		break;
		case GL_SAMPLE_COVERAGE_INVERT:
		{
			const SampleCoverageParams& params = m_state.getSampleCoverageParams();
			*data = static_cast<GLfloat>(params.sampleCoverageInvert);
		}
		break;
		case GL_SAMPLE_COVERAGE_VALUE:
		{
			const SampleCoverageParams& params = m_state.getSampleCoverageParams();
			*data = params.sampleCoverageValue;
		}
		break;
		case GL_SAMPLER_BINDING:
		{
			GLenum active_texture = m_state.getActiveTexture();
			GLuint unit_index = active_texture - GL_TEXTURE0;
			CoreSampler* core_sampler = m_state.getSampler(unit_index);
			if (core_sampler != nullptr) {
				*data = static_cast<GLfloat>(core_sampler->getId());
			} else {
				*data = 0.0f;
			}
		}
		break;
		case GL_SAMPLES:
			// TODO: framebuffer のサンプル数
			break;
		case GL_SCISSOR_BOX:
		{
			const ScissorParams& params = m_state.getScissorParams();
			data[0] = static_cast<GLfloat>(params.scissorBox[0]);
			data[1] = static_cast<GLfloat>(params.scissorBox[1]);
			data[2] = static_cast<GLfloat>(params.scissorBox[2]);
			data[3] = static_cast<GLfloat>(params.scissorBox[3]);
		}
		break;
		case GL_SCISSOR_TEST:
		{
			const ScissorParams& params = m_state.getScissorParams();
			*data = static_cast<GLfloat>(params.scissorTestEnable);
		}
		break;
		case GL_SHADER_BINARY_FORMATS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			for (int i = 0; i < params.numShaderBinaryFormats; i++) {
				data[i] = static_cast<GLfloat>(params.shaderBinaryFormats[i]);
			}
		}
		break;
		case GL_SHADER_COMPILER:
			*data = static_cast<GLfloat>(GL_TRUE); // 常にGL_TRUE
			break;
		case GL_STENCIL_BACK_FAIL:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = static_cast<GLfloat>(params.stencilBackFail);
		}
		break;
		case GL_STENCIL_BACK_FUNC:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = static_cast<GLfloat>(params.stencilBackFunc);
		}
		break;
		case GL_STENCIL_BACK_PASS_DEPTH_FAIL:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = static_cast<GLfloat>(params.stencilBackPassDepthFail);
		}
		break;
		case GL_STENCIL_BACK_PASS_DEPTH_PASS:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = static_cast<GLfloat>(params.stencilBackPassDepthPass);
		}
		break;
		case GL_STENCIL_BACK_REF:
		{
			const StencilReference& st_ref = m_state.getStencilReference();
			*data = static_cast<GLfloat>(st_ref.stencilBackRef);
		}
		break;
		case GL_STENCIL_BACK_VALUE_MASK:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = static_cast<GLfloat>(params.stencilBackValueMask);
		}
		break;
		case GL_STENCIL_BACK_WRITEMASK:
		{
			const DepthStencilWritemaskParams& params = m_state.getDepthStencilWritemaskParams();
			*data = static_cast<GLfloat>(params.stencilBackWritemask);
		}
		break;
		case GL_STENCIL_BITS:
		{
			GLenum format = getDrawFramebufferFormat(0);
			int green_bits = getGreenBitsFromFormat(format);
			*data = static_cast<GLfloat>(green_bits);
		}
		break;
		case GL_STENCIL_CLEAR_VALUE:
		{
			const ClearParameters& params = m_state.getClearParams();
			*data = static_cast<GLfloat>(params.stencilClearValue);
		}
		break;
		case GL_STENCIL_FAIL:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = static_cast<GLfloat>(params.stencilFail);
		}
		break;
		case GL_STENCIL_FUNC:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = static_cast<GLfloat>(params.stencilFunc);
		}
		break;
		case GL_STENCIL_PASS_DEPTH_FAIL:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = static_cast<GLfloat>(params.stencilPassDepthFail);
		}
		break;
		case GL_STENCIL_PASS_DEPTH_PASS:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = static_cast<GLfloat>(params.stencilPassDepthPass);
		}
		break;
		case GL_STENCIL_REF:
		{
			const StencilReference& st_ref = m_state.getStencilReference();
			*data = static_cast<GLfloat>(st_ref.stencilRef);
		}
		break;
		case GL_STENCIL_TEST:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = static_cast<GLfloat>(params.stencilTestEnable);
		}
		break;
		case GL_STENCIL_VALUE_MASK:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = static_cast<GLfloat>(params.stencilValueMask);
		}
		break;
		case GL_STENCIL_WRITEMASK:
		{
			const DepthStencilWritemaskParams& params = m_state.getDepthStencilWritemaskParams();
			*data = static_cast<GLfloat>(params.stencilWritemask);
		}
		break;
		case GL_SUBPIXEL_BITS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.subpixelBits);
		}
		break;
		case GL_TEXTURE_BINDING_2D:
		{
			CoreTexture* core_texture = m_state.getTexture(GL_TEXTURE_2D);
			if (core_texture != nullptr) {
				*data = static_cast<GLfloat>(core_texture->getId());
			} else {
				*data = 0.0f;
			}
		}
		break;
		case GL_TEXTURE_BINDING_2D_ARRAY:
		{
			CoreTexture* core_texture = m_state.getTexture(GL_TEXTURE_2D_ARRAY);
			if (core_texture != nullptr) {
				*data = static_cast<GLfloat>(core_texture->getId());
			} else {
				*data = 0.0f;
			}
		}
		break;
		case GL_TEXTURE_BINDING_3D:
		{
			CoreTexture* core_texture = m_state.getTexture(GL_TEXTURE_3D);
			if (core_texture != nullptr) {
				*data = static_cast<GLfloat>(core_texture->getId());
			} else {
				*data = 0.0f;
			}
		}
		break;
		case GL_TEXTURE_BINDING_CUBE_MAP:
		{
			CoreTexture* core_texture = m_state.getTexture(GL_TEXTURE_CUBE_MAP);
			if (core_texture != nullptr) {
				*data = static_cast<GLfloat>(core_texture->getId());
			} else {
				*data = 0.0f;
			}
		}
		break;
		case GL_TRANSFORM_FEEDBACK_BINDING:
		{
			CoreTransformFeedback* core_transform_feedback = m_state.getTransformFeedback(GL_TRANSFORM_FEEDBACK);
			if (core_transform_feedback != nullptr) {
				*data = static_cast<GLfloat>(core_transform_feedback->getId());
			} else {
				*data = 0.0f;
			}
		}
		break;
		case GL_TRANSFORM_FEEDBACK_ACTIVE:
		{
			const CoreState::TransformFeedbackParams& params = m_state.getTransformFeedbackParams();
			*data = static_cast<GLfloat>(params.transformFeedbackActive);
		}
		break;
		case GL_TRANSFORM_FEEDBACK_BUFFER_BINDING:
		{
			const IndexedBuffer* buffer = m_state.getIndexedBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);
			if ((buffer != nullptr) && (buffer->buffer != nullptr)) {
				*data = static_cast<GLfloat>(buffer->buffer->getId());
			} else {
				*data = 0.0f;
			}
		}
		break;
		case GL_TRANSFORM_FEEDBACK_PAUSED:
		{
			const CoreState::TransformFeedbackParams& params = m_state.getTransformFeedbackParams();
			*data = static_cast<GLfloat>(params.transformFeedbackPaused);
		}
		break;
		case GL_TRANSFORM_FEEDBACK_BUFFER_SIZE:
		{
			const IndexedBuffer* buffer = m_state.getIndexedBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);
			if (buffer != nullptr) {
				*data = static_cast<GLfloat>(buffer->size);
			} else {
				*data = 0.0f;
			}
		}
		break;
		case GL_TRANSFORM_FEEDBACK_BUFFER_START:
		{
			const IndexedBuffer* buffer = m_state.getIndexedBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);
			if (buffer != nullptr) {
				*data = static_cast<GLfloat>(buffer->offset);
			} else {
				*data = 0.0f;
			}
		}
		break;
		case GL_UNIFORM_BUFFER_BINDING:
		{
			CoreBuffer* core_buffer = m_state.getBuffer(GL_UNIFORM_BUFFER);
			if (core_buffer != nullptr) {
				*data = static_cast<GLfloat>(core_buffer->getId());
			} else {
				*data = 0.0f;
			}
		}
		break;
		case GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLfloat>(params.uniformBufferOffsetAlignment);
		}
		break;
		case GL_UNIFORM_BUFFER_SIZE:
		{
			const IndexedBuffer* buffer = m_state.getIndexedBuffer(GL_UNIFORM_BUFFER, 0);
			if (buffer != nullptr) {
				*data = static_cast<GLfloat>(buffer->size);
			} else {
				*data = 0.0f;
			}
		}
		break;
		case GL_UNIFORM_BUFFER_START:
		{
			const IndexedBuffer* buffer = m_state.getIndexedBuffer(GL_UNIFORM_BUFFER, 0);
			if (buffer != nullptr) {
				*data = static_cast<GLfloat>(buffer->offset);
			} else {
				*data = 0.0f;
			}
		}
		break;
		case GL_UNPACK_ALIGNMENT:
		{
			const CoreState::UnpackParams& params = m_state.getUnpackParams();
			*data = static_cast<GLfloat>(params.unpackAlignment);
		}
		break;
		case GL_UNPACK_IMAGE_HEIGHT:
		{
			const CoreState::UnpackParams& params = m_state.getUnpackParams();
			*data = static_cast<GLfloat>(params.unpackImageHeight);
		}
		break;
		case GL_UNPACK_ROW_LENGTH:
		{
			const CoreState::UnpackParams& params = m_state.getUnpackParams();
			*data = static_cast<GLfloat>(params.unpackRowLength);
		}
		break;
		case GL_UNPACK_SKIP_IMAGES:
		{
			const CoreState::UnpackParams& params = m_state.getUnpackParams();
			*data = static_cast<GLfloat>(params.unpackSkipImages);
		}
		break;
		case GL_UNPACK_SKIP_PIXELS:
		{
			const CoreState::UnpackParams& params = m_state.getUnpackParams();
			*data = static_cast<GLfloat>(params.unpackSkipPixels);
		}
		break;
		case GL_UNPACK_SKIP_ROWS:
		{
			const CoreState::UnpackParams& params = m_state.getUnpackParams();
			*data = static_cast<GLfloat>(params.unpackSkipRows);
		}
		break;
		case GL_VERTEX_ARRAY_BINDING:
		{
			CoreVertexArray* core_vertex_array = m_state.getVertexArray();
			if (core_vertex_array != nullptr) {
				*data = static_cast<GLfloat>(core_vertex_array->getId());
			} else {
				*data = 0.0f;
			}
		}
		break;
		case GL_VIEWPORT:
		{
			const ViewportParams& params = m_state.getViewportParams();
			data[0] = static_cast<GLfloat>(params.viewport[0]);
			data[1] = static_cast<GLfloat>(params.viewport[1]);
			data[2] = static_cast<GLfloat>(params.viewport[2]);
			data[3] = static_cast<GLfloat>(params.viewport[3]);
		}
		break;
		default:
			// GL_INVALID_ENUM
			setErrorCode(GL_INVALID_ENUM);
			break;
		}
	}
	return;
}

void CoreContext::getFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint* params)
{
	CoreFramebuffer* core_framebuffer = m_state.getFramebuffer(target);
	if (core_framebuffer == nullptr) {
		// TODO: default framebuffer
	} else {
		core_framebuffer->getAttachmentParameteriv(attachment, pname, params);
	}

	return;
}

void CoreContext::getIntegerv(GLenum pname, GLint* data)
{
	AXGL_ASSERT(data != nullptr);
	AXGL_ASSERT(m_pBackendContext != nullptr);
	if ((pname >= GL_DRAW_BUFFER0) && (pname < (GL_DRAW_BUFFER0 + AXGL_MAX_DRAW_BUFFERS))) {
		// GL_DRAW_BUFFERx
		GLenum buf = m_state.getDrawBuffer(pname);
		*data = buf;
	} else {
		// general
		switch (pname) {
		case GL_ACTIVE_TEXTURE:
			*data = m_state.getActiveTexture();
			break;
		case GL_ALIASED_LINE_WIDTH_RANGE:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			data[0] = static_cast<GLint>(params.aliasedLineWidthRange[0]);
			data[1] = static_cast<GLint>(params.aliasedLineWidthRange[1]);
		}
		break;
		case GL_ALIASED_POINT_SIZE_RANGE:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			data[0] = static_cast<GLint>(params.aliasedPointSizeRange[0]);
			data[1] = static_cast<GLint>(params.aliasedPointSizeRange[1]);
		}
		break;
		case GL_ALPHA_BITS:
		{
			GLenum format = getDrawFramebufferFormat(0);
			*data = getAlphaBitsFromFormat(format);
		}
		break;
		case GL_ARRAY_BUFFER_BINDING:
		{
			CoreBuffer* core_buffer = m_state.getBuffer(GL_ARRAY_BUFFER);
			if (core_buffer != nullptr) {
				*data = core_buffer->getId();
			} else {
				*data = 0;
			}
		}
		break;

		case GL_BLEND:
			*data = m_state.getEnable(GL_BLEND);
			break;
		case GL_BLEND_COLOR:
		{
			const BlendParams& params = m_state.getBlendParams();
			data[0] = FLOAT_TO_INT_COLOR(params.blendColor[0]);
			data[1] = FLOAT_TO_INT_COLOR(params.blendColor[1]);
			data[2] = FLOAT_TO_INT_COLOR(params.blendColor[2]);
			data[3] = FLOAT_TO_INT_COLOR(params.blendColor[3]);
		}
		break;
		case GL_BLEND_DST_ALPHA:
		{
			const BlendParams& params = m_state.getBlendParams();
			*data = params.blendDst[1];
		}
		break;
		case GL_BLEND_DST_RGB:
		{
			const BlendParams& params = m_state.getBlendParams();
			*data = params.blendDst[0];
		}
		break;
		case GL_BLEND_EQUATION_ALPHA:
		{
			const BlendParams& params = m_state.getBlendParams();
			*data = params.blendEquation[1];
		}
		break;
		case GL_BLEND_EQUATION_RGB:
		{
			const BlendParams& params = m_state.getBlendParams();
			*data = params.blendEquation[0];
		}
		break;
		case GL_BLEND_SRC_ALPHA:
		{
			const BlendParams& params = m_state.getBlendParams();
			*data = params.blendSrc[1];
		}
		break;
		case GL_BLEND_SRC_RGB:
		{
			const BlendParams& params = m_state.getBlendParams();
			*data = params.blendSrc[0];
		}
		break;
		case GL_BLUE_BITS:
		{
			GLenum format = getDrawFramebufferFormat(0);
			*data = getBlueBitsFromFormat(format);
		}
		break;
		case GL_COLOR_CLEAR_VALUE:
		{
			const ClearParameters& params = m_state.getClearParams();
			data[0] = FLOAT_TO_INT_COLOR(params.colorClearValue[0][0]);
			data[1] = FLOAT_TO_INT_COLOR(params.colorClearValue[0][1]);
			data[2] = FLOAT_TO_INT_COLOR(params.colorClearValue[0][2]);
			data[3] = FLOAT_TO_INT_COLOR(params.colorClearValue[0][3]);
		}
		break;
		case GL_COLOR_WRITEMASK:
		{
			const ColorWritemaskParams& params = m_state.getColorWritemaskParams();
			data[0] = params.colorWritemask[0];
			data[1] = params.colorWritemask[1];
			data[2] = params.colorWritemask[2];
			data[3] = params.colorWritemask[3];
		}
		break;
		case GL_COMPRESSED_TEXTURE_FORMATS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			for (int i = 0; i < params.numCompressedTextureFormats; i++) {
				data[i] = params.compressedTextureFormats[i];
			}
		}
		break;
		case GL_COPY_READ_BUFFER_BINDING:
		{
			CoreBuffer* core_buffer = m_state.getBuffer(GL_COPY_READ_BUFFER);
			if (core_buffer != nullptr) {
				*data = core_buffer->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_COPY_WRITE_BUFFER_BINDING:
		{
			CoreBuffer* core_buffer = m_state.getBuffer(GL_COPY_WRITE_BUFFER);
			if (core_buffer != nullptr) {
				*data = core_buffer->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_CULL_FACE:
		{
			const CullFaceParams& params = m_state.getCullFaceParams();
			*data = params.cullFaceEnable;
		}
		break;
		case GL_CULL_FACE_MODE:
		{
			const CullFaceParams& params = m_state.getCullFaceParams();
			*data = params.cullFaceMode;
		}
		break;
		case GL_CURRENT_PROGRAM:
		{
			CoreProgram* core_program = m_state.getProgram();
			if (core_program != nullptr) {
				*data = core_program->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_DEPTH_BITS:
		{
			GLenum format = getDrawDepthbufferFormat();
			*data = getDepthBitsFromFormat(format);
		}
		break;
		case GL_DEPTH_CLEAR_VALUE:
		{
			// NOTE: depth は正規化しない (おそらく過去のGL仕様の経緯から)
			const ClearParameters& params = m_state.getClearParams();
			*data = static_cast<GLint>(params.depthClearValue);
		}
		break;
		case GL_DEPTH_FUNC:
		{
			const DepthTestParams& params = m_state.getDepthTestParams();
			*data = params.depthFunc;
		}
		break;
		case GL_DEPTH_RANGE:
		{
			const ViewportParams& params = m_state.getViewportParams();
			data[0] = static_cast<GLint>(params.depthRange[0]);
			data[1] = static_cast<GLint>(params.depthRange[1]);
		}
		break;
		case GL_DEPTH_TEST:
		{
			const DepthTestParams& params = m_state.getDepthTestParams();
			*data = params.depthTestEnable;
		}
		break;
		case GL_DEPTH_WRITEMASK:
		{
			const DepthStencilWritemaskParams& params = m_state.getDepthStencilWritemaskParams();
			*data = params.depthWritemask;
		}
		break;
		case GL_DITHER:
			*data = m_state.getEnable(GL_DITHER);
			break;
		case GL_DRAW_FRAMEBUFFER_BINDING:
		{
			CoreFramebuffer* core_framebuffer = m_state.getFramebuffer(GL_DRAW_FRAMEBUFFER);
			if (core_framebuffer != nullptr) {
				*data = core_framebuffer->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_ELEMENT_ARRAY_BUFFER_BINDING:
		{
			CoreBuffer* core_buffer = m_state.getBuffer(GL_ELEMENT_ARRAY_BUFFER);
			if (core_buffer != nullptr) {
				*data = core_buffer->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_FRAGMENT_SHADER_DERIVATIVE_HINT:
			*data = m_state.getHint(GL_FRAGMENT_SHADER_DERIVATIVE_HINT);
			break;
		case GL_FRONT_FACE:
		{
			const CullFaceParams& params = m_state.getCullFaceParams();
			*data = params.frontFace;
		}
		break;
		case GL_GENERATE_MIPMAP_HINT:
			*data = m_state.getHint(GL_GENERATE_MIPMAP_HINT);
			break;
		case GL_GREEN_BITS:
		{
			GLenum format = getDrawFramebufferFormat(0);
			*data = getGreenBitsFromFormat(format);
		}
		break;
		case GL_IMPLEMENTATION_COLOR_READ_FORMAT:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.implementationColorReadFormat;
		}
		break;
		case GL_IMPLEMENTATION_COLOR_READ_TYPE:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.implementationColorReadType;
		}
		break;
		case GL_LINE_WIDTH:
			*data = static_cast<GLint>(m_state.getLineWidth());
			break;
		case GL_MAJOR_VERSION:
			*data = 3;
			break;
		case GL_MAX_3D_TEXTURE_SIZE:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.max3dTextureSize;
		}
		break;
		case GL_MAX_ARRAY_TEXTURE_LAYERS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxArrayTextureLayers;
		}
		break;
		case GL_MAX_COLOR_ATTACHMENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxColorAttachments;
		}
		break;
		case GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxCombinedFragmentUniformComponents;
		}
		break;
		case GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxCombinedTextureImageUnits;
		}
		break;
		case GL_MAX_COMBINED_UNIFORM_BLOCKS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxCombinedUniformBlocks;
		}
		break;
		case GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxCombinedVertexUniformComponents;
		}
		break;
		case GL_MAX_CUBE_MAP_TEXTURE_SIZE:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxCubeMapTextureSize;
		}
		break;
		case GL_MAX_DRAW_BUFFERS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxDrawBuffers;
		}
		break;
		case GL_MAX_ELEMENT_INDEX:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxElementIndex;
		}
		break;
		case GL_MAX_ELEMENTS_INDICES:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxElementsIndices;
		}
		break;
		case GL_MAX_ELEMENTS_VERTICES:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxElementsVertices;
		}
		break;
		case GL_MAX_FRAGMENT_INPUT_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxFragmentInputComponents;
		}
		break;
		case GL_MAX_FRAGMENT_UNIFORM_BLOCKS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxFragmentUniformBlocks;
		}
		break;
		case GL_MAX_FRAGMENT_UNIFORM_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxFragmentUniformComponents;
		}
		break;
		case GL_MAX_FRAGMENT_UNIFORM_VECTORS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxFragmentUniformVectors;
		}
		break;
		case GL_MAX_PROGRAM_TEXEL_OFFSET:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxProgramTexelOffset;
		}
		break;
		case GL_MAX_RENDERBUFFER_SIZE:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxRenderbufferSize;
		}
		break;
		case GL_MAX_SAMPLES:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxSamples;
		}
		break;
		case GL_MAX_SERVER_WAIT_TIMEOUT:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLint>(params.maxServerWaitTimeout);
		}
		break;
		case GL_MAX_TEXTURE_IMAGE_UNITS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxTextureImageUnits;
		}
		break;
		case GL_MAX_TEXTURE_LOD_BIAS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLint>(params.maxTextureLodBias);
		}
		break;
		case GL_MAX_TEXTURE_SIZE:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxTextureSize;
		}
		break;
		case GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxTransformFeedbackInterleavedComponents;
		}
		break;
		case GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxTransformFeedbackSeparateAttribs;
		}
		break;
		case GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxTransformFeedbackSeparateComponents;
		}
		break;
		case GL_MAX_UNIFORM_BLOCK_SIZE:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxUniformBlockSize;
		}
		break;
		case GL_MAX_UNIFORM_BUFFER_BINDINGS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxUniformBufferBindings;
		}
		break;
		case GL_MAX_VARYING_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxVaryingComponents;
		}
		break;
		case GL_MAX_VARYING_VECTORS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxVaryingVectors;
		}
		break;
		case GL_MAX_VERTEX_ATTRIBS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxVertexAttribs;
		}
		break;
		case GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxVertexTextureImageUnits;
		}
		break;
		case GL_MAX_VERTEX_OUTPUT_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxVertexOutputComponents;
		}
		break;
		case GL_MAX_VERTEX_UNIFORM_BLOCKS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxVertexUniformBlocks;
		}
		break;
		case GL_MAX_VERTEX_UNIFORM_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxVertexUniformComponents;
		}
		break;
		case GL_MAX_VERTEX_UNIFORM_VECTORS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxVertexUniformVectors;
		}
		break;
		case GL_MAX_VIEWPORT_DIMS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			data[0] = params.maxViewportDims[0];
			data[1] = params.maxViewportDims[1];
		}
		break;
		case GL_MIN_PROGRAM_TEXEL_OFFSET:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.minProgramTexelOffset;
		}
		break;
		case GL_MINOR_VERSION:
			*data = 0;
			break;
		case GL_NUM_COMPRESSED_TEXTURE_FORMATS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.numCompressedTextureFormats;
		}
		break;
		case GL_NUM_EXTENSIONS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.numExtensions;
		}
		break;
		case GL_NUM_PROGRAM_BINARY_FORMATS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.numProgramBinaryFormats;
		}
		break;
		case GL_NUM_SHADER_BINARY_FORMATS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.numShaderBinaryFormats;
		}
		break;
		case GL_PACK_ALIGNMENT:
		{
			const CoreState::PackParams& params = m_state.getPackParams();
			*data = params.packAlignment;
		}
		break;
		case GL_PACK_ROW_LENGTH:
		{
			const CoreState::PackParams& params = m_state.getPackParams();
			*data = params.packRowLength;
		}
		break;
		case GL_PACK_SKIP_PIXELS:
		{
			const CoreState::PackParams& params = m_state.getPackParams();
			*data = params.packSkipPixels;
		}
		break;
		case GL_PACK_SKIP_ROWS:
		{
			const CoreState::PackParams& params = m_state.getPackParams();
			*data = params.packSkipRows;
		}
		break;
		case GL_PIXEL_PACK_BUFFER_BINDING:
		{
			CoreBuffer* core_buffer = m_state.getBuffer(GL_PIXEL_PACK_BUFFER);
			if (core_buffer != nullptr) {
				*data = core_buffer->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_PIXEL_UNPACK_BUFFER_BINDING:
		{
			CoreBuffer* core_buffer = m_state.getBuffer(GL_PIXEL_UNPACK_BUFFER);
			if (core_buffer != nullptr) {
				*data = core_buffer->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_POLYGON_OFFSET_FACTOR:
		{
			const PolygonOffsetParams& params = m_state.getPolygonOffsetParams();
			*data = static_cast<GLint>(params.polygonOffsetFactor);
		}
		break;
		case GL_POLYGON_OFFSET_FILL:
		{
			const PolygonOffsetParams& params = m_state.getPolygonOffsetParams();
			*data = params.polygonOffsetFillEnable;
		}
		break;
		case GL_POLYGON_OFFSET_UNITS:
		{
			const PolygonOffsetParams& params = m_state.getPolygonOffsetParams();
			*data = static_cast<GLint>(params.polygonOffsetUnits);
		}
		break;
		case GL_PRIMITIVE_RESTART_FIXED_INDEX:
			*data = m_state.getEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
			break;
		case GL_PROGRAM_BINARY_FORMATS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			for (int i = 0; i < params.numProgramBinaryFormats; i++) {
				data[i] = params.programBinaryFormats[i];
			}
		}
		break;
		case GL_RASTERIZER_DISCARD:
			*data = m_state.getEnable(GL_RASTERIZER_DISCARD);
			break;
		case GL_READ_BUFFER:
		{
			GLenum read_buffer = m_state.getReadBuffer();
			*data = read_buffer;
		}
		break;
		case GL_READ_FRAMEBUFFER_BINDING:
		{
			CoreFramebuffer* core_framebuffer = m_state.getFramebuffer(GL_READ_FRAMEBUFFER);
			if (core_framebuffer != nullptr) {
				*data = core_framebuffer->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_RED_BITS:
		{
			GLenum format = getDrawFramebufferFormat(0);
			*data = getRedBitsFromFormat(format);
		}
		break;
		case GL_RENDERBUFFER_BINDING:
		{
			CoreRenderbuffer* core_renderbuffer = m_state.getRenderbuffer(GL_RENDERBUFFER);
			if (core_renderbuffer != nullptr) {
				*data = core_renderbuffer->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_SAMPLE_ALPHA_TO_COVERAGE:
		{
			const SampleCoverageParams& params = m_state.getSampleCoverageParams();
			*data = params.sampleAlphaToCoverageEnable;
		}
		break;
		case GL_SAMPLE_BUFFERS:
		{
			// TODO: Framebuffer のマルチサンプル数
		}
		break;
		case GL_SAMPLE_COVERAGE:
		{
			const SampleCoverageParams& params = m_state.getSampleCoverageParams();
			*data = params.sampleCoverageEnable;
		}
		break;
		case GL_SAMPLE_COVERAGE_INVERT:
		{
			const SampleCoverageParams& params = m_state.getSampleCoverageParams();
			*data = params.sampleCoverageInvert;
		}
		break;
		case GL_SAMPLE_COVERAGE_VALUE:
		{
			const SampleCoverageParams& params = m_state.getSampleCoverageParams();
			*data = static_cast<GLint>(params.sampleCoverageValue);
		}
		break;
		case GL_SAMPLER_BINDING:
		{
			GLenum active_texture = m_state.getActiveTexture();
			GLuint unit_index = active_texture - GL_TEXTURE0;
			CoreSampler* core_sampler = m_state.getSampler(unit_index);
			if (core_sampler != nullptr) {
				*data = core_sampler->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_SAMPLES:
			// TODO: framebuffer のサンプル数
			break;
		case GL_SCISSOR_BOX:
		{
			const ScissorParams& params = m_state.getScissorParams();
			data[0] = params.scissorBox[0];
			data[1] = params.scissorBox[1];
			data[2] = params.scissorBox[2];
			data[3] = params.scissorBox[3];
		}
		break;
		case GL_SCISSOR_TEST:
		{
			const ScissorParams& params = m_state.getScissorParams();
			*data = params.scissorTestEnable;
		}
		break;
		case GL_SHADER_BINARY_FORMATS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			for (int i = 0; i < params.numShaderBinaryFormats; i++) {
				data[i] = params.shaderBinaryFormats[i];
			}
		}
		break;
		case GL_SHADER_COMPILER:
			*data = GL_TRUE; // 常にGL_TRUE
			break;
		case GL_STENCIL_BACK_FAIL:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = params.stencilBackFail;
		}
		break;
		case GL_STENCIL_BACK_FUNC:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = params.stencilBackFunc;
		}
		break;
		case GL_STENCIL_BACK_PASS_DEPTH_FAIL:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = params.stencilBackPassDepthFail;
		}
		break;
		case GL_STENCIL_BACK_PASS_DEPTH_PASS:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = params.stencilBackPassDepthPass;
		}
		break;
		case GL_STENCIL_BACK_REF:
		{
			const StencilReference& st_ref = m_state.getStencilReference();
			*data = st_ref.stencilBackRef;
		}
		break;
		case GL_STENCIL_BACK_VALUE_MASK:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = params.stencilBackValueMask;
		}
		break;
		case GL_STENCIL_BACK_WRITEMASK:
		{
			const DepthStencilWritemaskParams& params = m_state.getDepthStencilWritemaskParams();
			*data = params.stencilBackWritemask;
		}
		break;
		case GL_STENCIL_BITS:
		{
			GLenum format = getDrawFramebufferFormat(0);
			*data = getGreenBitsFromFormat(format);
		}
		break;
		case GL_STENCIL_CLEAR_VALUE:
		{
			const ClearParameters& params = m_state.getClearParams();
			*data = params.stencilClearValue;
		}
		break;
		case GL_STENCIL_FAIL:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = params.stencilFail;
		}
		break;
		case GL_STENCIL_FUNC:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = params.stencilFunc;
		}
		break;
		case GL_STENCIL_PASS_DEPTH_FAIL:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = params.stencilPassDepthFail;
		}
		break;
		case GL_STENCIL_PASS_DEPTH_PASS:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = params.stencilPassDepthPass;
		}
		break;
		case GL_STENCIL_REF:
		{
			const StencilReference& st_ref = m_state.getStencilReference();
			*data = st_ref.stencilRef;
		}
		break;
		case GL_STENCIL_TEST:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = params.stencilTestEnable;
		}
		break;
		case GL_STENCIL_VALUE_MASK:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = params.stencilValueMask;
		}
		break;
		case GL_STENCIL_WRITEMASK:
		{
			const DepthStencilWritemaskParams& params = m_state.getDepthStencilWritemaskParams();
			*data = params.stencilWritemask;
		}
		break;
		case GL_SUBPIXEL_BITS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.subpixelBits;
		}
		break;
		case GL_TEXTURE_BINDING_2D:
		{
			CoreTexture* core_texture = m_state.getTexture(GL_TEXTURE_2D);
			if (core_texture != nullptr) {
				*data = core_texture->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_TEXTURE_BINDING_2D_ARRAY:
		{
			CoreTexture* core_texture = m_state.getTexture(GL_TEXTURE_2D_ARRAY);
			if (core_texture != nullptr) {
				*data = core_texture->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_TEXTURE_BINDING_3D:
		{
			CoreTexture* core_texture = m_state.getTexture(GL_TEXTURE_3D);
			if (core_texture != nullptr) {
				*data = core_texture->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_TEXTURE_BINDING_CUBE_MAP:
		{
			CoreTexture* core_texture = m_state.getTexture(GL_TEXTURE_CUBE_MAP);
			if (core_texture != nullptr) {
				*data = core_texture->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_TRANSFORM_FEEDBACK_BINDING:
		{
			CoreTransformFeedback* core_transform_feedback = m_state.getTransformFeedback(GL_TRANSFORM_FEEDBACK);
			if (core_transform_feedback != nullptr) {
				*data = core_transform_feedback->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_TRANSFORM_FEEDBACK_ACTIVE:
		{
			const CoreState::TransformFeedbackParams& params = m_state.getTransformFeedbackParams();
			*data = params.transformFeedbackActive;
		}
		break;
		case GL_TRANSFORM_FEEDBACK_BUFFER_BINDING:
		{
			const IndexedBuffer* buffer = m_state.getIndexedBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);
			if ((buffer != nullptr) && (buffer->buffer != nullptr)) {
				*data = buffer->buffer->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_TRANSFORM_FEEDBACK_PAUSED:
		{
			const CoreState::TransformFeedbackParams& params = m_state.getTransformFeedbackParams();
			*data = params.transformFeedbackPaused;
		}
		break;
		case GL_TRANSFORM_FEEDBACK_BUFFER_SIZE:
		{
			const IndexedBuffer* buffer = m_state.getIndexedBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);
			if (buffer != nullptr) {
				*data = static_cast<GLint>(buffer->size);
			} else {
				*data = 0;
			}
		}
		break;
		case GL_TRANSFORM_FEEDBACK_BUFFER_START:
		{
			const IndexedBuffer* buffer = m_state.getIndexedBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);
			if (buffer != nullptr) {
				*data = static_cast<GLint>(buffer->offset);
			} else {
				*data = 0;
			}
		}
		break;
		case GL_UNIFORM_BUFFER_BINDING:
		{
			CoreBuffer* core_buffer = m_state.getBuffer(GL_UNIFORM_BUFFER);
			if (core_buffer != nullptr) {
				*data = core_buffer->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.uniformBufferOffsetAlignment;
		}
		break;
		case GL_UNIFORM_BUFFER_SIZE:
		{
			const IndexedBuffer* buffer = m_state.getIndexedBuffer(GL_UNIFORM_BUFFER, 0);
			if (buffer != nullptr) {
				*data = static_cast<GLint>(buffer->size);
			} else {
				*data = 0;
			}
		}
		break;
		case GL_UNIFORM_BUFFER_START:
		{
			const IndexedBuffer* buffer = m_state.getIndexedBuffer(GL_UNIFORM_BUFFER, 0);
			if (buffer != nullptr) {
				*data = static_cast<GLint>(buffer->offset);
			} else {
				*data = 0;
			}
		}
		break;
		case GL_UNPACK_ALIGNMENT:
		{
			const CoreState::UnpackParams& params = m_state.getUnpackParams();
			*data = params.unpackAlignment;
		}
		break;
		case GL_UNPACK_IMAGE_HEIGHT:
		{
			const CoreState::UnpackParams& params = m_state.getUnpackParams();
			*data = params.unpackImageHeight;
		}
		break;
		case GL_UNPACK_ROW_LENGTH:
		{
			const CoreState::UnpackParams& params = m_state.getUnpackParams();
			*data = params.unpackRowLength;
		}
		break;
		case GL_UNPACK_SKIP_IMAGES:
		{
			const CoreState::UnpackParams& params = m_state.getUnpackParams();
			*data = params.unpackSkipImages;
		}
		break;
		case GL_UNPACK_SKIP_PIXELS:
		{
			const CoreState::UnpackParams& params = m_state.getUnpackParams();
			*data = params.unpackSkipPixels;
		}
		break;
		case GL_UNPACK_SKIP_ROWS:
		{
			const CoreState::UnpackParams& params = m_state.getUnpackParams();
			*data = params.unpackSkipRows;
		}
		break;
		case GL_VERTEX_ARRAY_BINDING:
		{
			CoreVertexArray* core_vertex_array = m_state.getVertexArray();
			if (core_vertex_array != nullptr) {
				*data = core_vertex_array->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_VIEWPORT:
		{
			const ViewportParams& params = m_state.getViewportParams();
			data[0] = params.viewport[0];
			data[1] = params.viewport[1];
			data[2] = params.viewport[2];
			data[3] = params.viewport[3];
		}
		break;
		default:
			// GL_INVALID_ENUM
			setErrorCode(GL_INVALID_ENUM);
			AXGL_ASSERT(0);
			break;
		}
	}
	return;
}

void CoreContext::getProgramiv(GLuint program, GLenum pname, GLint* params)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreProgram* core_program = m_pObjectsManager->getProgram(program);
	if (core_program == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_program->getProgramiv(pname, params);
	return;
}

void CoreContext::getProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreProgram* core_program = m_pObjectsManager->getProgram(program);
	if (core_program == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_program->getProgramInfoLog(bufSize, length, infoLog);
	return;
}

void CoreContext::getRenderbufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
	CoreRenderbuffer* core_renderbuffer = m_state.getRenderbuffer(target);
	if (core_renderbuffer == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_renderbuffer->getParameteriv(pname, params);
	return;
}

void CoreContext::getShaderiv(GLuint shader, GLenum pname, GLint* params)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreShader* core_shader = m_pObjectsManager->getShader(shader);
	if (core_shader == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_shader->getShaderiv(pname, params);
	return;
}

void CoreContext::getShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreShader* core_shader = m_pObjectsManager->getShader(shader);
	if (core_shader == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_shader->getShaderInfoLog(bufSize, length, infoLog);
	return;
}

void CoreContext::getShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision)
{
	// NOTE: GLES 3.0仕様で保証される値を返しておく
	if ((shadertype == GL_VERTEX_SHADER) || (shadertype == GL_FRAGMENT_SHADER)) {
		switch (precisiontype) {
		case GL_LOW_FLOAT:
			range[0] = 1;
			range[1] = 1;
			*precision = 8;
			break;
		case GL_MEDIUM_FLOAT:
			range[0] = 14;
			range[1] = 14;
			*precision = 10;
			break;
		case GL_HIGH_FLOAT:
			range[0] = 126;
			range[1] = 126;
			*precision = 24;
			break;
		case GL_LOW_INT:
			range[0] = 8;
			range[1] = 8;
			*precision = 0;
			break;
		case GL_MEDIUM_INT:
			range[0] = 11;
			range[1] = 11;
			*precision = 0;
			break;
		case GL_HIGH_INT:
			range[0] = 24;
			range[1] = 24;
			*precision = 0;
			break;
		default:
			break;
		}
	}
	return;
}

void CoreContext::getShaderSource(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* source)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreShader* core_shader = m_pObjectsManager->getShader(shader);
	if (core_shader == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_shader->getShaderSource(bufSize, length, source);
	return;
}

const GLubyte* CoreContext::getString(GLenum name)
{
	const char* rptr = nullptr;
	switch (name) {
	case GL_EXTENSIONS:
		rptr = c_extension_string;
		break;
	case GL_VENDOR:
		rptr = c_vendor_string;
		break;
	case GL_RENDERER:
		rptr = c_renderer_string;
		break;
	case GL_VERSION:
		rptr = c_version_string;
		break;
	case GL_SHADING_LANGUAGE_VERSION:
		rptr = c_shader_language_version;
		break;
	default:
		// GL_INVALID_ENUM is generated if name is not an accepted value.
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return reinterpret_cast<const GLubyte*>(rptr);
}

void CoreContext::getTexParameterfv(GLenum target, GLenum pname, GLfloat* params)
{
	CoreTexture* core_texture = m_state.getTexture(target);
	if (core_texture == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_texture->getParameterfv(pname, params);
	return;
}

void CoreContext::getTexParameteriv(GLenum target, GLenum pname, GLint* params)
{
	CoreTexture* core_texture = m_state.getTexture(target);
	if (core_texture == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_texture->getParameteriv(pname, params);
	return;
}

void CoreContext::getUniformfv(GLuint program, GLint location, GLfloat* params)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreProgram* core_program = m_pObjectsManager->getProgram(program);
	if (core_program == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_program->getUniformfv(location, params);
	return;
}

void CoreContext::getUniformiv(GLuint program, GLint location, GLint* params)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreProgram* core_program = m_pObjectsManager->getProgram(program);
	if (core_program == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_program->getUniformiv(location, params);
	return;
}

GLint CoreContext::getUniformLocation(GLuint program, const GLchar* name)
{
	if (m_pObjectsManager == nullptr) {
		return -1;
	}
	CoreProgram* core_program = m_pObjectsManager->getProgram(program);
	if (core_program == nullptr) {
		return -1;
	}
	return core_program->getUniformLocation(name);
}

void CoreContext::getVertexAttribfv(GLuint index, GLenum pname, GLfloat* params)
{
	CoreVertexArray* core_vertex_array = m_state.getVertexArray();
	if ((core_vertex_array == nullptr) || (pname == GL_CURRENT_VERTEX_ATTRIB)) {
		m_state.getVertexAttribfv(index, pname, params);
	} else {
		core_vertex_array->getVertexAttribfv(index, pname, params);
	}
	return;
}

void CoreContext::getVertexAttribiv(GLuint index, GLenum pname, GLint* params)
{
	CoreVertexArray* core_vertex_array = m_state.getVertexArray();
	if ((core_vertex_array == nullptr) || (pname == GL_CURRENT_VERTEX_ATTRIB)) {
		m_state.getVertexAttribiv(index, pname, params);
	} else {
		core_vertex_array->getVertexAttribIiv(index, pname, params);
	}
	return;
}

void CoreContext::getVertexAttribPointerv(GLuint index, GLenum pname, void** pointer)
{
	CoreVertexArray* core_vertex_array = m_state.getVertexArray();
	if (core_vertex_array == nullptr) {
		m_state.getVertexAttribPointerv(index, pname, pointer);
	} else {
		core_vertex_array->getVertexAttribPointerv(index, pname, pointer);
	}
	return;
}

void CoreContext::hint(GLenum target, GLenum mode)
{
	m_state.setHint(target, mode);
	return;
}

GLboolean CoreContext::isBuffer(GLuint buffer)
{
	if (m_pObjectsManager == nullptr) {
		return GL_FALSE;
	}
	CoreBuffer* core_buffer = m_pObjectsManager->getBuffer(buffer);
	return (core_buffer != nullptr) ? GL_TRUE : GL_FALSE;
}

GLboolean CoreContext::isEnabled(GLenum cap)
{
	return m_state.getEnable(cap);
}

GLboolean CoreContext::isFramebuffer(GLuint framebuffer)
{
	if (m_pObjectsManager == nullptr) {
		return GL_FALSE;
	}
	CoreFramebuffer* core_framebuffer = m_pObjectsManager->getFramebuffer(framebuffer);
	return (core_framebuffer != nullptr) ? GL_TRUE : GL_FALSE;
}

GLboolean CoreContext::isProgram(GLuint program)
{
	if (m_pObjectsManager == nullptr) {
		return GL_FALSE;
	}
	CoreProgram* core_program = m_pObjectsManager->getProgram(program);
	return (core_program != nullptr) ? GL_TRUE : GL_FALSE;
}

GLboolean CoreContext::isRenderbuffer(GLuint renderbuffer)
{
	if (m_pObjectsManager == nullptr) {
		return GL_FALSE;
	}
	CoreRenderbuffer* core_renderbuffer = m_pObjectsManager->getRenderbuffer(renderbuffer);
	return (core_renderbuffer != nullptr) ? GL_TRUE : GL_FALSE;
}

GLboolean CoreContext::isShader(GLuint shader)
{
	if (m_pObjectsManager == nullptr) {
		return GL_FALSE;
	}
	CoreShader* core_shader = m_pObjectsManager->getShader(shader);
	return (core_shader != nullptr) ? GL_TRUE : GL_FALSE;
}

GLboolean CoreContext::isTexture(GLuint texture)
{
	if (m_pObjectsManager == nullptr) {
		return GL_FALSE;
	}
	CoreTexture* core_texture = m_pObjectsManager->getTexture(texture);
	return (core_texture != nullptr) ? GL_TRUE : GL_FALSE;
}

void CoreContext::lineWidth(GLfloat width)
{
	m_state.setLineWidth(width);
	return;
}

void CoreContext::linkProgram(GLuint program)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreProgram* core_program = m_pObjectsManager->getProgram(program);
	if (core_program == nullptr) {
		return;
	}
	core_program->link(this);
	return;
}

void CoreContext::pixelStorei(GLenum pname, GLint param)
{
	m_state.setPixelStore(pname, param);
	return;
}

void CoreContext::polygonOffset(GLfloat factor, GLfloat units)
{
	m_state.setPolygonOffset(factor, units);
	return;
}

void CoreContext::readPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels)
{
	if ((m_pBackendContext == nullptr) || (pixels == nullptr)) {
		return;
	}
	// GL_READ_BUFFER
	GLenum read_buffer = m_state.getReadBuffer();
	if (read_buffer == GL_NONE) {
		return;
	}
	BackendFramebuffer* backend_framebuffer = nullptr;
	CoreFramebuffer* read_framebuffer = m_state.getFramebuffer(GL_READ_FRAMEBUFFER);
	if (read_framebuffer != 0) {
		backend_framebuffer = read_framebuffer->getBackendFramebuffer();
	}
	m_pBackendContext->readPixels(x, y, width, height, format, type,
		backend_framebuffer, read_buffer, pixels);
	return;
}

void CoreContext::releaseShaderCompiler(void)
{
	return;
}

void CoreContext::renderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
	CoreRenderbuffer* core_renderbufffer = m_state.getRenderbuffer(target);
	if (core_renderbufffer == nullptr) {
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_renderbufffer->createStorage(this, internalformat, width, height);
	return;
}

void CoreContext::sampleCoverage(GLfloat value, GLboolean invert)
{
	m_state.setSampleCoverage(value, invert);
	return;
}

void CoreContext::scissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
	m_state.setScissorBox(x, y, width, height);
	return;
}

void CoreContext::shaderBinary(GLsizei count, const GLuint* shaders, GLenum binaryformat, const void* binary, GLsizei length)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	for (int i = 0; i < count; i++) {
		CoreShader* core_shader = m_pObjectsManager->getShader(shaders[i]);
		if (core_shader != nullptr) {
			core_shader->setBinary(binaryformat, binary, length, i);
		}
	}
	return;
}

void CoreContext::shaderSource(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreShader* core_shader = m_pObjectsManager->getShader(shader);
	if (core_shader == nullptr) {
		return;
	}
	core_shader->setSource(count, string, length);
	return;
}

void CoreContext::stencilFunc(GLenum func, GLint ref, GLuint mask)
{
	m_state.setStencilFunc(func, ref, mask);
	return;
}

void CoreContext::stencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask)
{
	m_state.setStencilFuncSeparate(face, func, ref, mask);
	return;
}

void CoreContext::stencilMask(GLuint mask)
{
	m_state.setStencilWritemask(mask);
	return;
}

void CoreContext::stencilMaskSeparate(GLenum face, GLuint mask)
{
	m_state.setStencilWritemaskSeparate(face, mask);
	return;
}

void CoreContext::stencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
	m_state.setStencilOp(fail, zfail, zpass);
	return;
}

void CoreContext::stencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass)
{
	m_state.setStencilOpSeparate(face, sfail, dpfail, dppass);
	return;
}

void CoreContext::texImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels)
{
	CoreTexture* core_texture = m_state.getTexture(target);
	if (core_texture == nullptr) {
		return;
	}
	core_texture->texImage2d(this, target, level, internalformat, width, height, border, format, type, pixels);
	return;
}

void CoreContext::texParameterf(GLenum target, GLenum pname, GLfloat param)
{
	CoreTexture* core_texture = m_state.getTexture(target);
	if (core_texture == nullptr) {
		return;
	}
	core_texture->texParameterf(pname, param);
	return;
}

void CoreContext::texParameterfv(GLenum target, GLenum pname, const GLfloat* params)
{
	CoreTexture* core_texture = m_state.getTexture(target);
	if (core_texture == nullptr) {
		return;
	}
	core_texture->texParameterfv(pname, params);
	return;
}

void CoreContext::texParameteri(GLenum target, GLenum pname, GLint param)
{
	CoreTexture* core_texture = m_state.getTexture(target);
	if (core_texture == nullptr) {
		return;
	}
	core_texture->texParameteri(pname, param);
	return;
}

void CoreContext::texParameteriv(GLenum target, GLenum pname, const GLint* params)
{
	CoreTexture* core_texture = m_state.getTexture(target);
	if (core_texture == nullptr) {
		return;
	}
	core_texture->texParameteriv(pname, params);
	return;
}

void CoreContext::texSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels)
{
	CoreTexture* core_texture = m_state.getTexture(target);
	if (core_texture == nullptr) {
		return;
	}
	core_texture->texSubImage2d(this, target, level, xoffset, yoffset, width, height, format, type, pixels);
	return;
}

void CoreContext::uniform1f(GLint location, GLfloat v0)
{
	CoreProgram* core_program = m_state.getProgram();
	if (core_program == nullptr) {
		return;
	}
	core_program->setUniform1f(location, v0);
	return;
}

void CoreContext::uniform1fv(GLint location, GLsizei count, const GLfloat* value)
{
	CoreProgram* core_program = m_state.getProgram();
	if (core_program == nullptr) {
		return;
	}
	core_program->setUniform1fv(location, count, value);
	return;
}

void CoreContext::uniform1i(GLint location, GLint v0)
{
	CoreProgram* core_program = m_state.getProgram();
	if (core_program == nullptr) {
		return;
	}
	core_program->setUniform1i(location, v0);
	return;
}

void CoreContext::uniform1iv(GLint location, GLsizei count, const GLint* value)
{
	CoreProgram* core_program = m_state.getProgram();
	if (core_program == nullptr) {
		return;
	}
	core_program->setUniform1iv(location, count, value);
	return;
}

void CoreContext::uniform2f(GLint location, GLfloat v0, GLfloat v1)
{
	CoreProgram* core_program = m_state.getProgram();
	if (core_program == nullptr) {
		return;
	}
	core_program->setUniform2f(location, v0, v1);
	return;
}

void CoreContext::uniform2fv(GLint location, GLsizei count, const GLfloat* value)
{
	CoreProgram* core_program = m_state.getProgram();
	if (core_program == nullptr) {
		return;
	}
	core_program->setUniform2fv(location, count, value);
	return;
}

void CoreContext::uniform2i(GLint location, GLint v0, GLint v1)
{
	CoreProgram* core_program = m_state.getProgram();
	if (core_program == nullptr) {
		return;
	}
	core_program->setUniform2i(location, v0, v1);
	return;
}

void CoreContext::uniform2iv(GLint location, GLsizei count, const GLint* value)
{
	CoreProgram* core_program = m_state.getProgram();
	if (core_program == nullptr) {
		return;
	}
	core_program->setUniform2iv(location, count, value);
	return;
}

void CoreContext::uniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
	CoreProgram* core_program = m_state.getProgram();
	if (core_program == nullptr) {
		return;
	}
	core_program->setUniform3f(location, v0, v1, v2);
	return;
}

void CoreContext::uniform3fv(GLint location, GLsizei count, const GLfloat* value)
{
	CoreProgram* core_program = m_state.getProgram();
	if (core_program == nullptr) {
		return;
	}
	core_program->setUniform3fv(location, count, value);
	return;
}

void CoreContext::uniform3i(GLint location, GLint v0, GLint v1, GLint v2)
{
	CoreProgram* core_program = m_state.getProgram();
	if (core_program == nullptr) {
		return;
	}
	core_program->setUniform3i(location, v0, v1, v2);
	return;
}

void CoreContext::uniform3iv(GLint location, GLsizei count, const GLint* value)
{
	CoreProgram* core_program = m_state.getProgram();
	if (core_program == nullptr) {
		return;
	}
	core_program->setUniform3iv(location, count, value);
	return;
}

void CoreContext::uniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
	CoreProgram* core_program = m_state.getProgram();
	if (core_program == nullptr) {
		return;
	}
	core_program->setUniform4f(location, v0, v1, v2, v3);
	return;
}

void CoreContext::uniform4fv(GLint location, GLsizei count, const GLfloat* value)
{
	CoreProgram* core_program = m_state.getProgram();
	if (core_program == nullptr) {
		return;
	}
	core_program->setUniform4fv(location, count, value);
	return;
}

void CoreContext::uniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
	CoreProgram* core_program = m_state.getProgram();
	if (core_program == nullptr) {
		return;
	}
	core_program->setUniform4i(location, v0, v1, v2, v3);
	return;
}

void CoreContext::uniform4iv(GLint location, GLsizei count, const GLint* value)
{
	CoreProgram* core_program = m_state.getProgram();
	if (core_program == nullptr) {
		return;
	}
	core_program->setUniform4iv(location, count, value);
	return;
}

void CoreContext::uniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	CoreProgram* core_program = m_state.getProgram();
	if (core_program == nullptr) {
		return;
	}
	core_program->setUniformMatrix2fv(location, count, transpose, value);
	return;
}

void CoreContext::uniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	CoreProgram* core_program = m_state.getProgram();
	if (core_program == nullptr) {
		return;
	}
	core_program->setUniformMatrix3fv(location, count, transpose, value);
	return;
}

void CoreContext::uniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	CoreProgram* core_program = m_state.getProgram();
	if (core_program == nullptr) {
		return;
	}
	core_program->setUniformMatrix4fv(location, count, transpose, value);
	return;
}

void CoreContext::useProgram(GLuint program)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreProgram* core_program = m_pObjectsManager->getProgram(program);
	m_state.setProgram(this, core_program);
	return;
}

void CoreContext::validateProgram(GLuint program)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreProgram* core_program = m_pObjectsManager->getProgram(program);
	if (core_program == nullptr) {
		return;
	}
	core_program->validateProgram();
	return;
}

void CoreContext::vertexAttrib1f(GLuint index, GLfloat x)
{
	m_state.setVertexAttrib1f(index, x);
	return;
}

void CoreContext::vertexAttrib1fv(GLuint index, const GLfloat* v)
{
	m_state.setVertexAttrib1fv(index, v);
	return;
}

void CoreContext::vertexAttrib2f(GLuint index, GLfloat x, GLfloat y)
{
	m_state.setVertexAttrib2f(index, x, y);
	return;
}

void CoreContext::vertexAttrib2fv(GLuint index, const GLfloat* v)
{
	m_state.setVertexAttrib2fv(index, v);
	return;
}

void CoreContext::vertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
	m_state.setVertexAttrib3f(index, x, y, z);
	return;
}

void CoreContext::vertexAttrib3fv(GLuint index, const GLfloat* v)
{
	m_state.setVertexAttrib3fv(index, v);
	return;
}

void CoreContext::vertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	m_state.setVertexAttrib4f(index, x, y, z, w);
	return;
}

void CoreContext::vertexAttrib4fv(GLuint index, const GLfloat* v)
{
	m_state.setVertexAttrib4fv(index, v);
	return;
}

void CoreContext::vertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer)
{
	CoreBuffer* core_buffer = m_state.getBuffer(GL_ARRAY_BUFFER);
	CoreVertexArray* core_vertex_array = m_state.getVertexArray();
	if (core_vertex_array != nullptr) {
		core_vertex_array->setVertexAttribPointer(this, index, size, type, normalized, stride, pointer, core_buffer);
	} else {
		m_state.setVertexAttribPointer(this, index, size, type, normalized, stride, pointer, core_buffer);
	}
	return;
}

void CoreContext::viewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	m_state.setViewport(x, y, width, height);
	return;
}

// GLES 3.0  API --------------------------------------------------------------
void CoreContext::readBuffer(GLenum src)
{
	m_state.setReadBuffer(src);
	return;
}

void CoreContext::drawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices)
{
	// 引数start,endは無視: ドライバ最適化(頂点バッファの使用範囲)のヒント
	AXGL_UNUSED(start);
	AXGL_UNUSED(end);
	if (m_pBackendContext == nullptr) {
		return;
	}
	if (!m_state.elementArrayBufferAvailable()) {
		// Element Array Buffer がバインドされていない（クライアントメモリを使用する）描画はサポートしない
		return;
	}
	// サンプラとテクスチャを準備
	m_state.setupTextureSampler(this);
	// インスタンス描画を無効に設定
	m_state.setInstancedRendering(false);
	// 描画
	m_pBackendContext->drawElements(mode, count, type, indices, &m_state.getDrawParameters(), &m_state.getClearParams());
	// GLステートのダーティをクリア
	m_state.clearDrawParameterDirtyFlags();
	return;
}

void CoreContext::texImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels)
{
	CoreTexture* core_texture = m_state.getTexture(target);
	if (core_texture == nullptr) {
		return;
	}
	core_texture->texImage3d(this, target, level, internalformat, width, height, depth, border, format, type, pixels);
	return;
}

void CoreContext::texSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels)
{
	CoreTexture* core_texture = m_state.getTexture(target);
	if (core_texture == nullptr) {
		return;
	}
	core_texture->texSubImage3d(this, target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
	return;
}

void CoreContext::copyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	CoreTexture* core_texture = m_state.getTexture(target);
	if (core_texture == nullptr) {
		return;
	}
	core_texture->copyTexSubImage3d(level, xoffset, yoffset, zoffset, x, y, width, height);
	return;
}

void CoreContext::compressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* data)
{
	CoreTexture* core_texture = m_state.getTexture(target);
	if (core_texture == nullptr) {
		return;
	}
	core_texture->compressedTexImage3d(this, target, level, internalformat, width, height, depth, border, imageSize, data);
	return;
}

void CoreContext::compressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data)
{
	CoreTexture* core_texture = m_state.getTexture(target);
	if (core_texture == nullptr) {
		return;
	}
	core_texture->compressedTexSubImage3d(this, target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
	return;
}

void CoreContext::genQueries(GLsizei n, GLuint* ids)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	m_pObjectsManager->genQueries(this, n, ids);
	return;
}

void CoreContext::deleteQueries(GLsizei n, const GLuint* ids)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	m_pObjectsManager->deleteQueries(this, n, ids);
	return;
}

GLboolean CoreContext::isQuery(GLuint id)
{
	if (m_pObjectsManager == nullptr) {
		return GL_FALSE;
	}
	CoreQuery* core_query = m_pObjectsManager->getQuery(id);
	return (core_query != nullptr) ? GL_TRUE : GL_FALSE;
}

void CoreContext::beginQuery(GLenum target, GLuint id)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreQuery* core_query = m_pObjectsManager->getQuery(id);
	if (core_query == nullptr) {
		return;
	}
	// TODO: target check
	core_query->begin(this, target);
	m_state.setCurrentQuery(this, target, core_query);
	return;
}

void CoreContext::endQuery(GLenum target)
{
	// TODO: targat check
	CoreQuery* core_query = m_state.getCurrentQuery(target);
	if (core_query == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_query->end(this, target);
	m_state.setCurrentQuery(this, target, nullptr);
	return;
}

void CoreContext::getQueryiv(GLenum target, GLenum pname, GLint* params)
{
	if (pname != GL_CURRENT_QUERY) {
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	CoreQuery* core_query = m_state.getCurrentQuery(target);
	if (core_query == nullptr) {
		// no query is active
		*params = 0;
		return;
	}
	*params = core_query->getId();
	return;
}

void CoreContext::getQueryObjectuiv(GLuint id, GLenum pname, GLuint* params)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	switch (pname) {
	case GL_QUERY_RESULT:
	case GL_QUERY_RESULT_AVAILABLE:
		{
			CoreQuery* core_query = m_pObjectsManager->getQuery(id);
			if (core_query == nullptr) {
				return;
			}
			core_query->getQueryObjectuiv(pname, params);
			break;
		}
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return;
}

GLboolean CoreContext::unmapBuffer(GLenum target)
{
	// TODO: target check
	CoreBuffer* core_buffer = m_state.getBuffer(target);
	if (core_buffer == nullptr) {
		// GL_INVALID_OPERATION
		// NOTE: targetにバッファがバインドされていないケースがリファレンス記載なし
		setErrorCode(GL_INVALID_OPERATION);
		return GL_FALSE;
	}
	return core_buffer->unmapBuffer(this);
}

void CoreContext::getBufferPointerv(GLenum target, GLenum pname, void** params)
{
	// TODO: target check
	CoreBuffer* core_buffer = m_state.getBuffer(target);
	if (core_buffer == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_buffer->getBufferPointerv(this, pname, params);
	return;
}

void CoreContext::drawBuffers(GLsizei n, const GLenum* bufs)
{
	m_state.setDrawBuffers(n, bufs);
	return;
}

void CoreContext::uniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	CoreProgram* core_program = m_state.getProgram();
	if (core_program == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_program->setUniformMatrix2x3fv(location, count, transpose, value);
	return;
}

void CoreContext::uniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	CoreProgram* core_program = m_state.getProgram();
	if (core_program == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_program->setUniformMatrix3x2fv(location, count, transpose, value);
	return;
}

void CoreContext::uniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	CoreProgram* core_program = m_state.getProgram();
	if (core_program == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_program->setUniformMatrix2x4fv(location, count, transpose, value);
	return;
}

void CoreContext::uniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	CoreProgram* core_program = m_state.getProgram();
	if (core_program == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_program->setUniformMatrix4x2fv(location, count, transpose, value);
	return;
}

void CoreContext::uniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	CoreProgram* core_program = m_state.getProgram();
	if (core_program == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_program->setUniformMatrix3x4fv(location, count, transpose, value);
	return;
}

void CoreContext::uniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	CoreProgram* core_program = m_state.getProgram();
	if (core_program == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_program->setUniformMatrix4x3fv(location, count, transpose, value);
	return;
}

void CoreContext::blitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
	// TODO: Framebufferを描画ターゲットとテクスチャに設定して、矩形描画（単純なAPIだが、非常に面倒）
	AXGL_UNUSED(srcX0);
	AXGL_UNUSED(srcY0);
	AXGL_UNUSED(srcX1);
	AXGL_UNUSED(srcY1);
	AXGL_UNUSED(dstX0);
	AXGL_UNUSED(dstY0);
	AXGL_UNUSED(dstX1);
	AXGL_UNUSED(dstY1);
	AXGL_UNUSED(mask);
	AXGL_UNUSED(filter);
	return;
}

void CoreContext::renderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
	CoreRenderbuffer* core_renderbuffer = m_state.getRenderbuffer(target);
	if (core_renderbuffer == nullptr) {
		// GL_INVALID_OPERATION?
		return;
	}
	core_renderbuffer->createStorageMultisample(this, samples, internalformat, width, height);
	return;
}

void CoreContext::framebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreFramebuffer* core_framebuffer = m_state.getFramebuffer(target);
	if (core_framebuffer == nullptr) {
		// GL_INVALID_OPERATION is generated if zero is bound to target.
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	CoreTexture* core_texture = m_pObjectsManager->getTexture(texture);
	if (core_texture == nullptr) {
		// GL_INVALID_OPERATION is generated if textarget and texture are not compatible.
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_framebuffer->setTextureLayer(this, attachment, core_texture, level, layer);
	return;
}

void* CoreContext::mapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
	CoreBuffer* core_buffer = m_state.getBuffer(target);
	if (core_buffer == nullptr) {
		setErrorCode(GL_INVALID_OPERATION);
		return nullptr;
	}
	return core_buffer->mapBufferRange(this, offset, length, access);
}

void CoreContext::flushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length)
{
	CoreBuffer* core_buffer = m_state.getBuffer(target);
	if (core_buffer == nullptr) {
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_buffer->flushMappedBufferRange(this, offset, length);
	return;
}

void CoreContext::bindVertexArray(GLuint array)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreVertexArray* core_vertex_array = m_pObjectsManager->getVertexArray(array);
	m_state.setVertexArray(this, core_vertex_array);
	return;
}

void CoreContext::deleteVertexArrays(GLsizei n, const GLuint* arrays)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	m_pObjectsManager->deleteVertexArrays(this, n, arrays);
	return;
}

void CoreContext::genVertexArrays(GLsizei n, GLuint* arrays)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	m_pObjectsManager->genVertexArrays(this, n, arrays);
	return;
}

GLboolean CoreContext::isVertexArray(GLuint array)
{
	if (m_pObjectsManager == nullptr) {
		return GL_FALSE;
	}
	CoreVertexArray* core_vertex_array = m_pObjectsManager->getVertexArray(array);
	return (core_vertex_array != nullptr) ? GL_TRUE : GL_FALSE;
}

void CoreContext::getIntegeri_v(GLenum target, GLuint index, GLint* data)
{
	// TODO: index が 範囲外 の GL_INVALID_VALUE
	switch (target) {
	case GL_TRANSFORM_FEEDBACK_BUFFER_BINDING:
	{
		const IndexedBuffer* tfb = m_state.getIndexedBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, index);
		if (tfb != nullptr) {
			if (tfb->buffer != nullptr) {
				*data = tfb->buffer->getId();
			} else {
				*data = 0;
			}
		}
	}
	break;
	case GL_TRANSFORM_FEEDBACK_BUFFER_SIZE:
	{
		const IndexedBuffer* tfb = m_state.getIndexedBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, index);
		if (tfb != nullptr) {
			*data = static_cast<GLint>(tfb->size);
		}
	}
	break;
	case GL_TRANSFORM_FEEDBACK_BUFFER_START:
	{
		const IndexedBuffer* tfb = m_state.getIndexedBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, index);
		if (tfb != nullptr) {
			*data = static_cast<GLint>(tfb->offset);
		}
	}
	break;
	case GL_UNIFORM_BUFFER_BINDING:
	{
		const IndexedBuffer* tfb = m_state.getIndexedBuffer(GL_UNIFORM_BUFFER, index);
		if (tfb != nullptr) {
			if (tfb->buffer != nullptr) {
				*data = tfb->buffer->getId();
			} else {
				*data = 0;
			}
		}
	}
	break;
	case GL_UNIFORM_BUFFER_SIZE:
	{
		const IndexedBuffer* tfb = m_state.getIndexedBuffer(GL_UNIFORM_BUFFER, index);
		if (tfb != nullptr) {
			*data = static_cast<GLint>(tfb->size);
		}
	}
	break;
	case GL_UNIFORM_BUFFER_START:
	{
		const IndexedBuffer* tfb = m_state.getIndexedBuffer(GL_UNIFORM_BUFFER, index);
		if (tfb != nullptr) {
			*data = static_cast<GLint>(tfb->offset);
		}
	}
	break;
	default:
		// その他はインデックスなしと同様
		getIntegerv(target, data);
		break;
	}
	return;
}

void CoreContext::beginTransformFeedback(GLenum primitiveMode)
{
	m_state.setTransformFeedbackActive(GL_TRUE);
	m_state.setTransformFeedbackPrimitive(primitiveMode);
	return;
}

void CoreContext::endTransformFeedback(void)
{
	m_state.setTransformFeedbackActive(GL_FALSE);
	return;
}

void CoreContext::bindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreBuffer* core_buffer = nullptr;
	if (buffer != 0) {
		core_buffer = m_pObjectsManager->getBuffer(buffer);
		if (core_buffer == nullptr) {
			// GL_INVALID_OPERATION? リファレンスに記載なし
			return;
		}
	}
	// TODO: target check, UNIFORM_BUFFER or TRANSFORM_FEEDBACK_BUFFER
	m_state.setBufferRange(this, target, index, core_buffer, offset, size);
	return;
}

void CoreContext::bindBufferBase(GLenum target, GLuint index, GLuint buffer)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreBuffer* core_buffer = nullptr;
	if (buffer != 0) {
		core_buffer = m_pObjectsManager->getBuffer(buffer);
		if (core_buffer == nullptr) {
			// GL_INVALID_OPERATION?
			return;
		}
	}
	// TODO: target check, UNIFORM_BUFFER or TRANSFORM_FEEDBACK_BUFFER
	m_state.setBufferBase(this, target, index, core_buffer);
	return;
}

void CoreContext::transformFeedbackVaryings(GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreProgram* core_program = m_pObjectsManager->getProgram(program);
	if (core_program == nullptr) {
		// GL_INVALID_VALUE is generated if program is not the name of a program object
		setErrorCode(GL_INVALID_VALUE);
		return;
	}
	core_program->setTransformFeedbackVaryings(count, varyings, bufferMode);
	return;
}

void CoreContext::getTransformFeedbackVarying(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreProgram* core_program = m_pObjectsManager->getProgram(program);
	if (core_program == nullptr) {
		// GL_INVALID_VALUE is generated if program is not the name of a program object
		setErrorCode(GL_INVALID_VALUE);
		return;
	}
	core_program->getTransformFeedbackVarying(index, bufSize, length, size, type, name);
	return;
}

void CoreContext::vertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer)
{
	CoreBuffer* core_buffer = m_state.getBuffer(GL_ARRAY_BUFFER);
	CoreVertexArray* core_vertex_array = m_state.getVertexArray();
	if (core_vertex_array != nullptr) {
		// set to vertex array object
		core_vertex_array->setVertexAttribIPointer(this, index, size, type, stride, pointer, core_buffer);
	} else {
		// set to generic state
		m_state.setVertexAttribIPointer(this, index, size, type, stride, pointer, core_buffer);
	}
	return;
}

void CoreContext::getVertexAttribIiv(GLuint index, GLenum pname, GLint* params)
{
	getVertexAttribiv(index, pname, params);
	return;
}

void CoreContext::getVertexAttribIuiv(GLuint index, GLenum pname, GLuint* params)
{
	CoreVertexArray* core_vertex_array = m_state.getVertexArray();
	if ((core_vertex_array == nullptr) || (pname == GL_CURRENT_VERTEX_ATTRIB)) {
		m_state.getVertexAttribIuiv(index, pname, params);
	} else {
		core_vertex_array->getVertexAttribIuiv(index, pname, params);
	}
	return;
}

void CoreContext::vertexAttribI4i(GLuint index, GLint x, GLint y, GLint z, GLint w)
{
	m_state.setVertexAttribI4i(index, x, y, z, w);
	return;
}

void CoreContext::vertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
	m_state.setVertexAttribI4ui(index, x, y, z, w);
	return;
}

void CoreContext::vertexAttribI4iv(GLuint index, const GLint* v)
{
	m_state.setVertexAttribI4i(index, v[0], v[1], v[2], v[3]);
	return;
}

void CoreContext::vertexAttribI4uiv(GLuint index, const GLuint* v)
{
	m_state.setVertexAttribI4ui(index, v[0], v[1], v[2], v[3]);
	return;
}

void CoreContext::getUniformuiv(GLuint program, GLint location, GLuint* params)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreProgram* core_program = m_pObjectsManager->getProgram(program);
	if (core_program == nullptr) {
		// GL_INVALID_OPERATION is generated if program is not a program object
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_program->getUniformuiv(location, params);
	return;
}

GLint CoreContext::getFragDataLocation(GLuint program, const GLchar* name)
{
	if (m_pObjectsManager == nullptr) {
		return -1;
	}
	CoreProgram* core_program = m_pObjectsManager->getProgram(program);
	if (core_program == nullptr) {
		// GL_INVALID_OPERATION is generated if program is not a program object
		setErrorCode(GL_INVALID_OPERATION);
		return -1;
	}
	return core_program->getFragDataLocation(name);
}

void CoreContext::uniform1ui(GLint location, GLuint v0)
{
	CoreProgram* core_program = m_state.getProgram();
	if (core_program == nullptr) {
		// GL_INVALID_OPERATION is generated if there is no current program object
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_program->setUniform1ui(location, v0);
	return;
}

void CoreContext::uniform2ui(GLint location, GLuint v0, GLuint v1)
{
	CoreProgram* core_program = m_state.getProgram();
	if (core_program == nullptr) {
		// GL_INVALID_OPERATION is generated if there is no current program object
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_program->setUniform2ui(location, v0, v1);
	return;
}

void CoreContext::uniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2)
{
	CoreProgram* core_program = m_state.getProgram();
	if (core_program == nullptr) {
		// GL_INVALID_OPERATION is generated if there is no current program object
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_program->setUniform3ui(location, v0, v1, v2);
	return;
}

void CoreContext::uniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
	CoreProgram* core_program = m_state.getProgram();
	if (core_program == nullptr) {
		// GL_INVALID_OPERATION is generated if there is no current program object
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_program->setUniform4ui(location, v0, v1, v2, v3);
	return;
}

void CoreContext::uniform1uiv(GLint location, GLsizei count, const GLuint* value)
{
	CoreProgram* core_program = m_state.getProgram();
	if (core_program == nullptr) {
		// GL_INVALID_OPERATION is generated if there is no current program object
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_program->setUniform1uiv(location, count, value);
	return;
}

void CoreContext::uniform2uiv(GLint location, GLsizei count, const GLuint* value)
{
	CoreProgram* core_program = m_state.getProgram();
	if (core_program == nullptr) {
		// GL_INVALID_OPERATION is generated if there is no current program object
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_program->setUniform2uiv(location, count, value);
	return;
}

void CoreContext::uniform3uiv(GLint location, GLsizei count, const GLuint* value)
{
	CoreProgram* core_program = m_state.getProgram();
	if (core_program == nullptr) {
		// GL_INVALID_OPERATION is generated if there is no current program object
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_program->setUniform3uiv(location, count, value);
	return;
}

void CoreContext::uniform4uiv(GLint location, GLsizei count, const GLuint* value)
{
	CoreProgram* core_program = m_state.getProgram();
	if (core_program == nullptr) {
		// GL_INVALID_OPERATION is generated if there is no current program object
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_program->setUniform4uiv(location, count, value);
	return;
}

void CoreContext::clearBufferiv(GLenum buffer, GLint drawbuffer, const GLint* value)
{
	if (m_pBackendContext == nullptr) {
		return;
	}
	if ((buffer != GL_COLOR) && (buffer != GL_STENCIL)) {
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	m_pBackendContext->clearBufferiv(buffer, drawbuffer, value, &m_state.getDrawParameters());
	return;
}

void CoreContext::clearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint* value)
{
	if (m_pBackendContext == nullptr) {
		return;
	}
	if (buffer != GL_COLOR) {
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	m_pBackendContext->clearBufferuiv(buffer, drawbuffer, value, &m_state.getDrawParameters());
	return;
}

void CoreContext::clearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat* value)
{
	if (m_pBackendContext == nullptr) {
		return;
	}
	if ((buffer != GL_COLOR) && (buffer != GL_DEPTH)) {
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	m_pBackendContext->clearBufferfv(buffer, drawbuffer, value, &m_state.getDrawParameters());
	return;
}

void CoreContext::clearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
	if (m_pBackendContext == nullptr) {
		return;
	}
	if (buffer != GL_DEPTH_STENCIL) {
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	m_pBackendContext->clearBufferfi(buffer, drawbuffer, depth, stencil, &m_state.getDrawParameters());
	return;
}

const GLubyte* CoreContext::getStringi(GLenum name, GLuint index)
{
	static const GLubyte c_dummy[] = { "DummyString" };
	const GLubyte* rstr = nullptr;
	switch (name) {
	case GL_EXTENSIONS:
		// TODO: index check
		// TODO: extensions を分解した index の文字列を返す
		AXGL_UNUSED(index);
		rstr = c_dummy;
		break;
	default:
		// GL_INVALID_ENUM
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return rstr;
}

void CoreContext::copyBufferSubData(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
	CoreBuffer* read_buffer = m_state.getBuffer(readTarget);
	CoreBuffer* write_buffer = m_state.getBuffer(writeTarget);
	if ((read_buffer == nullptr) || (write_buffer == nullptr)) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	write_buffer->copyBufferSubData(this, read_buffer, readOffset, writeOffset, size);
	return;
}

void CoreContext::getUniformIndices(GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreProgram* core_program = m_pObjectsManager->getProgram(program);
	if (core_program == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_program->getUniformIndices(uniformCount, uniformNames, uniformIndices);
	return;
}

void CoreContext::getActiveUniformsiv(GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreProgram* core_program = m_pObjectsManager->getProgram(program);
	if (core_program == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_program->getActiveUniformsiv(uniformCount, uniformIndices, pname, params);
	return;
}

GLuint CoreContext::getUniformBlockIndex(GLuint program, const GLchar* uniformBlockName)
{
	if (m_pObjectsManager == nullptr) {
		return GL_INVALID_INDEX;
	}
	CoreProgram* core_program = m_pObjectsManager->getProgram(program);
	if (core_program == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return GL_INVALID_INDEX;
	}
	return core_program->getUniformBlockIndex(uniformBlockName);
}

void CoreContext::getActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreProgram* core_program = m_pObjectsManager->getProgram(program);
	if (core_program == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_program->getActiveUniformBlockiv(uniformBlockIndex, pname, params);
	return;
}

void CoreContext::getActiveUniformBlockName(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreProgram* core_program = m_pObjectsManager->getProgram(program);
	if (core_program == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_program->getActiveUniformBlockName(uniformBlockIndex, bufSize, length, uniformBlockName);
	return;
}

void CoreContext::uniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreProgram* core_program = m_pObjectsManager->getProgram(program);
	if (core_program == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_program->uniformBlockBinding(uniformBlockIndex, uniformBlockBinding);
	return;
}

void CoreContext::drawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount)
{
	if (m_pBackendContext == nullptr) {
		return;
	}
	// テクスチャとサンプラを準備
	m_state.setupTextureSampler(this);
	// インスタンス描画を有効
	m_state.setInstancedRendering(true);
	// 描画
	m_pBackendContext->drawArraysInstanced(mode, first, count, instancecount, &m_state.getDrawParameters(), &m_state.getClearParams());
	// GLステートのダーティをクリア
	m_state.clearDrawParameterDirtyFlags();
	return;
}

void CoreContext::drawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount)
{
	if (m_pBackendContext == nullptr) {
		return;
	}
	if (!m_state.elementArrayBufferAvailable()) {
		// Element Array Buffer がバインドされていない（クライアントメモリを使用する）描画はサポートしない
		return;
	}
	// テクスチャとサンプラを準備
	m_state.setupTextureSampler(this);
	// インスタンス描画を有効
	m_state.setInstancedRendering(true);
	// 描画
	m_pBackendContext->drawElementsInstanced(mode, count, type, indices, instancecount, &m_state.getDrawParameters(), &m_state.getClearParams());
	// GLステートのダーティをクリア
	m_state.clearDrawParameterDirtyFlags();
	return;
}

GLsync CoreContext::fenceSync(GLenum condition, GLbitfield flags)
{
	if (m_pObjectsManager == nullptr) {
		return static_cast<GLsync>(0);
	}
	CoreSync* core_sync = m_pObjectsManager->createSync(condition, flags);
	if (core_sync == nullptr) {
		// TODO: error GL仕様で既定されてない
		return static_cast<GLsync>(0);
	}
	core_sync->initialize(this);
	BackendSync* backend_sync = core_sync->getBackendSync();
	if (m_pBackendContext != nullptr) {
		m_pBackendContext->fenceSync(backend_sync);
	}
	return reinterpret_cast<GLsync>(core_sync);
}

GLboolean CoreContext::isSync(GLsync sync)
{
	if ((m_pObjectsManager == nullptr) || (sync == static_cast<GLsync>(0))) {
		// 仕様上、GLsync の 0 は sync object になり得ない
		return GL_FALSE;
	}
	CoreSync* core_sync = m_pObjectsManager->getSync(sync);
	return (core_sync != nullptr) ? GL_TRUE : GL_FALSE;
}

void CoreContext::deleteSync(GLsync sync)
{
	if ((m_pObjectsManager == nullptr) || (sync == static_cast<GLsync>(0))) {
		// GLsync が 0 は無視する。GLの仕様
		return;
	}
	m_pObjectsManager->deleteSync(this, sync);
	return;
}

GLenum CoreContext::clientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
	if (m_pObjectsManager == nullptr) {
		return GL_WAIT_FAILED;
	}
	CoreSync* core_sync = m_pObjectsManager->getSync(sync);
	if (core_sync == nullptr) {
		// GL_INVALID_VALUE
		setErrorCode(GL_INVALID_VALUE);
		return GL_WAIT_FAILED;
	}
	GLenum rval = GL_WAIT_FAILED;
	if (m_pBackendContext != nullptr) {
		BackendSync* backend_sync = core_sync->getBackendSync();
		rval = m_pBackendContext->clientWaitSync(backend_sync, flags, timeout);
	}

	return GL_ALREADY_SIGNALED;
}

void CoreContext::waitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	// TODO: flags の 0 チェック
	// TODO: timeout の GL_TIMEOUT_IGNORED チェック
	CoreSync* core_sync = m_pObjectsManager->getSync(sync);
	if (core_sync == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	if (m_pBackendContext != nullptr) {
		BackendSync* backend_sync = core_sync->getBackendSync();
		m_pBackendContext->waitSync(backend_sync, flags, timeout);
	}
	return;
}

void CoreContext::getInteger64v(GLenum pname, GLint64* data)
{
	AXGL_ASSERT(data != nullptr);
	AXGL_ASSERT(m_pBackendContext != nullptr);
	if ((pname >= GL_DRAW_BUFFER0) && (pname < (GL_DRAW_BUFFER0 + AXGL_MAX_DRAW_BUFFERS))) {
		// GL_DRAW_BUFFERx
		GLenum buf = m_state.getDrawBuffer(pname);
		*data = buf;
	} else {
		// general
		switch (pname) {
		case GL_ACTIVE_TEXTURE:
			*data = m_state.getActiveTexture();
			break;
		case GL_ALIASED_LINE_WIDTH_RANGE:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			data[0] = static_cast<GLint64>(params.aliasedLineWidthRange[0]);
			data[1] = static_cast<GLint64>(params.aliasedLineWidthRange[1]);
		}
		break;
		case GL_ALIASED_POINT_SIZE_RANGE:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			data[0] = static_cast<GLint64>(params.aliasedPointSizeRange[0]);
			data[1] = static_cast<GLint64>(params.aliasedPointSizeRange[1]);
		}
		break;
		case GL_ALPHA_BITS:
		{
			GLenum format = getDrawFramebufferFormat(0);
			*data = getAlphaBitsFromFormat(format);
		}
		break;
		case GL_ARRAY_BUFFER_BINDING:
		{
			CoreBuffer* core_buffer = m_state.getBuffer(GL_ARRAY_BUFFER);
			if (core_buffer != nullptr) {
				*data = core_buffer->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_BLEND:
			*data = m_state.getEnable(GL_BLEND);
			break;
		case GL_BLEND_COLOR:
		{
			const BlendParams& params = m_state.getBlendParams();
			data[0] = FLOAT_TO_INT64_COLOR(params.blendColor[0]);
			data[1] = FLOAT_TO_INT64_COLOR(params.blendColor[1]);
			data[2] = FLOAT_TO_INT64_COLOR(params.blendColor[2]);
			data[3] = FLOAT_TO_INT64_COLOR(params.blendColor[3]);
		}
		break;
		case GL_BLEND_DST_ALPHA:
		{
			const BlendParams& params = m_state.getBlendParams();
			*data = params.blendDst[1];
		}
		break;
		case GL_BLEND_DST_RGB:
		{
			const BlendParams& params = m_state.getBlendParams();
			*data = params.blendDst[0];
		}
		break;
		case GL_BLEND_EQUATION_ALPHA:
		{
			const BlendParams& params = m_state.getBlendParams();
			*data = params.blendEquation[1];
		}
		break;
		case GL_BLEND_EQUATION_RGB:
		{
			const BlendParams& params = m_state.getBlendParams();
			*data = params.blendEquation[0];
		}
		break;
		case GL_BLEND_SRC_ALPHA:
		{
			const BlendParams& params = m_state.getBlendParams();
			*data = params.blendSrc[1];
		}
		break;
		case GL_BLEND_SRC_RGB:
		{
			const BlendParams& params = m_state.getBlendParams();
			*data = params.blendSrc[0];
		}
		break;
		case GL_BLUE_BITS:
		{
			GLenum format = getDrawFramebufferFormat(0);
			*data = getBlueBitsFromFormat(format);
		}
		break;
		case GL_COLOR_CLEAR_VALUE:
		{
			const ClearParameters& params = m_state.getClearParams();
			data[0] = FLOAT_TO_INT64_COLOR(params.colorClearValue[0][0]);
			data[1] = FLOAT_TO_INT64_COLOR(params.colorClearValue[0][1]);
			data[2] = FLOAT_TO_INT64_COLOR(params.colorClearValue[0][2]);
			data[3] = FLOAT_TO_INT64_COLOR(params.colorClearValue[0][3]);
		}
		break;
		case GL_COLOR_WRITEMASK:
		{
			const ColorWritemaskParams& params = m_state.getColorWritemaskParams();
			data[0] = params.colorWritemask[0];
			data[1] = params.colorWritemask[1];
			data[2] = params.colorWritemask[2];
			data[3] = params.colorWritemask[3];
		}
		break;
		case GL_COMPRESSED_TEXTURE_FORMATS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			for (int i = 0; i < params.numCompressedTextureFormats; i++) {
				data[i] = params.compressedTextureFormats[i];
			}
		}
		break;
		case GL_COPY_READ_BUFFER_BINDING:
		{
			CoreBuffer* core_buffer = m_state.getBuffer(GL_COPY_READ_BUFFER);
			if (core_buffer != nullptr) {
				*data = core_buffer->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_COPY_WRITE_BUFFER_BINDING:
		{
			CoreBuffer* core_buffer = m_state.getBuffer(GL_COPY_WRITE_BUFFER);
			if (core_buffer != nullptr) {
				*data = core_buffer->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_CULL_FACE:
		{
			const CullFaceParams& params = m_state.getCullFaceParams();
			*data = params.cullFaceEnable;
		}
		break;
		case GL_CULL_FACE_MODE:
		{
			const CullFaceParams& params = m_state.getCullFaceParams();
			*data = params.cullFaceMode;
		}
		break;
		case GL_CURRENT_PROGRAM:
		{
			CoreProgram* core_program = m_state.getProgram();
			if (core_program != nullptr) {
				*data = core_program->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_DEPTH_BITS:
		{
			GLenum format = getDrawDepthbufferFormat();
			*data = getDepthBitsFromFormat(format);
		}
		break;
		case GL_DEPTH_CLEAR_VALUE:
		{
			// NOTE: depth は正規化しない (おそらく過去のGL仕様の経緯から)
			const ClearParameters& params = m_state.getClearParams();
			*data = static_cast<GLint64>(params.depthClearValue);
		}
		break;
		case GL_DEPTH_FUNC:
		{
			const DepthTestParams& params = m_state.getDepthTestParams();
			*data = params.depthFunc;
		}
		break;
		case GL_DEPTH_RANGE:
		{
			const ViewportParams& params = m_state.getViewportParams();
			data[0] = static_cast<GLint64>(params.depthRange[0]);
			data[1] = static_cast<GLint64>(params.depthRange[1]);
		}
		break;
		case GL_DEPTH_TEST:
		{
			const DepthTestParams& params = m_state.getDepthTestParams();
			*data = params.depthTestEnable;
		}
		break;
		case GL_DEPTH_WRITEMASK:
		{
			const DepthStencilWritemaskParams& params = m_state.getDepthStencilWritemaskParams();
			*data = params.depthWritemask;
		}
		break;
		case GL_DITHER:
			*data = m_state.getEnable(GL_DITHER);
			break;
		case GL_DRAW_FRAMEBUFFER_BINDING:
		{
			CoreFramebuffer* core_framebuffer = m_state.getFramebuffer(GL_DRAW_FRAMEBUFFER);
			if (core_framebuffer != nullptr) {
				*data = core_framebuffer->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_ELEMENT_ARRAY_BUFFER_BINDING:
		{
			CoreBuffer* core_buffer = m_state.getBuffer(GL_ELEMENT_ARRAY_BUFFER);
			if (core_buffer != nullptr) {
				*data = core_buffer->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_FRAGMENT_SHADER_DERIVATIVE_HINT:
			*data = m_state.getHint(GL_FRAGMENT_SHADER_DERIVATIVE_HINT);
			break;
		case GL_FRONT_FACE:
		{
			const CullFaceParams& params = m_state.getCullFaceParams();
			*data = params.frontFace;
		}
		break;
		case GL_GENERATE_MIPMAP_HINT:
			*data = m_state.getHint(GL_GENERATE_MIPMAP_HINT);
			break;
		case GL_GREEN_BITS:
		{
			GLenum format = getDrawFramebufferFormat(0);
			*data = getGreenBitsFromFormat(format);
		}
		break;
		case GL_IMPLEMENTATION_COLOR_READ_FORMAT:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.implementationColorReadFormat;
		}
		break;
		case GL_IMPLEMENTATION_COLOR_READ_TYPE:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.implementationColorReadType;
		}
		break;
		case GL_LINE_WIDTH:
			*data = static_cast<GLint64>(m_state.getLineWidth());
			break;
		case GL_MAJOR_VERSION:
			*data = 3;
			break;
		case GL_MAX_3D_TEXTURE_SIZE:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.max3dTextureSize;
		}
		break;
		case GL_MAX_ARRAY_TEXTURE_LAYERS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxArrayTextureLayers;
		}
		break;
		case GL_MAX_COLOR_ATTACHMENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxColorAttachments;
		}
		break;
		case GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxCombinedFragmentUniformComponents;
		}
		break;
		case GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxCombinedTextureImageUnits;
		}
		break;
		case GL_MAX_COMBINED_UNIFORM_BLOCKS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxCombinedUniformBlocks;
		}
		break;
		case GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxCombinedVertexUniformComponents;
		}
		break;
		case GL_MAX_CUBE_MAP_TEXTURE_SIZE:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxCubeMapTextureSize;
		}
		break;
		case GL_MAX_DRAW_BUFFERS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxDrawBuffers;
		}
		break;
		case GL_MAX_ELEMENT_INDEX:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxElementIndex;
		}
		break;
		case GL_MAX_ELEMENTS_INDICES:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxElementsIndices;
		}
		break;
		case GL_MAX_ELEMENTS_VERTICES:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxElementsVertices;
		}
		break;
		case GL_MAX_FRAGMENT_INPUT_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxFragmentInputComponents;
		}
		break;
		case GL_MAX_FRAGMENT_UNIFORM_BLOCKS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxFragmentUniformBlocks;
		}
		break;
		case GL_MAX_FRAGMENT_UNIFORM_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxFragmentUniformComponents;
		}
		break;
		case GL_MAX_FRAGMENT_UNIFORM_VECTORS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxFragmentUniformVectors;
		}
		break;
		case GL_MAX_PROGRAM_TEXEL_OFFSET:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxProgramTexelOffset;
		}
		break;
		case GL_MAX_RENDERBUFFER_SIZE:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxRenderbufferSize;
		}
		break;
		case GL_MAX_SAMPLES:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxSamples;
		}
		break;
		case GL_MAX_SERVER_WAIT_TIMEOUT:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxServerWaitTimeout;
		}
		break;
		case GL_MAX_TEXTURE_IMAGE_UNITS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxTextureImageUnits;
		}
		break;
		case GL_MAX_TEXTURE_LOD_BIAS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = static_cast<GLint64>(params.maxTextureLodBias);
		}
		break;
		case GL_MAX_TEXTURE_SIZE:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxTextureSize;
		}
		break;
		case GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxTransformFeedbackInterleavedComponents;
		}
		break;
		case GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxTransformFeedbackSeparateAttribs;
		}
		break;
		case GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxTransformFeedbackSeparateComponents;
		}
		break;
		case GL_MAX_UNIFORM_BLOCK_SIZE:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxUniformBlockSize;
		}
		break;
		case GL_MAX_UNIFORM_BUFFER_BINDINGS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxUniformBufferBindings;
		}
		break;
		case GL_MAX_VARYING_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxVaryingComponents;
		}
		break;
		case GL_MAX_VARYING_VECTORS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxVaryingVectors;
		}
		break;
		case GL_MAX_VERTEX_ATTRIBS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxVertexAttribs;
		}
		break;
		case GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxVertexTextureImageUnits;
		}
		break;
		case GL_MAX_VERTEX_OUTPUT_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxVertexOutputComponents;
		}
		break;
		case GL_MAX_VERTEX_UNIFORM_BLOCKS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxVertexUniformBlocks;
		}
		break;
		case GL_MAX_VERTEX_UNIFORM_COMPONENTS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxVertexUniformComponents;
		}
		break;
		case GL_MAX_VERTEX_UNIFORM_VECTORS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.maxVertexUniformVectors;
		}
		break;
		case GL_MAX_VIEWPORT_DIMS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			data[0] = params.maxViewportDims[0];
			data[1] = params.maxViewportDims[1];
		}
		break;
		case GL_MIN_PROGRAM_TEXEL_OFFSET:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.minProgramTexelOffset;
		}
		break;
		case GL_MINOR_VERSION:
			*data = 0;
			break;
		case GL_NUM_COMPRESSED_TEXTURE_FORMATS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.numCompressedTextureFormats;
		}
		break;
		case GL_NUM_EXTENSIONS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.numExtensions;
		}
		break;
		case GL_NUM_PROGRAM_BINARY_FORMATS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.numProgramBinaryFormats;
		}
		break;
		case GL_NUM_SHADER_BINARY_FORMATS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.numShaderBinaryFormats;
		}
		break;
		case GL_PACK_ALIGNMENT:
		{
			const CoreState::PackParams& params = m_state.getPackParams();
			*data = params.packAlignment;
		}
		break;
		case GL_PACK_ROW_LENGTH:
		{
			const CoreState::PackParams& params = m_state.getPackParams();
			*data = params.packRowLength;
		}
		break;
		case GL_PACK_SKIP_PIXELS:
		{
			const CoreState::PackParams& params = m_state.getPackParams();
			*data = params.packSkipPixels;
		}
		break;
		case GL_PACK_SKIP_ROWS:
		{
			const CoreState::PackParams& params = m_state.getPackParams();
			*data = params.packSkipRows;
		}
		break;
		case GL_PIXEL_PACK_BUFFER_BINDING:
		{
			CoreBuffer* core_buffer = m_state.getBuffer(GL_PIXEL_PACK_BUFFER);
			if (core_buffer != nullptr) {
				*data = core_buffer->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_PIXEL_UNPACK_BUFFER_BINDING:
		{
			CoreBuffer* core_buffer = m_state.getBuffer(GL_PIXEL_UNPACK_BUFFER);
			if (core_buffer != nullptr) {
				*data = core_buffer->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_POLYGON_OFFSET_FACTOR:
		{
			const PolygonOffsetParams& params = m_state.getPolygonOffsetParams();
			*data = static_cast<GLint64>(params.polygonOffsetFactor);
		}
		break;
		case GL_POLYGON_OFFSET_FILL:
		{
			const PolygonOffsetParams& params = m_state.getPolygonOffsetParams();
			*data = params.polygonOffsetFillEnable;
		}
		break;
		case GL_POLYGON_OFFSET_UNITS:
		{
			const PolygonOffsetParams& params = m_state.getPolygonOffsetParams();
			*data = static_cast<GLint64>(params.polygonOffsetUnits);
		}
		break;
		case GL_PRIMITIVE_RESTART_FIXED_INDEX:
			*data = m_state.getEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
			break;
		case GL_PROGRAM_BINARY_FORMATS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			for (int i = 0; i < params.numProgramBinaryFormats; i++) {
				data[i] = params.programBinaryFormats[i];
			}
		}
		break;
		case GL_RASTERIZER_DISCARD:
			*data = m_state.getEnable(GL_RASTERIZER_DISCARD);
			break;
		case GL_READ_BUFFER:
		{
			GLenum read_buffer = m_state.getReadBuffer();
			*data = read_buffer;
		}
		break;
		case GL_READ_FRAMEBUFFER_BINDING:
		{
			CoreFramebuffer* core_framebuffer = m_state.getFramebuffer(GL_READ_FRAMEBUFFER);
			if (core_framebuffer != nullptr) {
				*data = core_framebuffer->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_RED_BITS:
		{
			GLenum format = getDrawFramebufferFormat(0);
			*data = getRedBitsFromFormat(format);
		}
		break;
		case GL_RENDERBUFFER_BINDING:
		{
			CoreRenderbuffer* core_renderbuffer = m_state.getRenderbuffer(GL_RENDERBUFFER);
			if (core_renderbuffer != nullptr) {
				*data = core_renderbuffer->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_SAMPLE_ALPHA_TO_COVERAGE:
		{
			const SampleCoverageParams& params = m_state.getSampleCoverageParams();
			*data = params.sampleAlphaToCoverageEnable;
		}
		break;
		case GL_SAMPLE_BUFFERS:
		{
			// TODO: Framebuffer のマルチサンプル数
		}
		break;
		case GL_SAMPLE_COVERAGE:
		{
			const SampleCoverageParams& params = m_state.getSampleCoverageParams();
			*data = params.sampleCoverageEnable;
		}
		break;
		case GL_SAMPLE_COVERAGE_INVERT:
		{
			const SampleCoverageParams& params = m_state.getSampleCoverageParams();
			*data = params.sampleCoverageInvert;
		}
		break;
		case GL_SAMPLE_COVERAGE_VALUE:
		{
			const SampleCoverageParams& params = m_state.getSampleCoverageParams();
			*data = static_cast<GLint64>(params.sampleCoverageValue);
		}
		break;
		case GL_SAMPLER_BINDING:
		{
			GLenum active_texture = m_state.getActiveTexture();
			GLuint unit_index = active_texture - GL_TEXTURE0;
			CoreSampler* core_sampler = m_state.getSampler(unit_index);
			if (core_sampler != nullptr) {
				*data = core_sampler->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_SAMPLES:
			// TODO: framebuffer のサンプル数
			break;
		case GL_SCISSOR_BOX:
		{
			const ScissorParams& params = m_state.getScissorParams();
			data[0] = params.scissorBox[0];
			data[1] = params.scissorBox[1];
			data[2] = params.scissorBox[2];
			data[3] = params.scissorBox[3];
		}
		break;
		case GL_SCISSOR_TEST:
		{
			const ScissorParams& params = m_state.getScissorParams();
			*data = params.scissorTestEnable;
		}
		break;
		case GL_SHADER_BINARY_FORMATS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			for (int i = 0; i < params.numShaderBinaryFormats; i++) {
				data[i] = params.shaderBinaryFormats[i];
			}
		}
		break;
		case GL_SHADER_COMPILER:
			*data = GL_TRUE; // 常にGL_TRUE
			break;
		case GL_STENCIL_BACK_FAIL:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = params.stencilBackFail;
		}
		break;
		case GL_STENCIL_BACK_FUNC:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = params.stencilBackFunc;
		}
		break;
		case GL_STENCIL_BACK_PASS_DEPTH_FAIL:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = params.stencilBackPassDepthFail;
		}
		break;
		case GL_STENCIL_BACK_PASS_DEPTH_PASS:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = params.stencilBackPassDepthPass;
		}
		break;
		case GL_STENCIL_BACK_REF:
		{
			const StencilReference& st_ref = m_state.getStencilReference();
			*data = st_ref.stencilBackRef;
		}
		break;
		case GL_STENCIL_BACK_VALUE_MASK:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = params.stencilBackValueMask;
		}
		break;
		case GL_STENCIL_BACK_WRITEMASK:
		{
			const DepthStencilWritemaskParams& params = m_state.getDepthStencilWritemaskParams();
			*data = params.stencilBackWritemask;
		}
		break;
		case GL_STENCIL_BITS:
		{
			GLenum format = getDrawFramebufferFormat(0);
			*data = getGreenBitsFromFormat(format);
		}
		break;
		case GL_STENCIL_CLEAR_VALUE:
		{
			const ClearParameters& params = m_state.getClearParams();
			*data = static_cast<GLuint64>(params.stencilClearValue);
		}
		break;
		case GL_STENCIL_FAIL:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = params.stencilFail;
		}
		break;
		case GL_STENCIL_FUNC:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = params.stencilFunc;
		}
		break;
		case GL_STENCIL_PASS_DEPTH_FAIL:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = params.stencilPassDepthFail;
		}
		break;
		case GL_STENCIL_PASS_DEPTH_PASS:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = params.stencilPassDepthPass;
		}
		break;
		case GL_STENCIL_REF:
		{
			const StencilReference& st_ref = m_state.getStencilReference();
			*data = st_ref.stencilRef;
		}
		break;
		case GL_STENCIL_TEST:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = params.stencilTestEnable;
		}
		break;
		case GL_STENCIL_VALUE_MASK:
		{
			const StencilTestParams& params = m_state.getStencilTestParams();
			*data = params.stencilValueMask;
		}
		break;
		case GL_STENCIL_WRITEMASK:
		{
			const DepthStencilWritemaskParams& params = m_state.getDepthStencilWritemaskParams();
			*data = params.stencilWritemask;
		}
		break;
		case GL_SUBPIXEL_BITS:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.subpixelBits;
		}
		break;
		case GL_TEXTURE_BINDING_2D:
		{
			CoreTexture* core_texture = m_state.getTexture(GL_TEXTURE_2D);
			if (core_texture != nullptr) {
				*data = core_texture->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_TEXTURE_BINDING_2D_ARRAY:
		{
			CoreTexture* core_texture = m_state.getTexture(GL_TEXTURE_2D_ARRAY);
			if (core_texture != nullptr) {
				*data = core_texture->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_TEXTURE_BINDING_3D:
		{
			CoreTexture* core_texture = m_state.getTexture(GL_TEXTURE_3D);
			if (core_texture != nullptr) {
				*data = core_texture->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_TEXTURE_BINDING_CUBE_MAP:
		{
			CoreTexture* core_texture = m_state.getTexture(GL_TEXTURE_CUBE_MAP);
			if (core_texture != nullptr) {
				*data = core_texture->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_TRANSFORM_FEEDBACK_BINDING:
		{
			CoreTransformFeedback* core_transform_feedback = m_state.getTransformFeedback(GL_TRANSFORM_FEEDBACK);
			if (core_transform_feedback != nullptr) {
				*data = core_transform_feedback->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_TRANSFORM_FEEDBACK_ACTIVE:
		{
			const CoreState::TransformFeedbackParams& params = m_state.getTransformFeedbackParams();
			*data = params.transformFeedbackActive;
		}
		break;
		case GL_TRANSFORM_FEEDBACK_BUFFER_BINDING:
		{
			const IndexedBuffer* buffer = m_state.getIndexedBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);
			if ((buffer != nullptr) && (buffer->buffer != nullptr)) {
				*data = buffer->buffer->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_TRANSFORM_FEEDBACK_PAUSED:
		{
			const CoreState::TransformFeedbackParams& params = m_state.getTransformFeedbackParams();
			*data = params.transformFeedbackPaused;
		}
		break;
		case GL_TRANSFORM_FEEDBACK_BUFFER_SIZE:
		{
			const IndexedBuffer* buffer = m_state.getIndexedBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);
			if (buffer != nullptr) {
				*data = buffer->size;
			} else {
				*data = 0;
			}
		}
		break;
		case GL_TRANSFORM_FEEDBACK_BUFFER_START:
		{
			const IndexedBuffer* buffer = m_state.getIndexedBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);
			if (buffer != nullptr) {
				*data = buffer->offset;
			} else {
				*data = 0;
			}
		}
		break;
		case GL_UNIFORM_BUFFER_BINDING:
		{
			CoreBuffer* core_buffer = m_state.getBuffer(GL_UNIFORM_BUFFER);
			if (core_buffer != nullptr) {
				*data = core_buffer->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT:
		{
			const BackendContext::PlatformParams& params = m_pBackendContext->getPlatformParams();
			*data = params.uniformBufferOffsetAlignment;
		}
		break;
		case GL_UNIFORM_BUFFER_SIZE:
		{
			const IndexedBuffer* buffer = m_state.getIndexedBuffer(GL_UNIFORM_BUFFER, 0);
			if (buffer != nullptr) {
				*data = buffer->size;
			} else {
				*data = 0;
			}
		}
		break;
		case GL_UNIFORM_BUFFER_START:
		{
			const IndexedBuffer* buffer = m_state.getIndexedBuffer(GL_UNIFORM_BUFFER, 0);
			if (buffer != nullptr) {
				*data = buffer->offset;
			} else {
				*data = 0;
			}
		}
		break;
		case GL_UNPACK_ALIGNMENT:
		{
			const CoreState::UnpackParams& params = m_state.getUnpackParams();
			*data = params.unpackAlignment;
		}
		break;
		case GL_UNPACK_IMAGE_HEIGHT:
		{
			const CoreState::UnpackParams& params = m_state.getUnpackParams();
			*data = params.unpackImageHeight;
		}
		break;
		case GL_UNPACK_ROW_LENGTH:
		{
			const CoreState::UnpackParams& params = m_state.getUnpackParams();
			*data = params.unpackRowLength;
		}
		break;
		case GL_UNPACK_SKIP_IMAGES:
		{
			const CoreState::UnpackParams& params = m_state.getUnpackParams();
			*data = params.unpackSkipImages;
		}
		break;
		case GL_UNPACK_SKIP_PIXELS:
		{
			const CoreState::UnpackParams& params = m_state.getUnpackParams();
			*data = params.unpackSkipPixels;
		}
		break;
		case GL_UNPACK_SKIP_ROWS:
		{
			const CoreState::UnpackParams& params = m_state.getUnpackParams();
			*data = params.unpackSkipRows;
		}
		break;
		case GL_VERTEX_ARRAY_BINDING:
		{
			CoreVertexArray* core_vertex_array = m_state.getVertexArray();
			if (core_vertex_array != nullptr) {
				*data = core_vertex_array->getId();
			} else {
				*data = 0;
			}
		}
		break;
		case GL_VIEWPORT:
		{
			const ViewportParams& params = m_state.getViewportParams();
			data[0] = params.viewport[0];
			data[1] = params.viewport[1];
			data[2] = params.viewport[2];
			data[3] = params.viewport[3];
		}
		break;
		default:
			// GL_INVALID_ENUM
			setErrorCode(GL_INVALID_ENUM);
			break;
		}
	}
	return;
}

void CoreContext::getSynciv(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreSync* core_sync = m_pObjectsManager->getSync(sync);
	if (core_sync == nullptr) {
		// GL_INVALID_VALUE
		setErrorCode(GL_INVALID_VALUE);
		return;
	}
	core_sync->getSynciv(pname, bufSize, length, values);
	return;
}

void CoreContext::getInteger64i_v(GLenum target, GLuint index, GLint64* data)
{
	// TODO: index が 範囲外 の GL_INVALID_VALUE
	switch (target) {
	case GL_TRANSFORM_FEEDBACK_BUFFER_BINDING:
	{
		const IndexedBuffer* tfb = m_state.getIndexedBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, index);
		if (tfb != nullptr) {
			if (tfb->buffer != nullptr) {
				*data = tfb->buffer->getId();
			} else {
				*data = 0;
			}
		}
	}
	break;
	case GL_TRANSFORM_FEEDBACK_BUFFER_SIZE:
	{
		const IndexedBuffer* tfb = m_state.getIndexedBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, index);
		if (tfb != nullptr) {
			*data = tfb->size;
		}
	}
	break;
	case GL_TRANSFORM_FEEDBACK_BUFFER_START:
	{
		const IndexedBuffer* tfb = m_state.getIndexedBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, index);
		if (tfb != nullptr) {
			*data = tfb->offset;
		}
	}
	break;
	case GL_UNIFORM_BUFFER_BINDING:
	{
		const IndexedBuffer* tfb = m_state.getIndexedBuffer(GL_UNIFORM_BUFFER, index);
		if (tfb != nullptr) {
			if (tfb->buffer != nullptr) {
				*data = tfb->buffer->getId();
			} else {
				*data = 0;
			}
		}
	}
	break;
	case GL_UNIFORM_BUFFER_SIZE:
	{
		const IndexedBuffer* tfb = m_state.getIndexedBuffer(GL_UNIFORM_BUFFER, index);
		if (tfb != nullptr) {
			*data = tfb->size;
		}
	}
	break;
	case GL_UNIFORM_BUFFER_START:
	{
		const IndexedBuffer* tfb = m_state.getIndexedBuffer(GL_UNIFORM_BUFFER, index);
		if (tfb != nullptr) {
			*data = tfb->offset;
		}
	}
	break;
	default:
		// その他はインデックスなしと同様
		getInteger64v(target, data);
		break;
	}
	return;
}

void CoreContext::getBufferParameteri64v(GLenum target, GLenum pname, GLint64* params)
{
	CoreBuffer* core_buffer = m_state.getBuffer(target);
	if (core_buffer == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_buffer->getBufferParameteri64v(this, pname, params);
	return;
}

void CoreContext::genSamplers(GLsizei count, GLuint* samplers)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	m_pObjectsManager->genSamplers(count, samplers);
	return;
}

void CoreContext::deleteSamplers(GLsizei count, const GLuint* samplers)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	m_pObjectsManager->deleteSamplers(this, count, samplers);
	return;
}

GLboolean CoreContext::isSampler(GLuint sampler)
{
	if (m_pObjectsManager == nullptr) {
		return GL_FALSE;
	}
	CoreSampler* core_sampler = m_pObjectsManager->getSampler(sampler);
	return (core_sampler != nullptr) ? GL_TRUE : GL_FALSE;
}

void CoreContext::bindSampler(GLuint unit, GLuint sampler)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreSampler* core_sampler = m_pObjectsManager->getSampler(sampler);
	m_state.setSampler(this, unit, core_sampler);
	return;
}

void CoreContext::samplerParameteri(GLuint sampler, GLenum pname, GLint param)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreSampler* core_sampler = m_pObjectsManager->getSampler(sampler);
	if (core_sampler == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	core_sampler->setSamplerParameteri(this, pname, param);
	return;
}

void CoreContext::samplerParameteriv(GLuint sampler, GLenum pname, const GLint* param)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreSampler* core_sampler = m_pObjectsManager->getSampler(sampler);
	if (core_sampler == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	core_sampler->setSamplerParameteriv(this, pname, param);
	return;
}

void CoreContext::samplerParameterf(GLuint sampler, GLenum pname, GLfloat param)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreSampler* core_sampler = m_pObjectsManager->getSampler(sampler);
	if (core_sampler == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	core_sampler->setSamplerParameterf(this, pname, param);
	return;
}

void CoreContext::samplerParameterfv(GLuint sampler, GLenum pname, const GLfloat* param)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreSampler* core_sampler = m_pObjectsManager->getSampler(sampler);
	if (core_sampler == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	core_sampler->setSamplerParameterfv(this, pname, param);
	return;
}

void CoreContext::getSamplerParameteriv(GLuint sampler, GLenum pname, GLint* params)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreSampler* core_sampler = m_pObjectsManager->getSampler(sampler);
	if (core_sampler == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	core_sampler->getSamplerParameteriv(this, pname, params);
	return;
}

void CoreContext::getSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat* params)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreSampler* core_sampler = m_pObjectsManager->getSampler(sampler);
	if (core_sampler == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	core_sampler->getSamplerParameterfv(this, pname, params);
	return;
}

void CoreContext::vertexAttribDivisor(GLuint index, GLuint divisor)
{
	CoreVertexArray* core_vertex_array = m_state.getVertexArray();
	if (core_vertex_array != nullptr) {
		// set to vertex array object
		core_vertex_array->setVertexAttribDivisor(index, divisor);
	} else {
		// set to generic state
		m_state.setVertexAttribDivisor(index, divisor);
	}
	return;
}

void CoreContext::bindTransformFeedback(GLenum target, GLuint id)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreTransformFeedback* core_transform_feedback = m_pObjectsManager->getTransformFeedback(id);
	m_state.setTransformFeedback(this, target, core_transform_feedback);
	return;
}

void CoreContext::deleteTransformFeedbacks(GLsizei n, const GLuint* ids)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	m_pObjectsManager->deleteTransformFeedbacks(this, n, ids);
	return;
}

void CoreContext::genTransformFeedbacks(GLsizei n, GLuint* ids)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	m_pObjectsManager->genTransformFeedbacks(n, ids);
	return;
}

GLboolean CoreContext::isTransformFeedback(GLuint id)
{
	if (m_pObjectsManager == nullptr) {
		return GL_FALSE;
	}
	CoreTransformFeedback* core_transform_feedback = m_pObjectsManager->getTransformFeedback(id);
	return (core_transform_feedback != nullptr) ? GL_TRUE : GL_FALSE;
}

void CoreContext::pauseTransformFeedback(void)
{
	CoreTransformFeedback* core_transform_feedback = m_state.getTransformFeedback(GL_TRANSFORM_FEEDBACK);
	if (core_transform_feedback == nullptr) {
		// error: GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_transform_feedback->pause();
	return;
}

void CoreContext::resumeTransformFeedback(void)
{
	CoreTransformFeedback* core_transform_feedback = m_state.getTransformFeedback(GL_TRANSFORM_FEEDBACK);
	if (core_transform_feedback == nullptr) {
		// error: GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_transform_feedback->resume();
	return;
}

void CoreContext::getProgramBinary(GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, void* binary)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreProgram* core_program = m_pObjectsManager->getProgram(program);
	if (core_program == nullptr) {
		// error: GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_program->getProgramBinary(bufSize, length, binaryFormat, binary);
	return;
}

void CoreContext::programBinary(GLuint program, GLenum binaryFormat, const void* binary, GLsizei length)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreProgram* core_program = m_pObjectsManager->getProgram(program);
	if (core_program == nullptr) {
		// error: GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_program->setProgramBinary(binaryFormat, binary, length);
	return;
}

void CoreContext::programParameteri(GLuint program, GLenum pname, GLint value)
{
	if (m_pObjectsManager == nullptr) {
		return;
	}
	CoreProgram* core_program = m_pObjectsManager->getProgram(program);
	if (core_program == nullptr) {
		// error: GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	core_program->setProgramParameteri(pname, value);
	return;
}

void CoreContext::invalidateFramebuffer(GLenum target, GLsizei numAttachments, const GLenum* attachments)
{
	CoreFramebuffer* core_framebuffer = m_state.getFramebuffer(target);
	if (core_framebuffer == nullptr) {
		// default framebuffer なので無視
		return;
	}
	core_framebuffer->invalidateFramebuffer(numAttachments, attachments);
	return;
}

void CoreContext::invalidateSubFramebuffer(GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height)
{
	CoreFramebuffer* core_framebuffer = m_state.getFramebuffer(target);
	if (core_framebuffer == nullptr) {
		// default framebuffer なので無視
		return;
	}
	core_framebuffer->invalidateSubFramebuffer(numAttachments, attachments, x, y, width, height);
	return;
}

void CoreContext::texStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
	CoreTexture* core_texture = m_state.getTexture(target);
	if (core_texture == nullptr) {
		// default texture
		return;
	}
	core_texture->texStorage2d(this, levels, internalformat, width, height);
	return;
}

void CoreContext::texStorage3D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
	CoreTexture* core_texture = m_state.getTexture(target);
	if (core_texture == nullptr) {
		// default texture
		return;
	}
	core_texture->texStorage3d(this, levels, internalformat, width, height, depth);
	return;
}

void CoreContext::getInternalformativ(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params)
{
	if (m_pBackendContext == nullptr) {
		return;
	}
	if (target == GL_RENDERBUFFER) {
		// Renderbuffer internalformat
		// TODO: internalformat check
		uint32_t formats_count = m_pBackendContext->getNumRenderbufferFormat();
		const BackendRenderbufferFormat* renderbuffer_formats = m_pBackendContext->getRenderbufferFormat();
		AXGL_ASSERT(renderbuffer_formats != nullptr);
		const BackendRenderbufferFormat* brf = getRenderbufferFormat(formats_count, renderbuffer_formats, internalformat);
		if (brf != nullptr) {
			int32_t output_count;
			switch (pname) {
			case GL_SAMPLES:
				// Multisample でサポートするサンプル数
				output_count = (bufSize < brf->sample_counts) ? bufSize : brf->sample_counts;
				for (int32_t i = 0; i < output_count; i++) {
					params[i] = brf->samples[i];
				}
				break;
			case GL_NUM_SAMPLE_COUNTS:
				// GL_SAMPLES で返す数
				*params = brf->sample_counts;
				break;
			default:
				// GL_INVALID_ENUM
				setErrorCode(GL_INVALID_ENUM);
				break;
			}
		} else {
			// Backend がサポートしない
			switch (pname) {
			case GL_SAMPLES:
				// 何も出力しない
				break;
			case GL_NUM_SAMPLE_COUNTS:
				// GL_SAMPLES で返す数
				*params = 0;
				break;
			default:
				// GL_INVALID_ENUM
				setErrorCode(GL_INVALID_ENUM);
				break;
			}
		}
	} else {
		setErrorCode(GL_INVALID_ENUM);
	}
	return;
}

CoreRenderbuffer* CoreContext::getCurrentRenderbuffer()
{
	return m_state.getRenderbuffer(GL_RENDERBUFFER);
}

void CoreContext::invalidateCache(GLbitfield flags)
{
	if (m_pBackendContext == nullptr) {
		return;
	}
	m_pBackendContext->invalidateCache(flags);

	return;
}

GLenum CoreContext::getDrawFramebufferFormat(int colorIndex)
{
	// TODO: GL_DRAW_FRAMEBUFFER がバインドされている場合、framebuffer の color attachment から
	//       GL_DRAW_FRAMEBUFFER がバインドされていない場合、デフォルト framebuffer の情報から
	AXGL_UNUSED(colorIndex);
	return GL_RGBA8;
}

GLenum CoreContext::getDrawDepthbufferFormat()
{
	// TODO: GL_DRAW_FRAMEBUFFER がバインドされている場合、framebuffer の depth attachment から
	//       GL_DRAW_FRAMEBUFFER がバインドされていない場合、デフォルト framebuffer の情報から
	return GL_DEPTH_COMPONENT16;
}

GLenum CoreContext::getDrawStencilbufferFormat()
{
	// TODO: GL_DRAW_FRAMEBUFFER がバインドされている場合、framebuffer の stencil attachment から
	//       GL_DRAW_FRAMEBUFFER がバインドされていない場合、デフォルト framebuffer の情報から
	return GL_STENCIL_INDEX8;
}

const BackendRenderbufferFormat* CoreContext::getRenderbufferFormat(GLuint count, const BackendRenderbufferFormat* formats, GLenum format)
{
	const BackendRenderbufferFormat* rptr = nullptr;
	for (GLuint i = 0; i < count; i++) {
		if (format == formats[i].internalformat) {
			rptr = &formats[i];
			break;
		}
	}
	return rptr;
}

} // namespace axgl
