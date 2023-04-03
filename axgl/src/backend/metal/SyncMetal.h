// SyncMetal.h
#ifndef __SyncMetal_h_
#define __SyncMetal_h_
#include "BackendMetal.h"
#include "../BackendSync.h"

namespace axgl {

class SyncMetal : public BackendSync
{
public:
	SyncMetal();
	virtual ~SyncMetal();
	virtual bool initialize(BackendContext* context) override;
	virtual void terminate(BackendContext* context) override;
	virtual void setCondition(GLenum condition) override;
	virtual GLint getStatus() override;

public:
	GLenum getCondition() const;
	void setStatus(GLint status);

private:
	GLenum m_condition = 0;
	volatile GLint m_status = GL_UNSIGNALED;
};

} // namespace axgl

#endif // __SyncMetal_h_
