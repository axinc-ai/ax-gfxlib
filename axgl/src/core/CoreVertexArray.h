// CoreVertexArray.h
// VertexArrayクラスの宣言
#ifndef __CoreVertexArray_h_
#define __CoreVertexArray_h_

#include "CoreObject.h"

namespace axgl {

class CoreContext;
class CoreBuffer;
class BackendVertexArray;

// VertexArrayクラス
class CoreVertexArray : public CoreObject
{
public:
	CoreVertexArray();
	~CoreVertexArray();
	void setVertexAttribPointer(CoreContext* context, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer, CoreBuffer* buffer);
	void setVertexAttribIPointer(CoreContext* context, GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer, CoreBuffer* buffer);
	void setEnableVertexAttrib(GLuint index, GLboolean enable);
	void setVertexAttribDivisor(GLuint index, GLuint divisor);
	void setIndexBuffer(CoreContext* context, CoreBuffer* indexBuffer);
	void getVertexAttribfv(GLuint index, GLenum pname, GLfloat* params);
	void getVertexAttribIiv(GLuint index, GLenum pname, GLint* params);
	void getVertexAttribPointerv(GLuint index, GLenum pname, void** pointer);
	void getVertexAttribIuiv(GLuint index, GLenum pname, GLuint* params);
	CoreBuffer* getIndexBuffer() const;
	bool initialize(CoreContext* context);
	virtual void terminate(CoreContext* context) override;
	BackendVertexArray* getBackendVertexArray() const
	{
		return m_pBackendVertexArray;
	}

private:
	struct VertexAttrib
	{
		CoreBuffer* m_pBuffer = nullptr;
		GLboolean m_enable = GL_FALSE;
		GLint m_size = 4;
		GLenum m_type = GL_FLOAT;
		GLboolean m_normalized = GL_FALSE;
		GLsizei m_stride = 0;
		const void* m_pointer = nullptr;
		GLuint m_divisor = 0;
	};

private:
	VertexAttrib m_vertexAttribs[AXGL_MAX_VERTEX_ATTRIBS];
	BackendVertexArray* m_pBackendVertexArray = nullptr;
	CoreBuffer* m_pIndexBuffer = nullptr;
};

} // namespace axgl

#endif // __CoreVertexArray_h_
