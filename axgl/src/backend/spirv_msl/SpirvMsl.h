// SpirvMsl.h
// glslang,SPIRV-Crossを使用するためのクラス宣言
#ifndef __SpirvMsl_h_
#define __SpirvMsl_h_
#include "../BackendShader.h"
#include "../../common/axglCommon.h"

// NOTE: ユーザーuniform blockの後ろにdefault uniform blockを配置
#define AXGL_MSL_DEFAULT_UNIFORM_BLOCK_INDEX ((AXGL_MAX_UNIFORM_BUFFER_BINDINGS)-1)

namespace axgl {

class ShaderSpirvMsl;
class ProgramSpirvMsl;

// glslang,SPIRV-Crossを使用するためのクラス
class SpirvMsl
{
public:
	SpirvMsl();
	~SpirvMsl();
	bool initialize();
	void terminate();
	ShaderSpirvMsl* createShader(GLenum type);
	void destroyShader(ShaderSpirvMsl* shader);
	ProgramSpirvMsl* createProgram();
	void destroyProgram(ProgramSpirvMsl* program);
	const char* getDefaultUniformBlockIndexStr() const
	{
		return m_defaultUniformBlockIndexStr;
	}

private:
	char m_defaultUniformBlockIndexStr[16];
};

} // namespace axgl

#endif // __ShaderSpirvMsl_h_
