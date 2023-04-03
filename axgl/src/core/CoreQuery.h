// CoreQuery.h
// Queryクラスの宣言
#ifndef __CoreQuery_h_
#define __CoreQuery_h_

#include "CoreObject.h"

namespace axgl {

class BackendQuery;

// Queryクラス
class CoreQuery : public CoreObject
{
public:
	CoreQuery();
	~CoreQuery();
	void begin(CoreContext* context, GLenum target);
	void end(CoreContext* context, GLenum target);
	void getQueryObjectuiv(GLenum pname, GLuint* params);
	bool initialize(CoreContext* context);
	virtual void terminate(CoreContext* context) override;
	BackendQuery* getBackendQuery() const
	{
		return m_pBackendQuery;
	}

private:
	enum {
		TARGET_NONE = 0
	};

private:
	GLenum m_target = TARGET_NONE;
	BackendQuery* m_pBackendQuery = nullptr;
};

} // namespace axgl

#endif // __CoreQuery_h_
