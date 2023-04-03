// CoreRenderbuffer.h
// Renderbufferクラスの宣言
#ifndef __CoreRenderbuffer_h_
#define __CoreRenderbuffer_h_

#include "CoreObject.h"

namespace axgl { 

class CoreContext;
class BackendRenderbuffer;

// Renderbufferクラス
class CoreRenderbuffer : public CoreObject
{
public:
	CoreRenderbuffer();
	~CoreRenderbuffer();
	void createStorage(CoreContext* context, GLenum internalformat, GLsizei width, GLsizei height);
	void createStorageMultisample(CoreContext* context, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
	void updateStorageInformationFromBackend();
	GLenum getInternalformat() const
	{
		return m_internalformat;
	}
	GLsizei getWidth() const
	{
		return m_width;
	}
	GLsizei getHeight() const
	{
		return m_height;
	}
	GLsizei getSamples() const
	{
		return m_samples;
	}
	void getParameteriv(GLenum pname, GLint *params);
	bool initialize(CoreContext* context);
	virtual void terminate(CoreContext* context) override;
	BackendRenderbuffer* getBackendRenderbuffer() const
	{
		return m_pBackendRenderbuffer;
	}

private:
	GLenum m_internalformat = GL_RGBA8; // ### dummy initial value
	GLsizei m_width = 0;
	GLsizei m_height = 0;
	GLsizei m_samples = 0;
	BackendRenderbuffer* m_pBackendRenderbuffer = nullptr;
};

} // namespace axgl

#endif // __CoreRenderbuffer_h_
