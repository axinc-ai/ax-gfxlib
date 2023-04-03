// BackendSync.h
#ifndef __BackendSync_h_
#define __BackendSync_h_

#include "../common/axglCommon.h"

namespace axgl {

class BackendContext;

class BackendSync
{
public:
	virtual ~BackendSync() {}
	virtual bool initialize(BackendContext* context) = 0;
	virtual void terminate(BackendContext* context) = 0;
	virtual void setCondition(GLenum condition) = 0;
	virtual GLint getStatus() = 0;

public:
	static BackendSync* create();
	static void destroy(BackendSync* sync);
};

} // namespace axgl

#endif // __BackendSync_h_
