// CoreFramebuffer.h
// Framebufferクラスの宣言
#ifndef __CoreFramebuffer_h_
#define __CoreFramebuffer_h_

#include "CoreObject.h"

namespace axgl {

struct TargetAttachment;
class CoreContext;
class CoreRenderbuffer;
class CoreTexture;
class BackendFramebuffer;

// Framebufferクラス
class CoreFramebuffer : public CoreObject
{
public:
	CoreFramebuffer();
	~CoreFramebuffer();
	void setTarget(GLenum target);
	GLenum getTarget() const;
	GLenum checkStatus() const;
	void setRenderbuffer(CoreContext* context, GLenum attachment, CoreRenderbuffer* renderbuffer);
	void setTexture2d(CoreContext* context, GLenum attachment, CoreTexture* texture, GLenum textarget, GLint level);
	void setTextureLayer(CoreContext* context, GLenum attachment, CoreTexture* texture, GLint level, GLint layer);
	void invalidateFramebuffer(GLsizei numAttachments, const GLenum* attachments);
	void invalidateSubFramebuffer(GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height);
	void getAttachmentParameteriv(GLenum attachment, GLenum pname, GLint* params);
	void setupTargetAttachment(TargetAttachment* targetAttachment);
	bool initialize(CoreContext* context);
	virtual void terminate(CoreContext* context) override;
	BackendFramebuffer* getBackendFramebuffer() const
	{
		return m_pBackendFramebuffer;
	}

private:
	enum {
		TARGET_UNKNOWN = 0
	};
	struct AttachmentParams
	{
		CoreObject* m_pObject = nullptr;
		GLint m_textureLevel = 0;
		GLenum m_textureCubeMapFace = 0;
		GLint m_textureLayer = 0;
	};

private:
	static void setObjectToAttachment(CoreContext* context, AttachmentParams* attachment, CoreObject* object, GLenum textarget = 0, GLint level = 0, GLint layer = 0);
	static GLenum getFormatFromAttachmentParams(const AttachmentParams& attachment);

private:
	GLenum m_target = TARGET_UNKNOWN;
	AttachmentParams m_colorAttachment[AXGL_MAX_COLOR_ATTACHMENTS];
	AttachmentParams m_depthAttachment;
	AttachmentParams m_stencilAttachment;
	BackendFramebuffer* m_pBackendFramebuffer = nullptr;
};

} // namespace axgl

#endif // __CoreFramebuffer_h_
