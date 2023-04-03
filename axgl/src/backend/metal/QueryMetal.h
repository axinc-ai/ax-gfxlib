// QueryMetal.h
#ifndef __QueryMetal_h_
#define __QueryMetal_h_

#include "BackendMetal.h"
#include "../BackendQuery.h"

namespace axgl {

class QueryMetal : public BackendQuery
{
public:
	QueryMetal();
	virtual ~QueryMetal();
	virtual bool initialize(BackendContext* context) override;
	virtual void terminate(BackendContext* context) override;
	virtual bool begin(BackendContext* context) override;
	virtual bool end(BackendContext* context) override;
	virtual bool getQueryuiv(GLenum pname, GLuint* params) override;

public:
	id<MTLBuffer> getVisibilityResultBuffer() const;
	uint64_t getVisibilityResultBufferOffset() const;
	bool incrementOffset();
	id<MTLBuffer> getCopyDestBuffer() const;

private:
	id<MTLBuffer> m_visibilityResultBuffer = nil;
	id<MTLBuffer> m_copyDestBuffer = nil;
	volatile GLuint m_resultAvailable = GL_TRUE;
	uint64_t m_visibilityResultBufferOffset = 0;
};

} // namespace axgl

#endif // __QueryMetal_h_
