// CoreFramebuffer.cpp
// Framebufferクラスの実装
#include "CoreFramebuffer.h"
#include "CoreContext.h"
#include "CoreRenderbuffer.h"
#include "CoreTexture.h"
#include "CoreUtility.h"
#include "../common/PipelineState.h"
#include "../backend/BackendFramebuffer.h"

namespace axgl {

// コンストラクタ
CoreFramebuffer::CoreFramebuffer()
{
	m_objectType = TYPE_FRAMEBUFFER;
}

// デストラクタ
CoreFramebuffer::~CoreFramebuffer()
{
}

// ターゲットを設定
void CoreFramebuffer::setTarget(GLenum target)
{
	m_target = target;
}

// ターゲットを取得
GLenum CoreFramebuffer::getTarget() const 
{
	return m_target;
}

// ステータスをチェックする
GLenum CoreFramebuffer::checkStatus() const
{
	if (m_pBackendFramebuffer == nullptr) {
		// unsupported
		return GL_FRAMEBUFFER_UNSUPPORTED;
	}
	
	// incomplete missing attachment check
	GLenum status = GL_FRAMEBUFFER_COMPLETE;
	{
		bool attached = false;
		for (int i = 0; i < AXGL_MAX_COLOR_ATTACHMENTS; i++) {
			if (m_colorAttachment[i].m_pObject != nullptr) {
				attached = true;
				break;
			}
		}
		if ((m_depthAttachment.m_pObject != nullptr) || (m_stencilAttachment.m_pObject != nullptr)) {
			attached = true;
		}

		if (!attached) {
			status = GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT;
		}
	}
	// check in backend framebuffer
	if (status == GL_FRAMEBUFFER_COMPLETE) {
		status = m_pBackendFramebuffer->checkStatus();
	}
	return status;
}

// Renderbufferを設定する
void CoreFramebuffer::setRenderbuffer(CoreContext* context, GLenum attachment, CoreRenderbuffer* renderbuffer)
{
	int temp = attachment - GL_COLOR_ATTACHMENT0;

	if(attachment == GL_DEPTH_ATTACHMENT) {
		// depth
		// TODO: format check
		setObjectToAttachment(context, &m_depthAttachment, renderbuffer);
	} else if(attachment == GL_STENCIL_ATTACHMENT) {
		// stencil
		// TODO: format check
		setObjectToAttachment(context, &m_stencilAttachment, renderbuffer);
	} else if(attachment == GL_DEPTH_STENCIL_ATTACHMENT) {
		// depth_stencil
		// TODO: format check
		setObjectToAttachment(context, &m_depthAttachment, renderbuffer);
		setObjectToAttachment(context, &m_stencilAttachment, renderbuffer);
	} else if ((temp >= 0) && (temp < AXGL_MAX_COLOR_ATTACHMENTS)) {
		// color
		// TODO: format check
		setObjectToAttachment(context, &m_colorAttachment[temp], renderbuffer);
	} else {
		// GL_INVALID_ENUM is generated if attachment is not one of the accepted attachment points.
		context->setErrorCode(GL_INVALID_ENUM);
		return;
	}
	// set renderbuffer to BackendFramebuffer
	if (m_pBackendFramebuffer != nullptr) {
		BackendContext* backend_context = context->getBackendContext();
		AXGL_ASSERT(backend_context != nullptr);
		BackendRenderbuffer* backend_renderbuffer = nullptr;
		if (renderbuffer != nullptr) {
			backend_renderbuffer = renderbuffer->getBackendRenderbuffer();
		}

		bool result = m_pBackendFramebuffer->setRenderbuffer(backend_context, attachment, backend_renderbuffer);
		if (!result) {
			AXGL_DBGOUT("BackendFramebuffer::setTexture2d() failed\n");
		}
	}
	return;
}

// 2Dテクスチャを設定する
void CoreFramebuffer::setTexture2d(CoreContext* context, GLenum attachment, CoreTexture* texture, GLenum textarget, GLint level)
{
	bool attach_tex = true;

	if (texture != nullptr) {
		attach_tex = false;
		GLenum target = texture->getTarget();

		switch (target) {
		case GL_TEXTURE_2D:
			if (textarget == GL_TEXTURE_2D) {
				textarget = 0;
				attach_tex = true;
			}
			break;
		case GL_TEXTURE_CUBE_MAP:
			if ((textarget >= GL_TEXTURE_CUBE_MAP_POSITIVE_X) && (textarget <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)) {
				attach_tex = true;
			}
			break;
		default:
			// TODO: 無効なテクスチャ
			break;
		}
	} else {
		// clear to default value
		textarget = 0;
		level = 0;
	}
	if (attach_tex) {
		int temp = attachment - GL_COLOR_ATTACHMENT0;
		if ((temp >= 0) && (temp < AXGL_MAX_COLOR_ATTACHMENTS)) {
			// color
			// TODO: format check
			setObjectToAttachment(context, &m_colorAttachment[temp], texture, textarget, level);
		} else if(attachment == GL_DEPTH_ATTACHMENT) {
			// depth
			// TODO: format check
			setObjectToAttachment(context, &m_depthAttachment, texture, textarget, level);
		} else if(attachment == GL_STENCIL_ATTACHMENT) {
			// stencil
			// TODO: format check
			setObjectToAttachment(context, &m_stencilAttachment, texture, textarget, level);
		} else if(attachment == GL_DEPTH_STENCIL_ATTACHMENT) {
			// depth_stencil
			// TODO: format check
			setObjectToAttachment(context, &m_depthAttachment, texture, textarget, level);
			setObjectToAttachment(context, &m_stencilAttachment, texture, textarget, level);
		} else {
			// GL_INVALID_ENUM is generated if attachment is not one of the accepted attachment points.
			context->setErrorCode(GL_INVALID_ENUM);
		}

		// set texture to BackendFramebuffer
		if (m_pBackendFramebuffer != nullptr) {
			BackendContext* backend_context = context->getBackendContext();
			AXGL_ASSERT(backend_context != nullptr);
			BackendTexture* backend_texture = nullptr;
			if (texture != nullptr) {
				backend_texture = texture->getBackendTexture();
			}

			bool result = m_pBackendFramebuffer->setTexture2d(backend_context, attachment, backend_texture, textarget, level);
			if (!result) {
				AXGL_DBGOUT("BackendFramebuffer::setTexture2d() failed\n");
			}
		}
	}
	return;
}

// テクスチャのレイヤーを設定する
void CoreFramebuffer::setTextureLayer(CoreContext* context, GLenum attachment, CoreTexture* texture, GLint level, GLint layer)
{
	bool attach_tex = true;
	if (texture != nullptr) {
		attach_tex = false;
		GLenum target = texture->getTarget();
		switch (target) {
		case GL_TEXTURE_3D:
		case GL_TEXTURE_2D_ARRAY:
			attach_tex = true;
			break;
		default:
			// TODO: 無効なテクスチャ
			break;
		}
	} else {
		// clear to default value
		level = 0;
		layer = 0;
	}
	if (attach_tex) {
		int temp = attachment - GL_COLOR_ATTACHMENT0;
		if (temp < AXGL_MAX_COLOR_ATTACHMENTS) {
			// color
			// TODO: format check
			setObjectToAttachment(context, &m_colorAttachment[temp], texture, 0, level, layer);
		} else if(attachment == GL_DEPTH_ATTACHMENT) {
			// depth
			// TODO: format check
			setObjectToAttachment(context, &m_depthAttachment, texture, 0, level, layer);
		} else if(attachment == GL_STENCIL_ATTACHMENT) {
			// stencil
			// TODO: format check
			setObjectToAttachment(context, &m_stencilAttachment, texture, 0, level, layer);
		} else if(attachment == GL_DEPTH_STENCIL_ATTACHMENT) {
			// depth_stencil
			// TODO: format check
			setObjectToAttachment(context, &m_depthAttachment, texture, 0, level, layer);
			setObjectToAttachment(context, &m_stencilAttachment, texture, 0, level, layer);
		} else {
			// GL_INVALID_ENUM is generated if attachment is not one of the accepted attachment points.
			context->setErrorCode(GL_INVALID_ENUM);
		}
		
		// set texture to BackendFramebuffer
		if (m_pBackendFramebuffer != nullptr) {
			BackendContext* backend_context = context->getBackendContext();
			AXGL_ASSERT(backend_context != nullptr);
			BackendTexture* backend_texture = nullptr;
			if (texture != nullptr) {
				backend_texture = texture->getBackendTexture();
			}
			m_pBackendFramebuffer->setTextureLayer(backend_context, attachment, backend_texture, level, layer);
		}
	}
	return;
}

// FramebufferをInvalidate
void CoreFramebuffer::invalidateFramebuffer(GLsizei numAttachments, const GLenum *attachments)
{
	// TODO: attachment の RBO or Texture に invalidate
	AXGL_UNUSED(numAttachments);
	AXGL_UNUSED(attachments);
	return;
}

// Framebufferの領域をInvalidate
void CoreFramebuffer::invalidateSubFramebuffer(GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height)
{
	// TODO: attachment の RBO or Texture に invalidate
	// NOTE: サポートできるかあやしい
	AXGL_UNUSED(numAttachments);
	AXGL_UNUSED(attachments);
	AXGL_UNUSED(x);
	AXGL_UNUSED(y);
	AXGL_UNUSED(width);
	AXGL_UNUSED(height);
	return;
}

// アタッチメントパラメータを取得
void CoreFramebuffer::getAttachmentParameteriv(GLenum attachment, GLenum pname, GLint *params)
{
	const AttachmentParams* attachment_params = nullptr;
	if ((attachment == GL_DEPTH_STENCIL_ATTACHMENT) || (attachment == GL_DEPTH_ATTACHMENT)) {
		// depth
		attachment_params = &m_depthAttachment;
	} else if (attachment == GL_STENCIL_ATTACHMENT) {
		// stencil
		attachment_params = &m_stencilAttachment;
	} else {
		int color_index = attachment - GL_COLOR_ATTACHMENT0;
		if ((color_index >= 0) && (color_index < AXGL_MAX_COLOR_ATTACHMENTS)) {
			// color
			attachment_params = &m_colorAttachment[color_index];
		}
	}
	if (attachment_params == nullptr) {
		// GL_INVALID_OPERATION
		setErrorCode(GL_INVALID_OPERATION);
		return;
	}
	if (attachment_params->m_pObject == nullptr) {
		// GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE : GL_NONE
		if (pname == GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE) {
			*params = GL_NONE;
		} else {
			// GL_INVALID_ENUM
			setErrorCode(GL_INVALID_ENUM);
		}
		return;
	}
	CoreObject::CoreObjectType object_type = attachment_params->m_pObject->getObjectType();
	if (object_type == TYPE_TEXTURE) {
		CoreTexture* core_texture = static_cast<CoreTexture*>(attachment_params->m_pObject);

		switch (pname) {
		case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE:
			*params = GL_TEXTURE;
			break;
		case GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE:
			{
				GLenum format = core_texture->getInternalformat();
				*params = getRedBitsFromFormat(format);
			}
			break;
		case GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE:
			{
				GLenum format = core_texture->getInternalformat();
				*params = getGreenBitsFromFormat(format);
			}
			break;
		case GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE:
			{
				GLenum format = core_texture->getInternalformat();
				*params = getBlueBitsFromFormat(format);
			}
			break;
		case GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE:
			{
				GLenum format = core_texture->getInternalformat();
				*params = getAlphaBitsFromFormat(format);
			}
			break;
		case GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE:
			{
				GLenum format = core_texture->getInternalformat();
				*params = getDepthBitsFromFormat(format);
			}
			break;
		case GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE:
			{
				GLenum format = core_texture->getInternalformat();
				*params = getStencilBitsFromFormat(format);
			}
			break;
		case GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE:
			{
				GLenum format = core_texture->getInternalformat();
				*params = getComponentTypeFromFormat(format);
			}
			break;
		case GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING:
			{
				GLenum format = core_texture->getInternalformat();
				*params = getColorEncodingFromFormat(format);
			}
			break;
		case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME:
			*params = core_texture->getId();
			break;
		case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL:
			// texture only
			*params = attachment_params->m_textureLevel;
			break;
		case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE:
			// texture only
			*params = attachment_params->m_textureCubeMapFace;
			break;
		case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER:
			// texture only
			*params = attachment_params->m_textureLayer;
			break;
		default:
			// GL_INVALID_ENUM
			setErrorCode(GL_INVALID_ENUM);
			break;
		}
	} else if(object_type == TYPE_RENDERBUFFER) {
		CoreRenderbuffer* core_renderbuffer = static_cast<CoreRenderbuffer*>(attachment_params->m_pObject);

		switch (pname) {
		case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE:
			*params = GL_RENDERBUFFER;
			break;
		case GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE:
			{
				GLenum format = core_renderbuffer->getInternalformat();
				*params = getRedBitsFromFormat(format);
			}
			break;
		case GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE:
			{
				GLenum format = core_renderbuffer->getInternalformat();
				*params = getGreenBitsFromFormat(format);
			}
			break;
		case GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE:
			{
				GLenum format = core_renderbuffer->getInternalformat();
				*params = getBlueBitsFromFormat(format);
			}
			break;
		case GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE:
			{
				GLenum format = core_renderbuffer->getInternalformat();
				*params = getAlphaBitsFromFormat(format);
			}
			break;
		case GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE:
			{
				GLenum format = core_renderbuffer->getInternalformat();
				*params = getDepthBitsFromFormat(format);
			}
			break;
		case GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE:
			{
				GLenum format = core_renderbuffer->getInternalformat();
				*params = getStencilBitsFromFormat(format);
			}
			break;
		case GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE:
			{
				GLenum format = core_renderbuffer->getInternalformat();
				*params = getComponentTypeFromFormat(format);
			}
			break;
		case GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING:
			{
				GLenum format = core_renderbuffer->getInternalformat();
				*params = getColorEncodingFromFormat(format);
			}
			break;
		case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME:
			*params = core_renderbuffer->getId();
			break;
		default:
			// GL_INVALID_ENUM
			setErrorCode(GL_INVALID_ENUM);
			break;
		}
	} else {
		// あり得ない
		AXGL_ASSERT(0);
	}
	return;
}

// 現在のAttachmentから、TargetAttachment構造体の内容を取得
void CoreFramebuffer::setupTargetAttachment(TargetAttachment* targetAttachment)
{
	if (targetAttachment == nullptr) {
		return;
	}
	for (int i = 0; i < AXGL_MAX_COLOR_ATTACHMENTS; i++) {
		targetAttachment->colorFormat[i] = getFormatFromAttachmentParams(m_colorAttachment[i]);
	}
	targetAttachment->depthFormat = getFormatFromAttachmentParams(m_depthAttachment);
	targetAttachment->stencilFormat = getFormatFromAttachmentParams(m_stencilAttachment);
	return;
}

// 初期化
bool CoreFramebuffer::initialize(CoreContext* context)
{
	AXGL_ASSERT(m_pBackendFramebuffer == nullptr);
	m_pBackendFramebuffer = BackendFramebuffer::create();
	if (m_pBackendFramebuffer == nullptr) {
		return false;
	}
	AXGL_ASSERT(context != nullptr);
	BackendContext* backend_context = context->getBackendContext();
	AXGL_ASSERT(backend_context != nullptr);
	bool result = m_pBackendFramebuffer->initialize(backend_context);
	return result;
}

// 終了
void CoreFramebuffer::terminate(CoreContext* context)
{
	// release attached objects
	for (int i = 0; i < AXGL_MAX_COLOR_ATTACHMENTS; i++) {
		if (m_colorAttachment[i].m_pObject != nullptr) {
			m_colorAttachment[i].m_pObject->release(context);
			m_colorAttachment[i].m_pObject = nullptr;
		}
	}
	if (m_depthAttachment.m_pObject != nullptr) {
		m_depthAttachment.m_pObject->release(context);
		m_depthAttachment.m_pObject = nullptr;
	}
	if (m_stencilAttachment.m_pObject != nullptr) {
		m_stencilAttachment.m_pObject->release(context);
		m_stencilAttachment.m_pObject = nullptr;
	}
	if (m_pBackendFramebuffer != nullptr) {
		BackendContext* backend_context = nullptr;
		if (context != nullptr) {
			backend_context = context->getBackendContext();
		}
		m_pBackendFramebuffer->terminate(backend_context);

		BackendFramebuffer::destroy(m_pBackendFramebuffer);
		m_pBackendFramebuffer = nullptr;
	}
	return;
}

// AttachmentParams構造体にオブジェクト(CoreTexture/CoreRenderbuffer)を設定
void CoreFramebuffer::setObjectToAttachment(CoreContext* context, AttachmentParams* attachment, CoreObject* object, GLenum textarget, GLint level, GLint layer)
{
	if (attachment == nullptr) {
		return;
	}
	if (object != nullptr) {
		object->addRef();
	}
	if (attachment->m_pObject != nullptr) {
		attachment->m_pObject->release(context);
	}
	switch (textarget) {
	case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
	case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
	case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
		attachment->m_textureCubeMapFace = textarget;
		break;
	default:
		attachment->m_textureCubeMapFace = 0;
		break;
	}
	attachment->m_pObject = object;
	attachment->m_textureLevel = level;
	attachment->m_textureLayer = layer;
	return;
}

// AttachmentParams構造体からフォーマットを取得
GLenum CoreFramebuffer::getFormatFromAttachmentParams(const AttachmentParams& attachment)
{
	if (attachment.m_pObject == nullptr) {
		return 0;
	}
	GLenum format = 0;
	if (attachment.m_pObject->getObjectType() == TYPE_TEXTURE) {
		const CoreTexture* tex = static_cast<const CoreTexture*>(attachment.m_pObject);
		format = tex->getInternalformat();
	} else {
		const CoreRenderbuffer* rbo = static_cast<const CoreRenderbuffer*>(attachment.m_pObject);
		format = rbo->getInternalformat();
	}
	return format;
}

} // namespace axgl
