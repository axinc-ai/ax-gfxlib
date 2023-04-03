// CoreShader.h
// Shaderクラスの宣言
#ifndef __CoreShader_h_
#define __CoreShader_h_

#include "CoreObject.h"
#include "../AXGLString.h"

namespace axgl {

class BackendShader;

// Shaderクラス
class CoreShader : public CoreObject
{
public:
	CoreShader(GLenum type);
	~CoreShader();
	GLenum getType() const
	{
		return m_type;
	}
	void compile(CoreContext* context);
	void setBinary(GLenum binaryFormat, const void* binary, GLsizei length, int index);
	void setSource(GLsizei count, const GLchar* const* string, const GLint* length);
	void getShaderiv(GLenum pname, GLint* params);
	void getShaderInfoLog(GLsizei bufSize, GLsizei* length, GLchar* infoLog);
	void getShaderSource(GLsizei bufSize, GLsizei* length, GLchar* source);
	void setDeleteStatus();
	bool initialize(CoreContext* context);
	virtual void terminate(CoreContext* context) override;
	BackendShader* getBackendShader() const
	{
		return m_pBackendShader;
	}
	GLboolean getCompileStatus() const { return m_compileStatus;  }

private:
	GLenum m_type = GL_VERTEX_SHADER;
	AXGLString m_source;
	AXGLString m_infoLog;
	GLboolean m_deleteStatus = GL_FALSE;
	GLboolean m_compileStatus = GL_FALSE;
	BackendShader* m_pBackendShader = nullptr;
};

} // namespace axgl

#endif // __CoreSampler_h_
