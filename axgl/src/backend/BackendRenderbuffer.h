// BackendRenderbuffer.h
#ifndef __BackendRenderbuffer_h_
#define __BackendRenderbuffer_h_

#include "../common/axglCommon.h"

namespace axgl {

class BackendContext;

struct BackendRenderbufferFormat
{
	GLenum internalformat;
	GLint sample_counts;
	const GLint* samples;
};

class BackendRenderbuffer
{
public:
	virtual ~BackendRenderbuffer() {}
	virtual bool initialize(BackendContext* context) = 0;
	virtual void terminate(BackendContext* context) = 0;
	virtual bool createStorage(BackendContext* context, GLenum internalformat, GLsizei width, GLsizei height) = 0;
	virtual bool createStorageMultisample(BackendContext* context, GLsizei samples,
		GLenum internalformat, GLsizei width, GLsizei height) = 0;
	virtual void getStorageInformation(GLenum* format, GLsizei* width, GLsizei* height, GLsizei* samples) = 0;

public:
	static BackendRenderbuffer* create();
	static void destroy(BackendRenderbuffer* renderbuffer);
};

} // namespace axgl

#endif // __BackendRenderbuffer_h_
