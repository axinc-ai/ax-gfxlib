// ShaderSpirvMsl.h
// SPIRV-Crossを使用したShaderクラスの宣言
#ifndef __ShaderSpirvMsl_h_
#define __ShaderSpirvMsl_h_
#include "../../common/axglCommon.h"
#include "../../AXGLString.h"

namespace glslang {
class TShader;
} // namespace glslang

namespace axgl {

class SpirvMsl;

// SPIRV-Crossを使用したShaderクラス
class ShaderSpirvMsl
{
public:
	ShaderSpirvMsl();
	~ShaderSpirvMsl();
	bool initialize(SpirvMsl* context, GLenum type);
	void terminate(SpirvMsl* context);
	bool compileSource(SpirvMsl* context, const char* source);
	static const char* getDefaultBlockName();
	static const char* getDefaultBlockInputName();
	static const char* getDefaultFragColorName();
	static const char* getDefaultFragDataName(int32_t index);
	bool isCompiled() const { return m_isCompiled;  }
	int32_t getLanguageType() const { return m_lang; }
	glslang::TShader* getGlslangShader() const { return m_pShader; }	

private:
	static void replaceGlslOutput(AXGLString* glsl, const char* glslName, const char* outName, const char* outDefinition);
	static void replaceGlslTextureFunction(AXGLString* glsl, const char* glslName, const char* funcName);
	
private:
	glslang::TShader* m_pShader = nullptr;
	int32_t m_lang = 0;
	bool m_isCompiled = false;
};

} // namespace axgl

#endif // __ShaderSpirvCross_h_
