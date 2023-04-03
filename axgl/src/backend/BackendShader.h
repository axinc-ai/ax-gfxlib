// BackendShader.h
#ifndef __BackendShader_h_
#define __BackendShader_h_

#include "../common/axglCommon.h"

namespace axgl {

class  BackendContext;

class BackendShader
{
public:
	virtual ~BackendShader() {}
	virtual bool initialize(BackendContext* context, GLenum type) = 0;
	virtual void terminate(BackendContext* context) = 0;
	virtual bool compileSource(BackendContext* context, const char* source) = 0;

public:
	static BackendShader* create();
	static void destroy(BackendShader* shader);
};

} // namespace axgl

#endif // __BackendShader_h_
