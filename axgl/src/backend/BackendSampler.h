// BackendSampler.h
#ifndef __BackendSampler_h_
#define __BackendSampler_h_

#include "../common/axglCommon.h"

namespace axgl {

class BackendContext;

class BackendSampler
{
public:
	struct SamplerParameters {
		GLint magFilter = GL_LINEAR;
		GLint minFilter = GL_NEAREST_MIPMAP_LINEAR;
		GLint wrapS = GL_REPEAT;
		GLint wrapT = GL_REPEAT;
		GLint wrapR = GL_REPEAT;
		GLint compareMode = GL_NONE;
		GLint compareFunc = GL_LEQUAL;
		GLfloat minLod = -1000.0f;
		GLfloat maxLod = 1000.0f;
	};

public:
	virtual ~BackendSampler() {}
	virtual bool initialize(BackendContext* context) = 0;
	virtual bool terminate(BackendContext* context) = 0;
	virtual bool setupSampler(BackendContext* context, const SamplerParameters& params) = 0;

public:
	static BackendSampler* create();
	static void destroy(BackendSampler* sampler);
};

} // namespace axgl

#endif // __BackendSampler_h_
