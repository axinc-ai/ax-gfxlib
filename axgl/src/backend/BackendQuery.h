// BackendQuery.h
#ifndef __BackendQuery_h_
#define __BackendQuery_h_

#include "../common/axglCommon.h"

namespace axgl {

class BackendContext;

class BackendQuery
{
public:
	virtual ~BackendQuery() {}
	virtual bool initialize(BackendContext* context) = 0;
	virtual void terminate(BackendContext* context) = 0;
	virtual bool begin(BackendContext* context) = 0;
	virtual bool end(BackendContext* context) = 0;
	virtual bool getQueryuiv(GLenum pname, GLuint* params) = 0;

public:
	static BackendQuery* create();
	static void destroy(BackendQuery* query);
};

} // namespace axgl

#endif // __BackendQuery_h_
