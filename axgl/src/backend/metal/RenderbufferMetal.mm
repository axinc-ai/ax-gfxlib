// RenderbufferMetal.mm
#include "RenderbufferMetal.h"
#include "ContextMetal.h"
#include "../../AXGLAllocatorImpl.h"

namespace axgl {

// BackendRenderbufferクラスの実装 --------
BackendRenderbuffer* BackendRenderbuffer::create()
{
	RenderbufferMetal* renderbuffer = AXGL_NEW(RenderbufferMetal);
	return renderbuffer;
}
	
void BackendRenderbuffer::destroy(BackendRenderbuffer* renderbuffer)
{
	if (renderbuffer == nullptr) {
		return;
	}
	AXGL_DELETE(renderbuffer);
	return;
}

// RenderbufferMetalクラスの実装 --------
RenderbufferMetal::RenderbufferMetal()
{
}

RenderbufferMetal::~RenderbufferMetal()
{
}

bool RenderbufferMetal::initialize(BackendContext* context)
{
	AXGL_ASSERT((m_mtlTextureDesc == nil) && (m_mtlTexture == nil));
	// MTLTextureDescriptorを作成
	m_mtlTextureDesc = [[MTLTextureDescriptor alloc] init];
	if (m_mtlTextureDesc == nil) {
		return false;
	}
	return true;
}

void RenderbufferMetal::terminate(BackendContext* context)
{
	m_layer = nil;
	m_currentDrawable = nil;
	m_mtlTexture = nil;
	m_mtlTextureDesc = nil;
	return;
}

bool RenderbufferMetal::createStorage(BackendContext* context, GLenum internalformat, GLsizei width, GLsizei height)
{
	// 古いCAMetalLayerをリリース
	releaseLayer();
	// TextureDescriptorを設定
	MTLPixelFormat pixel_format = convert_internalformat(internalformat);
	MTLTextureUsage texture_usage = MTLTextureUsageRenderTarget;
	MTLStorageMode storage_mode = MTLStorageModePrivate;
	[m_mtlTextureDesc setTextureType:MTLTextureType2D];
	[m_mtlTextureDesc setPixelFormat:pixel_format];
	[m_mtlTextureDesc setWidth:width];
	[m_mtlTextureDesc setHeight:height];
	[m_mtlTextureDesc setUsage:texture_usage];
	[m_mtlTextureDesc setStorageMode:storage_mode];
	[m_mtlTextureDesc setSampleCount:1];
	// MTLTextureを作成
	ContextMetal* mtl_context = reinterpret_cast<ContextMetal*>(context);
	id<MTLDevice> mtl_device = mtl_context->getDevice();
	m_mtlTexture = [mtl_device newTextureWithDescriptor:m_mtlTextureDesc];
	if (m_mtlTexture == nil) {
		return false;
	}
	return true;
}

bool RenderbufferMetal::createStorageMultisample(BackendContext* context, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
	// 古いCAMetalLayerをリリース
	releaseLayer();
	// TextureDescriptorを設定
	MTLPixelFormat pixel_format = convert_internalformat(internalformat);
	MTLTextureUsage texture_usage = MTLTextureUsageRenderTarget;
	MTLStorageMode storage_mode = MTLStorageModePrivate;
	[m_mtlTextureDesc setTextureType:MTLTextureType2DMultisample];
	[m_mtlTextureDesc setPixelFormat:pixel_format];
	[m_mtlTextureDesc setWidth:width];
	[m_mtlTextureDesc setHeight:height];
	[m_mtlTextureDesc setUsage:texture_usage];
	[m_mtlTextureDesc setStorageMode:storage_mode];
	[m_mtlTextureDesc setSampleCount:samples];
	// MTLTextureを作成
	ContextMetal* mtl_context = reinterpret_cast<ContextMetal*>(context);
	id<MTLDevice> mtl_device = mtl_context->getDevice();
	m_mtlTexture = [mtl_device newTextureWithDescriptor:m_mtlTextureDesc];
	if (m_mtlTexture == nil) {
		return false;
	}
	return true;
}

void RenderbufferMetal::getStorageInformation(GLenum* format, GLsizei* width, GLsizei* height, GLsizei* samples)
{
	AXGL_ASSERT((format != nullptr) && (width != nullptr) && (height != nullptr) && (samples != nullptr));
	if (m_layer != nil) {
		// CAMetalLayerから取得
		*format = get_internalformat([m_layer pixelFormat]);
		*width  = [m_layer drawableSize].width;
		*height = [m_layer drawableSize].height;
		*samples = 0;
	} else {
		if (m_mtlTexture == nil) {
			// MTLTextureから取得
			*format  = get_internalformat([m_mtlTexture pixelFormat]);
			*width   = (GLsizei)[m_mtlTexture width];
			*height  = (GLsizei)[m_mtlTexture height];
			*samples = (GLsizei)[m_mtlTexture sampleCount];
		} else {
			// 未確定
			*format = GL_RGBA4;
			*width = 0;
			*height = 0;
			*samples = 0;
		}
	}
	return;
}

bool RenderbufferMetal::setStorageFromLayer(ContextMetal* context, CAMetalLayer* layer)
{
	if ((context == nil) || (layer == nil)) {
		return false;
	}
	// テクスチャをリリース
	releaseTexture();
	// CAMetalLayerを保持
	m_layer = layer;
	// Drawableを取得
	m_currentDrawable = [m_layer nextDrawable];
	return true;
}

bool RenderbufferMetal::nextDrawable()
{
	bool result = false;
	if (m_layer != nil) {
		// Drawableを取得
		m_currentDrawable = [m_layer nextDrawable];
		result = (m_currentDrawable != nil);
	}
	return result;
}

id<MTLTexture> RenderbufferMetal::getMtlTexture() const
{
	return m_mtlTexture;
}

MTLPixelFormat RenderbufferMetal::getPixelFormat()
{
	MTLPixelFormat pixel_format = MTLPixelFormatInvalid;
	if (m_mtlTextureDesc != nil) {
		pixel_format = [m_mtlTextureDesc pixelFormat];
	}
	return pixel_format;
}

id<CAMetalDrawable> RenderbufferMetal::getCurrentDrawable() const
{
	return m_currentDrawable;
}

id<MTLTexture> RenderbufferMetal::getMtlTextureFromDrawable() const
{
	if (m_currentDrawable == nil) {
		return nil;
	}
	return [m_currentDrawable texture];
}

bool RenderbufferMetal::isLayerStorage() const
{
	return (m_layer != nil);
}

void RenderbufferMetal::releaseTexture()
{
	// MTLTextureをリリース
	if (m_mtlTexture != nil) {
		m_mtlTexture = nil;
	}
	return;
}

void RenderbufferMetal::releaseLayer()
{
	// Drawableをリリース
	if (m_currentDrawable != nil) {
		m_currentDrawable = nil;
	}
	// CAMetalLayerをリリース
	if (m_layer != nil) {
		m_layer = nil;
	}
	return;
}

} // namespace axgl
