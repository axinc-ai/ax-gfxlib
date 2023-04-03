// FramebufferMetal.mm
#include "FramebufferMetal.h"
#include "ContextMetal.h"
#include "RenderbufferMetal.h"
#include "TextureMetal.h"
#include "QueryMetal.h"
#include "../../AXGLAllocatorImpl.h"

namespace axgl {

// BackendFramebufferクラスの実装 --------
BackendFramebuffer* BackendFramebuffer::create()
{
	FramebufferMetal* framebuffer = AXGL_NEW(FramebufferMetal);
	return framebuffer;
}
	
void BackendFramebuffer::destroy(BackendFramebuffer* framebuffer)
{
	if (framebuffer == nullptr) {
		return;
	}
	AXGL_DELETE(framebuffer);
	return;
}

// FramebufferMetalクラスの実装 --------
FramebufferMetal::FramebufferMetal()
{
}

FramebufferMetal::~FramebufferMetal()
{
}

bool FramebufferMetal::initialize(BackendContext* context)
{
	// カラーのアタッチ情報をクリア
	memset(m_colorAttachInfo, 0, sizeof(m_colorAttachInfo));
	// RenderPassDescriptorを作成
	m_mtlRenderPassDesc = [[MTLRenderPassDescriptor alloc] init];
	m_renderPassModified = true;
	return (m_mtlRenderPassDesc != nil) ? true : false;
}

void FramebufferMetal::terminate(BackendContext* context)
{
	// RenderPassDescriptorをリリース
	m_mtlRenderPassDesc = nil;
	// カラーのアタッチ情報をクリアしておく
	memset(m_colorAttachInfo, 0, sizeof(m_colorAttachInfo));
	return;
}

bool FramebufferMetal::setRenderbuffer(BackendContext* context, GLenum attachment, BackendRenderbuffer* renderbuffer)
{
	if (m_mtlRenderPassDesc == nil) {
		return false;
	}
	// レンダーバッファからMTLTextureを取得
	RenderbufferMetal* renderbuffer_metal = static_cast<RenderbufferMetal*>(renderbuffer);
	id<MTLTexture> mtl_texture = nil;
	if (renderbuffer_metal != nullptr) {
		mtl_texture = renderbuffer_metal->getMtlTexture();
	}
	// 指定されたアタッチメントに設定
	switch (attachment) {
	case GL_COLOR_ATTACHMENT0:
	case GL_COLOR_ATTACHMENT1:
	case GL_COLOR_ATTACHMENT2:
	case GL_COLOR_ATTACHMENT3:
	case GL_COLOR_ATTACHMENT4:
	case GL_COLOR_ATTACHMENT5:
	case GL_COLOR_ATTACHMENT6:
	case GL_COLOR_ATTACHMENT7:
		{
			int index = attachment - GL_COLOR_ATTACHMENT0;
			AXGL_ASSERT(index <= 7);
			setColorAttachment(index, mtl_texture, 0, 0, 0, MTLLoadActionLoad, MTLStoreActionStore);
			if (renderbuffer_metal == nullptr) {
				unsetColor(index);
			} else {
				setColorRenderbuffer(index, renderbuffer_metal);
			}
		}
		break;
	case GL_DEPTH_ATTACHMENT:
		setDepthAttachment(mtl_texture, 0, 0, 0, MTLLoadActionLoad, MTLStoreActionStore);
		break;
	case GL_STENCIL_ATTACHMENT:
		setStencilAttachment(mtl_texture, 0, 0, 0, MTLLoadActionLoad, MTLStoreActionStore);
		break;
	case GL_DEPTH_STENCIL_ATTACHMENT:
		setDepthAttachment(mtl_texture, 0, 0, 0, MTLLoadActionLoad, MTLStoreActionStore);
		setStencilAttachment(mtl_texture, 0, 0, 0, MTLLoadActionLoad, MTLStoreActionStore);
		break;
	default:
		AXGL_ASSERT(0);
		break;
	}
	// 変更ありに設定
	m_renderPassModified = true;
	return true;
}

bool FramebufferMetal::setTexture2d(BackendContext* context, GLenum attachment, BackendTexture* texture,
	GLenum textarget, GLint level)
{
	if (m_mtlRenderPassDesc == nil) {
		return false;
	}
	// テクスチャからMTLTextureを取得
	TextureMetal* texture_metal = static_cast<TextureMetal*>(texture);
	id<MTLTexture> mtl_texture = nil;
	MTLPixelFormat pixel_format = MTLPixelFormatInvalid;
	if (texture_metal != nullptr) {
		mtl_texture = texture_metal->getMtlTexture();
		pixel_format = texture_metal->getPixelFormat();
	}
	// スライスをGL->Metal変換
	int slice = 0;
	if ((textarget >= GL_TEXTURE_CUBE_MAP_POSITIVE_X) && (textarget <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)) {
		slice = textarget - GL_TEXTURE_CUBE_MAP_POSITIVE_X;
	}
	// 指定されたアタッチメントに設定
	switch (attachment) {
	case GL_COLOR_ATTACHMENT0:
	case GL_COLOR_ATTACHMENT1:
	case GL_COLOR_ATTACHMENT2:
	case GL_COLOR_ATTACHMENT3:
	case GL_COLOR_ATTACHMENT4:
	case GL_COLOR_ATTACHMENT5:
	case GL_COLOR_ATTACHMENT6:
	case GL_COLOR_ATTACHMENT7:
		{
			int index = attachment - GL_COLOR_ATTACHMENT0;
			AXGL_ASSERT(index <= 7);
			setColorAttachment(index, mtl_texture, level, slice, 0, MTLLoadActionLoad, MTLStoreActionStore);
			if (texture_metal == nullptr) {
				unsetColor(index);
			} else {
				setColorTexture(index, texture_metal);
			}
		}
		break;
	case GL_DEPTH_ATTACHMENT:
		setDepthAttachment(mtl_texture, level, slice, 0, MTLLoadActionLoad, MTLStoreActionStore);
		break;
	case GL_STENCIL_ATTACHMENT:
		setStencilAttachment(mtl_texture, level, slice, 0, MTLLoadActionLoad, MTLStoreActionStore);
		break;
	case GL_DEPTH_STENCIL_ATTACHMENT:
		setDepthAttachment(mtl_texture, level, slice, 0, MTLLoadActionLoad, MTLStoreActionStore);
		setStencilAttachment(mtl_texture, level, slice, 0, MTLLoadActionLoad, MTLStoreActionStore);
		break;
	default:
		AXGL_ASSERT(0);
		break;
	}
	// 変更ありに設定
	m_renderPassModified = true;
	return true;
}

bool FramebufferMetal::setTextureLayer(BackendContext* context, GLenum attachment, BackendTexture* texture,
	GLint level, GLint layer)
{
	if (m_mtlRenderPassDesc == nil) {
		return false;
	}
	// テクスチャからMTLTextureを取得
	TextureMetal* texture_metal = static_cast<TextureMetal*>(texture);
	id<MTLTexture> mtl_texture = nil;
	if (texture_metal != nullptr) {
		mtl_texture = texture_metal->getMtlTexture();
	}
	// 指定されたアタッチメントに設定
	switch (attachment) {
	case GL_COLOR_ATTACHMENT0:
	case GL_COLOR_ATTACHMENT1:
	case GL_COLOR_ATTACHMENT2:
	case GL_COLOR_ATTACHMENT3:
	case GL_COLOR_ATTACHMENT4:
	case GL_COLOR_ATTACHMENT5:
	case GL_COLOR_ATTACHMENT6:
	case GL_COLOR_ATTACHMENT7:
		{
			int index = attachment - GL_COLOR_ATTACHMENT0;
			AXGL_ASSERT(index <= 7);
			setColorAttachment(index, mtl_texture, level, 0, layer, MTLLoadActionLoad, MTLStoreActionStore);
			if (texture_metal == nullptr) {
				unsetColor(index);
			} else {
				setColorTexture(index, texture_metal);
			}
		}
		break;
	case GL_DEPTH_ATTACHMENT:
		setDepthAttachment(mtl_texture, level, 0, layer, MTLLoadActionLoad, MTLStoreActionStore);
		break;
	case GL_STENCIL_ATTACHMENT:
		setStencilAttachment(mtl_texture, level, 0, layer, MTLLoadActionLoad, MTLStoreActionStore);
		break;
	case GL_DEPTH_STENCIL_ATTACHMENT:
		setDepthAttachment(mtl_texture, level, 0, layer, MTLLoadActionLoad, MTLStoreActionStore);
		setStencilAttachment(mtl_texture, level, 0, layer, MTLLoadActionLoad, MTLStoreActionStore);
		break;
	default:
		AXGL_ASSERT(0);
		break;
	}
	// 変更ありに設定
	m_renderPassModified = true;
	return true;
}

GLenum FramebufferMetal::checkStatus() const
{
	if (m_mtlRenderPassDesc == nil) {
		return GL_FRAMEBUFFER_UNSUPPORTED;
	}
	GLenum status = GL_FRAMEBUFFER_COMPLETE;
	// アタッチメントをチェックする
	{
		bool attached = false;
		// color
		for (int i = 0; i < AXGL_MAX_COLOR_ATTACHMENTS; i++) {
			if (m_colorAttachInfo[i].type != ObjectTypeNone) {
				attached = true;
			}
		}
		// depth
		if (m_mtlRenderPassDesc.depthAttachment.texture != nil) {
			attached = true;
		}
		// stencil
		if (m_mtlRenderPassDesc.stencilAttachment.texture != nil) {
			attached = true;
		}
		if (!attached) {
			status = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
		}
	}
	// TODO: マルチサンプル
	// GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE
	return status;
}

bool FramebufferMetal::setupRenderPassDescriptor(QueryMetal* query, GLbitfield clearBits, const ClearParameters* clearParams)
{
	AXGL_ASSERT(clearParams != nullptr);
	// クリアカラー
	const float* src = clearParams->colorClearValue[0];
	MTLClearColor mtl_clear_color = MTLClearColorMake((double)src[0], (double)src[1], (double)src[2], (double)src[3]);
	// クリアが指定されていたら、エンコーディングの区切りとしてmodifiedで処理
	bool modified = m_renderPassModified || (clearBits != 0);
	// カラーアタッチメント
	MTLLoadAction load_action = ((clearBits & GL_COLOR_BUFFER_BIT) != 0) ? MTLLoadActionClear : MTLLoadActionLoad;
	for (int i = 0; i < AXGL_MAX_COLOR_ATTACHMENTS; i++) {
		if (m_colorAttachInfo[i].type == ObjectTypeRenderbuffer) {
			// Drawableから取得したテクスチャが変わっているかチェックして設定
			const RenderbufferMetal* rb = m_colorAttachInfo[i].renderbuffer;
			if ((rb != nullptr) && rb->isLayerStorage()) {
				id<MTLTexture> mtl_tex = rb->getMtlTextureFromDrawable();
				if (m_mtlRenderPassDesc.colorAttachments[i].texture != mtl_tex) {
					m_mtlRenderPassDesc.colorAttachments[i].texture = mtl_tex;
					modified = true;
				}
			}
		}
		m_mtlRenderPassDesc.colorAttachments[i].loadAction = load_action;
		m_mtlRenderPassDesc.colorAttachments[i].clearColor = mtl_clear_color;
	}
	// 深度
	m_mtlRenderPassDesc.depthAttachment.loadAction = ((clearBits & GL_DEPTH_BUFFER_BIT) != 0) ? MTLLoadActionClear : MTLLoadActionLoad;
	m_mtlRenderPassDesc.depthAttachment.clearDepth = (double)clearParams->depthClearValue;
	// ステンシル
	m_mtlRenderPassDesc.stencilAttachment.loadAction = ((clearBits & GL_STENCIL_BUFFER_BIT) != 0) ? MTLLoadActionClear : MTLLoadActionLoad;
	m_mtlRenderPassDesc.stencilAttachment.clearStencil = (uint32_t)clearParams->stencilClearValue;
	// VisibilityResultBuffer
	id<MTLBuffer> vr_buffer = nil;
	if (query != nullptr) {
		vr_buffer = query->getVisibilityResultBuffer();
	}
	// VisibilityResultBufferの変更をチェックして設定
	if (m_mtlRenderPassDesc.visibilityResultBuffer != vr_buffer) {
		m_mtlRenderPassDesc.visibilityResultBuffer = vr_buffer;
		modified = true;
	}
	// 変更なしにクリア
	m_renderPassModified = false;
	return modified;
}

bool FramebufferMetal::setupRenderPassDescriptorForClearI(GLenum buffer, GLint drawbuffer, const GLint* value)
{
	AXGL_ASSERT(value != nullptr);
	if (buffer == GL_COLOR) {
		// クリアカラー
		MTLClearColor mtl_clear_color = MTLClearColorMake((double)value[0], (double)value[1], (double)value[2], (double)value[3]);
		// カラーアタッチメント
		for (int i = 0; i < AXGL_MAX_COLOR_ATTACHMENTS; i++) {
			if (m_colorAttachInfo[i].type == ObjectTypeRenderbuffer) {
				// Drawableから取得したテクスチャを設定
				const RenderbufferMetal* rb = m_colorAttachInfo[i].renderbuffer;
				if ((rb != nullptr) && rb->isLayerStorage()) {
					id<MTLTexture> mtl_tex = rb->getMtlTextureFromDrawable();
					m_mtlRenderPassDesc.colorAttachments[i].texture = mtl_tex;
				}
			}
			m_mtlRenderPassDesc.colorAttachments[i].loadAction = (i == drawbuffer) ? MTLLoadActionClear : MTLLoadActionLoad;
			m_mtlRenderPassDesc.colorAttachments[i].clearColor = mtl_clear_color;
		}
		// ステンシル
		m_mtlRenderPassDesc.stencilAttachment.loadAction = MTLLoadActionLoad;
	} else if (buffer == GL_STENCIL) {
		// カラーアタッチメント
		for (int i = 0; i < AXGL_MAX_COLOR_ATTACHMENTS; i++) {
			if (m_colorAttachInfo[i].type == ObjectTypeRenderbuffer) {
				// Drawableから取得したテクスチャを設定
				const RenderbufferMetal* rb = m_colorAttachInfo[i].renderbuffer;
				if ((rb != nullptr) && rb->isLayerStorage()) {
					id<MTLTexture> mtl_tex = rb->getMtlTextureFromDrawable();
					m_mtlRenderPassDesc.colorAttachments[i].texture = mtl_tex;
				}
			}
			m_mtlRenderPassDesc.colorAttachments[i].loadAction = MTLLoadActionLoad;
		}
		// ステンシル
		m_mtlRenderPassDesc.stencilAttachment.loadAction = MTLLoadActionClear;
		m_mtlRenderPassDesc.stencilAttachment.clearStencil = (uint32_t)(*value);
	}
	// 深度
	m_mtlRenderPassDesc.depthAttachment.loadAction = MTLLoadActionLoad;
	// VisibilityResultBuffer
	m_mtlRenderPassDesc.visibilityResultBuffer = nil;
	// 変更なしにクリア
	m_renderPassModified = false;
	// 機能的にエンコードを分割して実行せざるを得ないため、変更ありとして返す
	return true;
}

bool FramebufferMetal::setupRenderPassDescriptorForClearUI(GLenum buffer, GLint drawbuffer, const GLuint* value)
{
	AXGL_ASSERT(value != nullptr);
	AXGL_ASSERT(buffer == GL_COLOR); // カラーしか有り得ない
	// クリアカラー
	MTLClearColor mtl_clear_color = MTLClearColorMake((double)value[0], (double)value[1], (double)value[2], (double)value[3]);
	// カラーアタッチメント
	for (int i = 0; i < AXGL_MAX_COLOR_ATTACHMENTS; i++) {
		if (m_colorAttachInfo[i].type == ObjectTypeRenderbuffer) {
			const RenderbufferMetal* rb = m_colorAttachInfo[i].renderbuffer;
			if ((rb != nullptr) && rb->isLayerStorage()) {
				// Drawableから取得したテクスチャを設定
				id<MTLTexture> mtl_tex = rb->getMtlTextureFromDrawable();
				m_mtlRenderPassDesc.colorAttachments[i].texture = mtl_tex;
			}
		}
		m_mtlRenderPassDesc.colorAttachments[i].loadAction = (i == drawbuffer) ? MTLLoadActionClear : MTLLoadActionLoad;
		m_mtlRenderPassDesc.colorAttachments[i].clearColor = mtl_clear_color;
	}
	// 深度
	m_mtlRenderPassDesc.depthAttachment.loadAction = MTLLoadActionLoad;
	// ステンシル
	m_mtlRenderPassDesc.stencilAttachment.loadAction = MTLLoadActionLoad;
	// VisibilityResultBuffer
	m_mtlRenderPassDesc.visibilityResultBuffer = nil;
	// 変更なしにクリア
	m_renderPassModified = false;
	// 機能的にエンコードを分割して実行せざるを得ないため、変更ありとして返す
	return true;
}

bool FramebufferMetal::setupRenderPassDescriptorForClearF(GLenum buffer, GLint drawbuffer, const GLfloat* value)
{
	AXGL_ASSERT(value != nullptr);

	if (buffer == GL_COLOR) {
		// クリアカラー
		MTLClearColor mtl_clear_color = MTLClearColorMake((double)value[0], (double)value[1], (double)value[2], (double)value[3]);
		// カラーアタッチメント
		for (int i = 0; i < AXGL_MAX_COLOR_ATTACHMENTS; i++) {
			if (m_colorAttachInfo[i].type == ObjectTypeRenderbuffer) {
				const RenderbufferMetal* rb = m_colorAttachInfo[i].renderbuffer;
				if ((rb != nullptr) && rb->isLayerStorage()) {
					// Drawableから取得したテクスチャを設定
					id<MTLTexture> mtl_tex = rb->getMtlTextureFromDrawable();
					m_mtlRenderPassDesc.colorAttachments[i].texture = mtl_tex;
				}
			}
			m_mtlRenderPassDesc.colorAttachments[i].loadAction = (i == drawbuffer) ? MTLLoadActionClear : MTLLoadActionLoad;
			m_mtlRenderPassDesc.colorAttachments[i].clearColor = mtl_clear_color;
		}
		// 深度
		m_mtlRenderPassDesc.depthAttachment.loadAction = MTLLoadActionLoad;
	} else if (buffer == GL_DEPTH) {
		// カラーアタッチメント
		for (int i = 0; i < AXGL_MAX_COLOR_ATTACHMENTS; i++) {
			if (m_colorAttachInfo[i].type == ObjectTypeRenderbuffer) {
				const RenderbufferMetal* rb = m_colorAttachInfo[i].renderbuffer;
				if ((rb != nullptr) && rb->isLayerStorage()) {
					// Drawableから取得したテクスチャを設定
					id<MTLTexture> mtl_tex = rb->getMtlTextureFromDrawable();
					m_mtlRenderPassDesc.colorAttachments[i].texture = mtl_tex;
				}
			}
			m_mtlRenderPassDesc.colorAttachments[i].loadAction = MTLLoadActionLoad;
		}
		// 深度
		m_mtlRenderPassDesc.depthAttachment.loadAction = MTLLoadActionClear;
		m_mtlRenderPassDesc.depthAttachment.clearDepth = (double)(*value);
	}
	// ステンシル
	m_mtlRenderPassDesc.stencilAttachment.loadAction = MTLLoadActionLoad;
	// VisibilityResultBuffer
	m_mtlRenderPassDesc.visibilityResultBuffer = nil;
	// 変更なしにクリア
	m_renderPassModified = false;
	// 機能的にエンコードを分割して実行せざるを得ないため、変更ありとして返す
	return true;
}

bool FramebufferMetal::setupRenderPassDescriptorForClearFI(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
	AXGL_ASSERT((buffer == GL_DEPTH_STENCIL) && (drawbuffer == 0)); // 深度とステンシルのみ
	// カラーアタッチメント
	for (int i = 0; i < AXGL_MAX_COLOR_ATTACHMENTS; i++) {
		if (m_colorAttachInfo[i].type == ObjectTypeRenderbuffer) {
			const RenderbufferMetal* rb = m_colorAttachInfo[i].renderbuffer;
			if ((rb != nullptr) && rb->isLayerStorage()) {
				// Drawableから取得したテクスチャを設定
				id<MTLTexture> mtl_tex = rb->getMtlTextureFromDrawable();
				m_mtlRenderPassDesc.colorAttachments[i].texture = mtl_tex;
			}
		}
		m_mtlRenderPassDesc.colorAttachments[i].loadAction = MTLLoadActionLoad;
	}
	// 深度
	m_mtlRenderPassDesc.depthAttachment.loadAction = MTLLoadActionClear;
	m_mtlRenderPassDesc.depthAttachment.clearDepth = (double)depth;
	// ステンシル
	m_mtlRenderPassDesc.stencilAttachment.loadAction = MTLLoadActionClear;
	m_mtlRenderPassDesc.stencilAttachment.clearStencil = (uint32_t)stencil;
	// VisibilityResultBuffer
	m_mtlRenderPassDesc.visibilityResultBuffer = nil;
	// 変更なしにクリア
	m_renderPassModified = false;
	// 機能的にエンコードを分割して実行せざるを得ないため、変更ありとして返す
	return true;
}

MTLPixelFormat FramebufferMetal::getColorFormat(int index) const
{
	if ((m_mtlRenderPassDesc == nil) || (index >= AXGL_MAX_COLOR_ATTACHMENTS)) {
		return MTLPixelFormatInvalid;
	}
	MTLPixelFormat format = MTLPixelFormatInvalid;
	id<MTLTexture> texture = m_mtlRenderPassDesc.colorAttachments[index].texture;
	if (texture != nil) {
		format = texture.pixelFormat;
	}
	return format;
}

MTLPixelFormat FramebufferMetal::getDepthFormat() const
{
	if (m_mtlRenderPassDesc == nil) {
		return MTLPixelFormatInvalid;
	}
	MTLPixelFormat format = MTLPixelFormatInvalid;
	id<MTLTexture> texture = m_mtlRenderPassDesc.depthAttachment.texture;
	if (texture != nil) {
		format = texture.pixelFormat;
	}
	return format;
}

MTLPixelFormat FramebufferMetal::getStencilFormat() const
{
	if (m_mtlRenderPassDesc == nil) {
		return MTLPixelFormatInvalid;
	}
	MTLPixelFormat format = MTLPixelFormatInvalid;
	id<MTLTexture> texture = m_mtlRenderPassDesc.stencilAttachment.texture;
	if (texture != nil) {
		format = texture.pixelFormat;
	}
	return format;
}

void FramebufferMetal::getRectSize(RectSize* rectSize) const
{
	AXGL_ASSERT(rectSize != nullptr);
	rectSize->width = 0;
	rectSize->height = 0;
	if (m_mtlRenderPassDesc == nil) {
		return;
	}
	// カラーアタッチメントから取得
	for (int i = 0; i < AXGL_MAX_COLOR_ATTACHMENTS; i++) {
		id<MTLTexture> texture = m_mtlRenderPassDesc.colorAttachments[i].texture;
		if (texture != nil) {
			rectSize->width  = static_cast<uint32_t>([texture width]);
			rectSize->height = static_cast<uint32_t>([texture height]);
			break;
		}
	}
	// 未取得の場合、深度から取得
	if ((rectSize->width == 0) && (m_mtlRenderPassDesc.depthAttachment.texture != nil)) {
		id<MTLTexture> texture = m_mtlRenderPassDesc.depthAttachment.texture;
		rectSize->width = static_cast<uint32_t>([texture width]);
		rectSize->height = static_cast<uint32_t>([texture height]);
	}
	// 未取得の場合、ステンシルから取得
	if ((rectSize->width == 0) && (m_mtlRenderPassDesc.stencilAttachment.texture != nil)) {
		id<MTLTexture> texture = m_mtlRenderPassDesc.stencilAttachment.texture;
		rectSize->width = static_cast<uint32_t>([texture width]);
		rectSize->height = static_cast<uint32_t>([texture height]);
	}
	return;
}

id<MTLTexture> FramebufferMetal::getColorTexture(int index) const
{
	if ((m_mtlRenderPassDesc == nil) || (index >= AXGL_MAX_COLOR_ATTACHMENTS)) {
		return nil;
	}
	id<MTLTexture> mtl_texture = m_mtlRenderPassDesc.colorAttachments[index].texture;
	return mtl_texture;
}

uint32_t FramebufferMetal::getColorTextureLevel(int index) const
{
	if ((m_mtlRenderPassDesc == nil) || (index >= AXGL_MAX_COLOR_ATTACHMENTS)) {
		return 0;
	}
	uint32_t level = (uint32_t)m_mtlRenderPassDesc.colorAttachments[index].level;
	return level;
}

bool FramebufferMetal::onlyOffscreenBufferAttached() const
{
	bool is_onscreen = false;
	// カラーアタッチメントにCAMetalLayerのレンダーバッファを含むか
	for (int i = 0; i < AXGL_MAX_COLOR_ATTACHMENTS; i++) {
		if (m_colorAttachInfo[i].type == ObjectTypeRenderbuffer) {
			const RenderbufferMetal* rb = m_colorAttachInfo[i].renderbuffer;
			if ((rb != nullptr) && rb->isLayerStorage()) {
				is_onscreen = true;
				break;
			}
		}
	}
	// 含まない場合、オフスクリーンのみ
	return !is_onscreen;
}

void FramebufferMetal::setColorRenderbuffer(int index, RenderbufferMetal* renderbuffer)
{
	if (index >= AXGL_MAX_COLOR_ATTACHMENTS) {
		return;
	}
	m_colorAttachInfo[index].type = ObjectTypeRenderbuffer;
	m_colorAttachInfo[index].renderbuffer = renderbuffer;
	return;
}

void FramebufferMetal::setColorTexture(int index, TextureMetal* texture)
{
	if (index >= AXGL_MAX_COLOR_ATTACHMENTS) {
		return;
	}
	m_colorAttachInfo[index].type = ObjectTypeTexture;
	m_colorAttachInfo[index].texture = texture;
	return;
}

void FramebufferMetal::unsetColor(int index)
{
	if (index >= AXGL_MAX_COLOR_ATTACHMENTS) {
		return;
	}
	m_colorAttachInfo[index].type = ObjectTypeNone;
	m_colorAttachInfo[index].renderbuffer = nullptr;
	return;
}

void FramebufferMetal::setColorAttachment(int index, id<MTLTexture> tex, int level, int slice, int depthPlane,
	MTLLoadAction loadAction, MTLStoreAction storeAction)
{
	AXGL_ASSERT(m_mtlRenderPassDesc != nil);
	AXGL_ASSERT(index <= 7);
	m_mtlRenderPassDesc.colorAttachments[index].texture = tex;
	m_mtlRenderPassDesc.colorAttachments[index].loadAction = loadAction;
	m_mtlRenderPassDesc.colorAttachments[index].storeAction = storeAction;
	m_mtlRenderPassDesc.colorAttachments[index].level = level;
	m_mtlRenderPassDesc.colorAttachments[index].slice = slice;
	m_mtlRenderPassDesc.colorAttachments[index].depthPlane = depthPlane;
	return;
}

void FramebufferMetal::setDepthAttachment(id<MTLTexture> tex, int level, int slice, int depthPlane,
	MTLLoadAction loadAction, MTLStoreAction storeAction)
{
	AXGL_ASSERT(m_mtlRenderPassDesc != nil);
	m_mtlRenderPassDesc.depthAttachment.texture = tex;
	m_mtlRenderPassDesc.depthAttachment.loadAction = loadAction;
	m_mtlRenderPassDesc.depthAttachment.storeAction = storeAction;
	m_mtlRenderPassDesc.depthAttachment.level = level;
	m_mtlRenderPassDesc.depthAttachment.slice = slice;
	m_mtlRenderPassDesc.depthAttachment.depthPlane = depthPlane;
	return;
}

void FramebufferMetal::setStencilAttachment(id<MTLTexture> tex, int level, int slice, int depthPlane,
	MTLLoadAction loadAction, MTLStoreAction storeAction)
{
	AXGL_ASSERT(m_mtlRenderPassDesc != nil);
	m_mtlRenderPassDesc.stencilAttachment.texture = tex;
	m_mtlRenderPassDesc.stencilAttachment.loadAction = loadAction;
	m_mtlRenderPassDesc.stencilAttachment.storeAction = storeAction;
	m_mtlRenderPassDesc.stencilAttachment.level = level;
	m_mtlRenderPassDesc.stencilAttachment.slice = slice;
	m_mtlRenderPassDesc.stencilAttachment.depthPlane = depthPlane;
	return;
}

} // namespace axgl
