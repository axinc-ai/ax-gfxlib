// FramebufferMetal.h
#ifndef __FramebufferMetal_h_
#define __FramebufferMetal_h_
#include "BackendMetal.h"
#include "../BackendFramebuffer.h"
#include "../../common/ClearParameters.h"

namespace axgl {

class TextureMetal;
class RenderbufferMetal;
class QueryMetal;

class FramebufferMetal : public BackendFramebuffer
{
public:
	FramebufferMetal();
	virtual ~FramebufferMetal();
	virtual bool initialize(BackendContext* context) override;
	virtual void terminate(BackendContext* context) override;
	virtual bool setRenderbuffer(BackendContext* context, GLenum attachment, BackendRenderbuffer* renderbuffer) override;
	virtual bool setTexture2d(BackendContext* context, GLenum attachment, BackendTexture* texture,
		GLenum textarget, GLint level) override;
	virtual bool setTextureLayer(BackendContext* context, GLenum attachment, BackendTexture* texture,
		GLint level, GLint layer) override;
	virtual GLenum checkStatus() const override;

public:
	bool setupRenderPassDescriptor(QueryMetal* query, GLbitfield clearBits, const ClearParameters* clearParams);
	bool setupRenderPassDescriptorForClearI(GLenum buffer, GLint drawbuffer, const GLint* value);
	bool setupRenderPassDescriptorForClearUI(GLenum buffer, GLint drawbuffer, const GLuint* value);
	bool setupRenderPassDescriptorForClearF(GLenum buffer, GLint drawbuffer, const GLfloat* value);
	bool setupRenderPassDescriptorForClearFI(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
	MTLRenderPassDescriptor* getMtlRenderPassDescriptor() const
	{
		return m_mtlRenderPassDesc;
	}
	MTLPixelFormat getColorFormat(int index) const;
	MTLPixelFormat getDepthFormat() const;
	MTLPixelFormat getStencilFormat() const;
	void getRectSize(RectSize* rectSize) const;
	id<MTLTexture> getColorTexture(int index) const;
	uint32_t getColorTextureLevel(int index) const;
	bool onlyOffscreenBufferAttached() const;

private:
	void setColorRenderbuffer(int index, RenderbufferMetal* renderbuffer);
	void setColorTexture(int index, TextureMetal* texture);
	void unsetColor(int index);
	void setColorAttachment(int index, id<MTLTexture> tex, int level, int slice, int depthPlane,
		MTLLoadAction loadAction, MTLStoreAction storeAction);
	void setDepthAttachment(id<MTLTexture> tex, int level, int slice, int depthPlane,
		MTLLoadAction loadAction, MTLStoreAction storeAction);
	void setStencilAttachment(id<MTLTexture> tex, int level, int slice, int depthPlane,
		MTLLoadAction loadAction, MTLStoreAction storeAction);

private:
	enum ObjectType {
		ObjectTypeNone = 0,
		ObjectTypeRenderbuffer = 1,
		ObjectTypeTexture = 2
	};
	struct ColorAttachInfo {
		ObjectType type;
		union {
			TextureMetal* texture;
			RenderbufferMetal* renderbuffer;
		};
	};
	MTLRenderPassDescriptor* m_mtlRenderPassDesc = nil;
	ColorAttachInfo m_colorAttachInfo[AXGL_MAX_COLOR_ATTACHMENTS];
	bool m_renderPassModified = true;
};

} // namespace axgl

#endif // __FramebufferMetal_h_


