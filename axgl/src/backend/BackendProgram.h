// BackendProgram.h
#ifndef __BackendProgram_h_
#define __BackendProgram_h_

#include "../common/axglCommon.h"
#include "../AXGLString.h"

namespace axgl {

class BackendContext;
class BackendShader;

class BackendProgram
{
public:
	struct ShaderVariable {
		GLenum  m_Type = GL_FLOAT;
		GLsizei m_Size = 1;
		int32_t m_NativeIndex = -1; // default: invalid index
		GLboolean m_LayoutSpecified = GL_FALSE;
	};
	struct ShaderUniform {
		GLenum     m_Type = 0; // default: invalid type
		GLsizei    m_Size = 0;
		GLint      m_BlockIndex = 0;
		GLsizei    m_Offset = 0;
		GLsizei    m_ArrayStride = 0;
		GLsizei    m_MatrixStride = 0;
		GLboolean  m_IsRowMajor = GL_FALSE;
		GLboolean  m_IsDefaultBlock = GL_FALSE;
		GLboolean  m_IsUniformBlockMember = GL_FALSE;
		GLboolean  m_IsSampler = GL_FALSE;
		int32_t    m_NativeIndex = -1; // default: invalid index
	};
	struct ShaderUniformBlock {
		GLsizei m_DataSize = 0;
		GLint   m_ActiveUniforms = 0;
		int32_t m_NativeIndex = -1; // default: invalid index
	};

public:
	virtual ~BackendProgram() {}
	virtual bool initialize(BackendContext* context) = 0;
	virtual void terminate(BackendContext* context) = 0;
	virtual bool link(BackendContext* context, BackendShader* vs, BackendShader* fs) = 0;
	virtual int32_t getNumActiveAttribs() const = 0;
	virtual int32_t getNumActiveUniforms() const = 0;
	virtual int32_t getNumActiveUniformBlocks() const = 0;
	virtual int32_t getNumFragData() const = 0;
	virtual const char* getActiveVertexAttribName(int32_t index) const = 0;
	virtual const char* getActiveUniformName(int32_t index) const = 0;
	virtual const char* getActiveUniformBlockName(int32_t index) const = 0;
	virtual const char* getFragDataName(int32_t index) const = 0;
	virtual int32_t getActiveVertexAttribIndex(int32_t index) const = 0;
	virtual bool getActiveVertexAttrib(int32_t index, ShaderVariable* attrib) const = 0;
	virtual bool getActiveUniform(int32_t index, ShaderUniform* uniform) const = 0;
	virtual bool getActiveUniformBlock(BackendContext* context, int32_t index, ShaderUniformBlock* uniformBlock) const = 0;
	virtual uint32_t getUniformBlockBinding(int32_t index) const = 0;
	virtual int32_t getFragDataLocation(int32_t index) const = 0;
	virtual bool setUniformf(GLenum type, int32_t index, int32_t num, const float* value) = 0;
	virtual bool setUniformi(GLenum type, int32_t index, int32_t num, const int32_t* value) = 0;
	virtual bool setUniformui(GLenum type, int32_t index, int32_t num, const uint32_t* value) = 0;
	virtual bool setUniformSampler(int32_t index, int32_t value) = 0;
	virtual bool setUniformBlockBinding(int32_t index, uint32_t binding) = 0;
	virtual void setAttribLocation(int32_t index, int32_t location) = 0;
	
public:
	static BackendProgram* create();
	static void destroy(BackendProgram* program);
};

} // namespace axgl

#endif // __BackendProgram_h_
