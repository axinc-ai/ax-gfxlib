// CoreBuffer.h
// Bufferクラスの宣言
#ifndef __CoreBuffer_h_
#define __CoreBuffer_h_

#include "CoreObject.h"

namespace axgl {

class CoreContext;
class BackendBuffer;

// Bufferクラス
class CoreBuffer : public CoreObject
{
public:
	CoreBuffer();
	~CoreBuffer();
	void setTarget(GLenum target);
	GLenum getTarget() const;
	void setData(CoreContext* context, GLsizeiptr size, const void* data, GLenum usage);
	void setSubData(CoreContext* context, GLintptr offset, GLsizeiptr size, const void* data);
	void* mapBufferRange(CoreContext* context, GLintptr offset, GLsizeiptr size, GLbitfield access);
	GLboolean unmapBuffer(CoreContext* context);
	void getBufferPointerv(CoreContext* context, GLenum pname, void** params);
	void flushMappedBufferRange(CoreContext* context, GLintptr offset, GLsizeiptr length);
	void copyBufferSubData(CoreContext* context, CoreBuffer* src, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
	void getBufferParameteriv(CoreContext* context, GLenum pname, GLint* params);
	void getBufferParameteri64v(CoreContext* context, GLenum pname, GLint64* params);
	bool initialize(CoreContext* context);
	virtual void terminate(CoreContext* context) override;
	BackendBuffer* getBackendBuffer() const
	{
		return m_pBackendBuffer;
	}

private:
	enum {
		TARGET_UNKNOWN = 0
	};

private:
	GLenum m_target = TARGET_UNKNOWN;
	bool m_mapped = false;
	void* m_pMapPointer = nullptr;
	GLsizeiptr m_size = 0;
	GLenum m_usage = GL_STATIC_DRAW;
	GLintptr m_mapOffset = 0;
	GLsizeiptr m_mapLength = 0;
	GLbitfield m_accessFlags = 0;
	BackendBuffer* m_pBackendBuffer = nullptr;
};

} // namespace axgl

#endif // __CoreBuffer_h_
