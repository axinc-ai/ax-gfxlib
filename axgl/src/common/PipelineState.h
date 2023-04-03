// PipelineState.h
#ifndef __PipelineState_h_
#define __PipelineState_h_

#include "axglCommon.h"

namespace axgl {

class BackendProgram;
class BackendVertexArray;

#pragma pack(push, 1)
// ブレンドパラメータ
struct BlendParams
{
	// GL_BLEND
	GLboolean blendEnable = GL_FALSE;
	// GL_BLEND_COLOR
	float blendColor[4] = {0.0f,0.0f,0.0f,0.0f};
	// GL_BLEND_EQUATION_*
	GLenum blendEquation[2] = {GL_FUNC_ADD,GL_FUNC_ADD};
	// GL_BLEND_SRC_*
	GLenum blendSrc[2] = {GL_ONE,GL_ONE};
	// GL_BLEND_DST_*
	GLenum blendDst[2] = {GL_ZERO,GL_ZERO};
};

// カラー書き込みマスクパラメータ
struct ColorWritemaskParams
{
	// GL_COLOR_WRITEMASK
	GLboolean colorWritemask[4] = {GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE};
};

// 頂点属性パラメータ
struct VertexAttrib
{
	GLboolean enable = GL_FALSE;
	GLint size = 4;
	GLenum type = GL_FLOAT;
	GLboolean normalized  = GL_FALSE;
	GLsizei stride = 0;
	float currentValue[4] = {0.0f,0.0f,0.0f,1.0f};
	const void* pointer = nullptr;
	GLuint divisor = 0;
};

// ターゲットアタッチメントパラメータ
struct TargetAttachment
{
	GLenum colorFormat[AXGL_MAX_COLOR_ATTACHMENTS] = {0};
	GLenum depthFormat = 0;
	GLenum stencilFormat = 0;
	GLint samples = 0;
};

// サンプルカバレッジパラメータ
struct SampleCoverageParams
{
	// GL_SAMPLE_ALPHA_TO_COVERAGE
	GLboolean sampleAlphaToCoverageEnable = GL_FALSE;
	// GL_SAMPLE_COVERAGE
	GLboolean sampleCoverageEnable = GL_FALSE;
	// GL_SAMPLE_COVERAGE_VALUE
	float sampleCoverageValue = 1.0f;
	// GL_SAMPLE_COVERAGE_INVERT
	GLboolean sampleCoverageInvert = GL_FALSE;
};
#pragma pack(pop)

// PipelineStateクラス
class PipelineState
{
public:
	PipelineState();
	~PipelineState();
	// operator == for unordered_map
	bool operator==(const PipelineState& rhs) const;
	// hash function struct for unordered_map
	struct Hash
	{
		size_t operator()(const PipelineState& state) const;
	};

public:
	// specifying graphics functions
	BackendProgram* program = nullptr;
	// specifying buffer layouts and fetch behavior
	VertexAttrib vertexAttribs[AXGL_MAX_VERTEX_ATTRIBS];
	// specifying buffer mutability
	// NOTE: Immutable only
	// specifying rendering pipeline state
	TargetAttachment targetAttachment;
	BlendParams blendParams;
	ColorWritemaskParams writemaskParams;
	// specifying rasterization and visibility state
	// NOTE: layered rendering (in GS) is always disabled
	SampleCoverageParams sampleCoverageParams;
	// specifying tessellation state
	// NOTE: tessellation is always disabled
	// vertex array object
	BackendVertexArray* vertexArray = nullptr;
	// instanced draw
	bool isInstanced = false;
};

} // namespace axgl

#endif // __PipelineState_h_
