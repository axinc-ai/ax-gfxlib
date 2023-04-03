// BackendFramebuffer.h
#ifndef __BackendFramebuffer_h_
#define __BackendFramebuffer_h_

#include "../common/axglCommon.h"

namespace axgl {

class BackendContext;
class BackendRenderbuffer;
class BackendTexture;

class BackendFramebuffer
{
public:
	virtual ~BackendFramebuffer() {}
	virtual bool initialize(BackendContext* context) = 0;
	virtual void terminate(BackendContext* context) = 0;
	virtual bool setRenderbuffer(BackendContext* context, GLenum attachment, BackendRenderbuffer* renderbuffer) = 0;
	virtual bool setTexture2d(BackendContext* context, GLenum attachment, BackendTexture* texture,
		GLenum textarget, GLint level) = 0;
	virtual bool setTextureLayer(BackendContext* context, GLenum attachment, BackendTexture* texture,
		GLint level, GLint layer) = 0;
	virtual GLenum checkStatus() const = 0;

public:
	static BackendFramebuffer* create();
	static void destroy(BackendFramebuffer* framebuffer);
};

} // namespace axgl

#endif // __BackendFramebuffer_h_
