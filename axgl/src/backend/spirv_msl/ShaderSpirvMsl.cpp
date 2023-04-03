// ShaderSpirvMsl.cpp
// SPIRV-Crossを使用したShaderクラスの実装
#include "ShaderSpirvMsl.h"
#include "SpirvMsl.h"
#include "glslang/Public/ShaderLang.h"
#include "glslang/Public/ResourceLimits.h"
#include "SPIRV/GlslangToSpv.h"
#include "../../AXGLAllocatorImpl.h"

#if 0
#define AXGL_SHADER_MSL_DBGOUT(...)
#else
#define AXGL_SHADER_MSL_DBGOUT AXGL_DBGOUT
#endif

static constexpr char c_defaultBlockName[] = {
	"AXGL_Default"
};
static constexpr char c_defaultBlockInputName[] = {
	"AXGL_Default& "
};
static constexpr char c_defaultFragColorName[] = {
	"axgl_FragColor"
};
static constexpr char c_defaultFragData0Name[] = {
	"axgl_FragData0"
};
static constexpr char c_defaultFragData1Name[] = {
	"axgl_FragData1"
};
static constexpr char c_defaultFragData2Name[] = {
	"axgl_FragData2"
};
static constexpr char c_defaultFragData3Name[] = {
	"axgl_FragData3"
};

static const TBuiltInResource* getDefaultBuiltInResources()
{
	static const TBuiltInResource c_defaultResources = {
		32, // MaxLights
		6, // MaxClipPlanes
		32, // MaxTextureUnits
		32, // MaxTextureCoords
		64, // MaxVertexAttribs
		4096, // MaxVertexUniformComponents
		64, // MaxVaryingFloats
		32, // MaxVertexTextureImageUnits
		80, // MaxCombinedTextureImageUnits
		32, // MaxTextureImageUnits
		4096, // MaxFragmentUniformComponents
		32, // MaxDrawBuffers
		128, // MaxVertexUniformVectors
		8, // MaxVaryingVectors
		16, // MaxFragmentUniformVectors
		16, // MaxVertexOutputVectors
		15, // MaxFragmentInputVectors
		-8, // MinProgramTexelOffset
		7, // MaxProgramTexelOffset
		8, // MaxClipDistances
		65535, // MaxComputeWorkGroupCountX
		65535, // MaxComputeWorkGroupCountY
		65535, // MaxComputeWorkGroupCountZ
		1024, // MaxComputeWorkGroupSizeX
		1024, // MaxComputeWorkGroupSizeY
		64, // MaxComputeWorkGroupSizeZ
		1024, // MaxComputeUniformComponents
		16, // MaxComputeTextureImageUnits
		8, // MaxComputeImageUniforms
		8, // MaxComputeAtomicCounters
		1, // MaxComputeAtomicCounterBuffers
		60, // MaxVaryingComponents
		64, // MaxVertexOutputComponents
		64, // MaxGeometryInputComponents
		128, // MaxGeometryOutputComponents
		128, // MaxFragmentInputComponents
		8, // MaxImageUnits
		8, // MaxCombinedImageUnitsAndFragmentOutputs
		8, // MaxCombinedShaderOutputResources
		0, // MaxImageSamples
		0, // MaxVertexImageUniforms
		0, // MaxTessControlImageUniforms
		0, // MaxTessEvaluationImageUniforms
		0, // MaxGeometryImageUniforms
		8, // MaxFragmentImageUniforms
		8, // MaxCombinedImageUniforms
		16, // MaxGeometryTextureImageUnits
		256, // MaxGeometryOutputVertices
		1024, // MaxGeometryTotalOutputComponents
		1024, // MaxGeometryUniformComponents
		64, // MaxGeometryVaryingComponents
		128, // MaxTessControlInputComponents
		128, // MaxTessControlOutputComponents
		16, // MaxTessControlTextureImageUnits
		1024, // MaxTessControlUniformComponents
		4096, // MaxTessControlTotalOutputComponents
		128, // MaxTessEvaluationInputComponents
		128, // MaxTessEvaluationOutputComponents
		16, // MaxTessEvaluationTextureImageUnits
		1024, // MaxTessEvaluationUniformComponents
		120, // MaxTessPatchComponents
		32, // MaxPatchVertices
		64, // MaxTessGenLevel
		16, // MaxViewports
		0, // MaxVertexAtomicCounters
		0, // MaxTessControlAtomicCounters
		0, // MaxTessEvaluationAtomicCounters
		0, // MaxGeometryAtomicCounters
		8, // MaxFragmentAtomicCounters
		8, // MaxCombinedAtomicCounters
		1, // MaxAtomicCounterBindings
		0, // MaxVertexAtomicCounterBuffers
		0, // MaxTessControlAtomicCounterBuffers
		0, // MaxTessEvaluationAtomicCounterBuffers
		0, // MaxGeometryAtomicCounterBuffers
		1, // MaxFragmentAtomicCounterBuffers
		1, // MaxCombinedAtomicCounterBuffers
		16384, // MaxAtomicCounterBufferSize
		4, // MaxTransformFeedbackBuffers
		64, // MaxTransformFeedbackInterleavedComponents
		8, // MaxCullDistances
		8, // MaxCombinedClipAndCullDistances
		4, // MaxSamples
		256, // maxMeshOutputVerticesNV
		512, // maxMeshOutputPrimitivesNV
		32, // maxMeshWorkGroupSizeX_NV
		1, // maxMeshWorkGroupSizeY_NV
		1, // maxMeshWorkGroupSizeZ_NV
		32, // maxTaskWorkGroupSizeX_NV
		1, // maxTaskWorkGroupSizeY_NV
		1, // maxTaskWorkGroupSizeZ_NV
		4, // maxMeshViewCountNV
		256, // maxMeshOutputVerticesEXT
		256, // maxMeshOutputPrimitivesEXT
		128, // maxMeshWorkGroupSizeX_EXT
		128, // maxMeshWorkGroupSizeY_EXT
		128, // maxMeshWorkGroupSizeZ_EXT
		128, // maxTaskWorkGroupSizeX_EXT
		128, // maxTaskWorkGroupSizeY_EXT
		128, // maxTaskWorkGroupSizeZ_EXT
		4, // maxMeshViewCountEXT
		1, // maxDualSourceDrawBuffersEXT
		{ // limits
			1, // nonInductiveForLoops
			1, // whileLoops
			1, // doWhileLoops
			1, // generalUniformIndexing
			1, // generalAttributeMatrixVectorIndexing
			1, // generalVaryingIndexing
			1, // generalSamplerIndexing
			1, // generalVariableIndexing
			1 // generalConstantMatrixVectorIndexing
		}
	};
	return &c_defaultResources;
}

