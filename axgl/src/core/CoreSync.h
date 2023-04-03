// CoreSync.h
// Syncクラスの宣言
#ifndef __CoreSync_h_
#define __CoreSync_h_

#include "CoreObject.h"

namespace axgl {

class BackendSync;

// Syncクラス
class CoreSync : public CoreObject
{
public:
	CoreSync();
	CoreSync(GLenum condition, GLbitfield flags);
	~CoreSync();
	void getSynciv(GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values);
	bool initialize(CoreContext* context);
	virtual void terminate(CoreContext* context) override;
	BackendSync* getBackendSync() const
	{
		return m_pBackendSync;
	}

private:
	GLenum m_condition = GL_SYNC_GPU_COMMANDS_COMPLETE;
	GLbitfield m_flags = 0;
	BackendSync* m_pBackendSync = nullptr;
};

} // namespace axgl

#endif // __CoreSync_h_
