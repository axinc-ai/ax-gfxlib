// CoreSampler.h
// Samplerクラスの宣言
#ifndef __CoreSampler_h_
#define __CoreSampler_h_

#include "CoreObject.h"
#include "../backend/BackendSampler.h"

namespace axgl {

class CoreContext;

// Samplerクラス
class CoreSampler : public CoreObject
{
public:
	CoreSampler();
	~CoreSampler();
	void terminate(CoreContext* context) override;
	void setSamplerParameteri(CoreContext* context, GLenum pname, GLint param);
	void setSamplerParameteriv(CoreContext* context, GLenum pname, const GLint* param);
	void setSamplerParameterf(CoreContext* context, GLenum pname, GLfloat param);
	void setSamplerParameterfv(CoreContext* context, GLenum pname, const GLfloat* param);
	void getSamplerParameteriv(CoreContext* context, GLenum pname, GLint* params);
	void getSamplerParameterfv(CoreContext* context, GLenum pname, GLfloat* params);
	bool setup(CoreContext* context);
	BackendSampler* getBackendSampler() const
	{
		return m_pBackendSampler;
	}

private:
	BackendSampler::SamplerParameters m_samplerParameters;
	BackendSampler* m_pBackendSampler = nullptr;
	bool m_dirty = true;
};

} // namespace axgl

#endif // __CoreSampler_h_