static EShLanguage get_glslang_shader_type(GLenum type)
{
	EShLanguage glslang_type = EShLangVertex;
	switch (type) {
	case GL_FRAGMENT_SHADER:
		glslang_type = EShLangFragment;
		break;
	case GL_VERTEX_SHADER:
		break;
	default:
		AXGL_ASSERT(0);
		break;
	}
	return glslang_type;
}

namespace axgl {

ShaderSpirvMsl::ShaderSpirvMsl()
{
}

ShaderSpirvMsl::~ShaderSpirvMsl()
{
}

bool ShaderSpirvMsl::initialize(SpirvMsl* context, GLenum type)
{
	AXGL_UNUSED(context);
	if (m_pShader != nullptr) {
		return true;
	}
	bool result = false;
	EShLanguage glslang_lang = get_glslang_shader_type(type);
	m_pShader = AXGL_NEW(glslang::TShader(glslang_lang));
	if (m_pShader != nullptr) {
		m_lang = static_cast<int32_t>(glslang_lang);
		result = true;
	}
	m_isCompiled = false;
	return result;
}

void ShaderSpirvMsl::terminate(SpirvMsl* context)
{
	AXGL_UNUSED(context);
	if (m_pShader == nullptr) {
		return;
	}
	AXGL_DELETE(m_pShader);
	m_pShader = nullptr;
	m_isCompiled = false;
	return;
}

bool ShaderSpirvMsl::compileSource(SpirvMsl* context, const char* source)
{
	AXGL_UNUSED(context);
	if (source == nullptr) {
		return false;
	}
	const TBuiltInResource* default_resources = getDefaultBuiltInResources();
	const char* src_ptr[1] = { source };
	static const int input_version = 100;
	AXGLString glsl_src = source;
	// version
	{
		AXGLString::size_type version_pos = glsl_src.find("#version");
		if (version_pos != AXGLString::npos) {
			AXGLString::size_type end_directive = glsl_src.find("\n", version_pos);
			if (end_directive != AXGLString::npos) {
				glsl_src.replace(version_pos, end_directive - version_pos, "#version 450");
			}
		} else {
			glsl_src.insert(0, "#version 450\n");
		}
	}
	
	EShLanguage lang = m_pShader->getStage();
	if (lang == EShLangVertex) {
		// attribute,varyingの置き換え
		AXGLString::size_type pos = glsl_src.find("attribute");
		while (pos != AXGLString::npos) {
			glsl_src.replace(pos, 9, "in");
			pos = glsl_src.find("attribute", pos + 2);
		}
		pos = glsl_src.find("varying");
		while (pos != AXGLString::npos) {
			glsl_src.replace(pos, 7, "out");
			pos = glsl_src.find("varying", pos + 3);
		}
	} else if (lang == EShLangFragment) {
		// varyingの置き換え
		AXGLString::size_type pos = glsl_src.find("varying");
		while (pos != AXGLString::npos) {
			glsl_src.replace(pos, 7, "in");
			pos = glsl_src.find("varying", pos + 2);
		}
		// Fragment出力の置き換え
		replaceGlslOutput(&glsl_src, "gl_FragColor", c_defaultFragColorName, "layout(location = 0) out vec4 axgl_FragColor;\n");
		replaceGlslOutput(&glsl_src, "gl_FragData[0]", c_defaultFragData0Name, "layout(location = 0) out vec4 axgl_FragData0;\n");
		replaceGlslOutput(&glsl_src, "gl_FragData[1]", c_defaultFragData1Name, "layout(location = 1) out vec4 axgl_FragData1;\n");
		replaceGlslOutput(&glsl_src, "gl_FragData[2]", c_defaultFragData2Name, "layout(location = 2) out vec4 axgl_FragData2;\n");
		replaceGlslOutput(&glsl_src, "gl_FragData[3]", c_defaultFragData3Name, "layout(location = 3) out vec4 axgl_FragData3;\n");
	}
	// texture functionの置き換え
	{
		replaceGlslTextureFunction(&glsl_src, "texture2D(", "texture(");
		replaceGlslTextureFunction(&glsl_src, "textureCube(", "texture(");
		replaceGlslTextureFunction(&glsl_src, "texture2DProj(", "textureProj(");
		replaceGlslTextureFunction(&glsl_src, "texture2DLod(", "textureLod(");
		replaceGlslTextureFunction(&glsl_src, "textureCubeLod(", "textureLod(");
		replaceGlslTextureFunction(&glsl_src, "texture2DProjLod(", "textureProjLod(");
	}
	AXGL_SHADER_MSL_DBGOUT("GLSL =========\n%s\n", glsl_src.c_str());
	src_ptr[0] = glsl_src.c_str();
	// GL_EXT_vulkan_glsl_relaxed
	m_pShader->setEnvInputVulkanRulesRelaxed();
	m_pShader->setGlobalUniformBlockName(c_defaultBlockName);
	m_pShader->setGlobalUniformSet(0);
	m_pShader->setGlobalUniformBinding(AXGL_MSL_DEFAULT_UNIFORM_BLOCK_INDEX);
	// set auto binding/locations
	m_pShader->setAutoMapBindings(true);
	m_pShader->setAutoMapLocations(true);
	m_pShader->setStrings(src_ptr, 1);
	m_pShader->setEnvInput(glslang::EShSourceGlsl, static_cast<EShLanguage>(m_lang), glslang::EShClientVulkan, input_version);
	m_pShader->setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_0);
	m_pShader->setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);
	m_pShader->setEntryPoint("main");
	EShMessages messages = EShMsgDefault;
	bool parse_result = m_pShader->parse(default_resources, 100, false, messages);
	if (!parse_result) {
		AXGL_DBGOUT("BackendShaderSpirvCross::compileSource> TShader::parse FAILED\n");
		AXGL_DBGOUT("InfoLog:%s\n", m_pShader->getInfoLog());
	}
	m_isCompiled = parse_result;
	return parse_result;
}

