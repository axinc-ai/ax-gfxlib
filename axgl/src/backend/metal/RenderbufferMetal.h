// RenderbufferMetal.h
#ifndef __RenderbufferMetal_h_
#define __RenderbufferMetal_h_
#include "BackendMetal.h"
#include "../BackendRenderbuffer.h"
#import <QuartzCore/CAMetalLayer.h>

namespace axgl {

class ContextMetal;

class RenderbufferMetal : public BackendRenderbuffer
{
public:
	RenderbufferMetal();
	virtual ~RenderbufferMetal();
	virtual bool initialize(BackendContext* context) override;
	virtual void terminate(BackendContext* context) override;
	virtual bool createStorage(BackendContext* context, GLenum internalformat, GLsizei width, GLsizei height) override;
	virtual bool createStorageMultisample(BackendContext* context, GLsizei samples,
		GLenum internalformat, GLsizei width, GLsizei height) override;
	virtual void getStorageInformation(GLenum* format, GLsizei* width, GLsizei* height, GLsizei* samples) override;

public:
	bool setStorageFromLayer(ContextMetal* context, CAMetalLayer* layer);
	bool nextDrawable();
	id<MTLTexture> getMtlTexture() const;
	MTLPixelFormat getPixelFormat();
	id<CAMetalDrawable> getCurrentDrawable() const;
	id<MTLTexture> getMtlTextureFromDrawable() const;
	bool isLayerStorage() const;

private:
	void releaseTexture();
	void releaseLayer();

private:
	MTLTextureDescriptor* m_mtlTextureDesc = nil;
	id<MTLTexture> m_mtlTexture = nil;
	CAMetalLayer* m_layer = nil;
	id<CAMetalDrawable> m_currentDrawable = nil;
};

} // namespace axgl

#endif // __RenderbufferMetal_h_
