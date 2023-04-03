// CoreProgram.h
// Programクラスの宣言
#ifndef __CoreProgram_h_
#define __CoreProgram_h_

#include "CoreObject.h"
#include "../AXGLString.h"
#include "../backend/BackendProgram.h"
#include "../common/axglCommon.h"

namespace axgl {

class CoreShader;
class BackendProgram;

// Programクラス
class CoreProgram : public CoreObject
{
public:
	CoreProgram();
	~CoreProgram();
	void attachShader(CoreShader* shader);
	void detachShader(CoreContext* context, CoreShader* shader);
	void getAttachedShaders(GLsizei maxCount, GLsizei* count, GLuint* shaders);
	void bindAttribLocation(GLuint index, const GLchar* name);
	GLint getAttribLocation(const GLchar* name);
	GLint getUniformLocation(const GLchar* name);
	GLint getFragDataLocation(const GLchar* name);
	void getActiveAttrib(GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
	void getActiveUniform(GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
	void getUniformIndices(GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices);
	void getActiveUniformsiv(GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params);
	GLuint getUniformBlockIndex(const GLchar* uniformBlockName);
	void getActiveUniformBlockiv(GLuint uniformBlockIndex, GLenum pname, GLint* params);
	void getActiveUniformBlockName(GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName);
	void link(CoreContext* context);
	void setUniform1f(GLint location, GLfloat v0);
	void setUniform1fv(GLint location, GLsizei count, const GLfloat* value);
	void setUniform1i(GLint location, GLint v0);
	void setUniform1iv(GLint location, GLsizei count, const GLint* value);
	void setUniform2f(GLint location, GLfloat v0, GLfloat v1);
	void setUniform2fv(GLint location, GLsizei count, const GLfloat* value);
	void setUniform2i(GLint location, GLint v0, GLint v1);
	void setUniform2iv(GLint location, GLsizei count, const GLint* value);
	void setUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
	void setUniform3fv(GLint location, GLsizei count, const GLfloat* value);
	void setUniform3i(GLint location, GLint v0, GLint v1, GLint v2);
	void setUniform3iv(GLint location, GLsizei count, const GLint* value);
	void setUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
	void setUniform4fv(GLint location, GLsizei count, const GLfloat* value);
	void setUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
	void setUniform4iv(GLint location, GLsizei count, const GLint* value);
	void setUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	void setUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	void setUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	void setUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	void setUniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	void setUniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	void setUniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	void setUniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	void setUniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	void setUniform1ui(GLint location, GLuint v0);
	void setUniform1uiv(GLint location, GLsizei count, const GLuint* value);
	void setUniform2ui(GLint location, GLuint v0, GLuint v1);
	void setUniform2uiv(GLint location, GLsizei count, const GLuint* value);
	void setUniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2);
	void setUniform3uiv(GLint location, GLsizei count, const GLuint* value);
	void setUniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
	void setUniform4uiv(GLint location, GLsizei count, const GLuint* value);
	void getUniformfv(GLint location, GLfloat* params);
	void getUniformiv(GLint location, GLint* params);
	void getUniformuiv(GLint location, GLuint* params);
	void validateProgram();
	void setTransformFeedbackVaryings(GLsizei count, const GLchar* const* varyings, GLenum bufferMode);
	void getTransformFeedbackVarying(GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name);
	void uniformBlockBinding(GLuint uniformBlockIndex, GLuint uniformBlockBinding);
	void getProgramBinary(GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, void* binary);
	void setProgramBinary(GLenum binaryFormat, const void* binary, GLsizei length);
	void setProgramParameteri(GLenum pname, GLint value);
	void getProgramiv(GLenum pname, GLint* params);
	void getProgramInfoLog(GLsizei bufSize, GLsizei* length, GLchar* infoLog);
	void setDeleteStatus();
	bool initialize(CoreContext* context);
	virtual void terminate(CoreContext* context) override;
	BackendProgram* getBackendProgram() const
	{
		return m_pBackendProgram;
	}

private:
	struct ProgramVariable {
		AXGLString name;
		BackendProgram::ShaderVariable info;
	};
	struct ProgramUniform {
		AXGLString name;
		BackendProgram::ShaderUniform info;
	};
	struct ProgramUniformBlock {
		AXGLString name;
		BackendProgram::ShaderUniformBlock info;
	};
	typedef std::vector<ProgramVariable, AXGLStlAllocator<ProgramVariable>> AXGLShaderVariables;
	typedef std::vector<ProgramUniform, AXGLStlAllocator<ProgramUniform>> AXGLShaderUniforms;
	typedef std::vector<ProgramUniformBlock, AXGLStlAllocator<ProgramUniformBlock>> AXGLUniformBlocks;

private:
	static void getShaderUniformParam(const ProgramUniform& us, GLenum pname, GLint* param);

private:
	CoreShader* m_pVertexShader = nullptr;
	CoreShader* m_pFragmentShader = nullptr;
	AXGLUnorderedMap<AXGLString, GLuint> m_bindAttribMap;
	AXGLVector<AXGLString> m_transformFeedbackVaryings;
	GLenum m_transformFeedbackBufferMode = GL_INTERLEAVED_ATTRIBS;
	AXGLShaderVariables m_vertexShaderInputs;
	AXGLShaderUniforms m_uniforms;
	AXGLShaderVariables m_transformFeedbackOutputs;
	AXGLUniformBlocks m_uniformBlocks;
	AXGLUnorderedMap<AXGLString, GLint> m_attribLocations;
	AXGLUnorderedMap<AXGLString, GLint> m_uniformLocations;
	AXGLUnorderedMap<AXGLString, GLint> m_fragDataLocations;
	AXGLUnorderedMap<AXGLString, GLint> m_uniformBlockIndices;
	AXGLString m_infoLog;
	GLboolean m_programBinaryRetrievableHint = GL_FALSE;
	GLboolean m_deleteStatus = GL_FALSE;
	GLboolean m_linkStatus = GL_FALSE;
	GLboolean m_validateStatus = GL_FALSE;
	BackendProgram* m_pBackendProgram = nullptr;
};

} // namespace axgl

#endif // __CoreProgram_h_
