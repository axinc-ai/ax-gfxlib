// CoreObject.h
// オブジェクトクラスの宣言
#ifndef __CoreObject_h_
#define __CoreObject_h_

#include "../common/axglCommon.h"

namespace axgl {

class CoreContext;

// オブジェクトクラス
class CoreObject
{
public:
	enum CoreObjectType {
		TYPE_UNKNOWN = 0,
		TYPE_BUFFER,
		TYPE_FRAMEBUFFER,
		TYPE_PROGRAM,
		TYPE_QUERY,
		TYPE_RENDERBUFFER,
		TYPE_SAMPLER,
		TYPE_SHADER,
		TYPE_SYNC,
		TYPE_TEXTURE,
		TYPE_TRANSFORM_FEEDBACK,
		TYPE_VERTEX_ARRAY
	};

public:
	CoreObject();
	virtual ~CoreObject();
	virtual void terminate(CoreContext* context);
	void setId(GLuint id)
	{
		m_id = id;
	}
	GLuint getId() const
	{
		return m_id;
	}
	CoreObjectType getObjectType() const
	{
		return m_objectType;
	}
	void addRef();
	void release(CoreContext* context);

protected:
	CoreObjectType m_objectType = TYPE_UNKNOWN;
	GLuint m_id = 0;
	uint32_t m_referenceCount = 0;
};

} // namespace axgl

#endif // __CoreObject_h_
