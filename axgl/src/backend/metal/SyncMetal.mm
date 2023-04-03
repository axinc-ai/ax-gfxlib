// SyncMetal.mm
#include "SyncMetal.h"
#include "../../AXGLAllocatorImpl.h"

namespace axgl {

// BackendSyncクラスの実装 --------
BackendSync* BackendSync::create()
{
	SyncMetal* sync = AXGL_NEW(SyncMetal);
	return sync;
}
	
void BackendSync::destroy(BackendSync* sync)
{
	if (sync == nullptr) {
		return;
	}
	AXGL_DELETE(sync);
	return;
}

// SyncMetalクラスの実装 --------
SyncMetal::SyncMetal()
{
}

SyncMetal::~SyncMetal()
{
}

bool SyncMetal::initialize(BackendContext* context)
{
	m_condition = 0;
	m_status = GL_UNSIGNALED;
	return true;
}

void SyncMetal::terminate(BackendContext* context)
{
	return;
}

void SyncMetal::setCondition(GLenum condition)
{
	m_condition = condition;
	return;
}

GLint SyncMetal::getStatus()
{
	return m_status;
}

GLenum SyncMetal::getCondition() const
{
	return m_condition;
}

void SyncMetal::setStatus(GLint status)
{
	m_status = status;
	return;
}

} // namespace axgl