const char* ShaderSpirvMsl::getDefaultBlockName()
{
	return c_defaultBlockName;
}

const char* ShaderSpirvMsl::getDefaultBlockInputName()
{
	return c_defaultBlockInputName;
}

const char* ShaderSpirvMsl::getDefaultFragColorName()
{
	return c_defaultFragColorName;
}

const char* ShaderSpirvMsl::getDefaultFragDataName(int32_t index)
{
	const char* name = "";
	switch (index) {
	case 0:
		name = c_defaultFragData0Name;
		break;
	case 1:
		name = c_defaultFragData1Name;
		break;
	case 2:
		name = c_defaultFragData2Name;
		break;
	case 3:
		name = c_defaultFragData3Name;
		break;
	default:
		break;
	}
	return name;
}

void ShaderSpirvMsl::replaceGlslOutput(AXGLString* glsl, const char* glslName, const char* outName, const char* outDefinition)
{
	AXGL_ASSERT((glsl != nullptr) && (glslName != nullptr) && (outName != nullptr) && (outDefinition != nullptr));
	AXGLString::size_type pos = glsl->find(glslName);
	if (pos != AXGLString::npos) {
		const size_t len_glsl_name = strlen(glslName);
		const size_t len_out_name = strlen(outName);
		while (pos != AXGLString::npos) {
			glsl->replace(pos, len_glsl_name, outName);
			pos = glsl->find(glslName, pos + len_out_name);
		}
		pos = glsl->find("void main(");
		if (pos != AXGLString::npos) {
			glsl->insert(pos, outDefinition);
		}
	}
	return;
}

void ShaderSpirvMsl::replaceGlslTextureFunction(AXGLString* glsl, const char* glslName, const char* funcName)
{
	AXGL_ASSERT((glsl != nullptr) && (glslName != nullptr) && (funcName != nullptr));
	AXGLString::size_type pos = glsl->find(glslName);
	if (pos != AXGLString::npos) {
		size_t len_glsl_name = strlen(glslName);
		size_t len_func_name = strlen(funcName);
		while (pos != AXGLString::npos) {
			glsl->replace(pos, len_glsl_name, funcName);
			pos = glsl->find(glslName, pos + len_func_name);
		}
	}
	return;
}

} // namespace axgl
