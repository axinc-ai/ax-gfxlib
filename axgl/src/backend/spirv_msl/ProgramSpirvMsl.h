// ProgramSpirvMsl.h
// SPIRV-Crossを使用したProgramクラスの宣言
#ifndef __ProgramSpirvMsl_h_
#define __ProgramSpirvMsl_h_
#include "../../common/axglCommon.h"
#include "../../AXGLString.h"
#include "../../AXGLAllocatorImpl.h"
#include "../BackendProgram.h"
#include <unordered_map>
#include <vector>

namespace glslang {
class TProgram;
} // namespace glslang

namespace axgl {

class SpirvMsl;
class ShaderSpirvMsl;

// SPIRV-Crossを使用したProgramクラス
class ProgramSpirvMsl
{
public:
	enum {
		VertexShaderBit = 1,
		FragmentShaderBit = 2
	};
	struct BlockMemberInfo {
		int32_t offset;
		int32_t glType;
		int32_t arraySize;
	};

public:
	ProgramSpirvMsl();
	~ProgramSpirvMsl();
	bool initialize(SpirvMsl* context);
	void terminate(SpirvMsl* context);
	void setVertexShader(ShaderSpirvMsl* vs);
	void setFragmentShader(ShaderSpirvMsl* fs);
	bool link(const SpirvMsl* spirvMsl);
	const AXGLString& getVsMsl() const { return m_vsMsl; }
	const AXGLString& getFsMsl() const { return m_fsMsl; }
	int32_t getNumPipeInputs() const;
	int32_t getNumUniformVariables() const;
	int32_t getNumUniformBlocks() const;
	int32_t getNumPipeOutputs() const;
	const char* getPipeInputName(int32_t index) const;
	const char* getUniformVariableName(int32_t index) const;
	const char* getUniformBlockName(int32_t index) const;
	const char* getPipeOutputName(int32_t index) const;
	int32_t getPipeInputLocation(int32_t index) const;
	int32_t getPipeOutputLocation(int32_t index) const;
	bool getPipeInput(int32_t index, BackendProgram::ShaderVariable* sv) const;
	bool getUniformVariable(int32_t index, BackendProgram::ShaderUniform* su) const;
	bool getUniformBlock(int32_t index, BackendProgram::ShaderUniformBlock* sub) const;
	int32_t getGlobalBlockIndex() const;
	int32_t getGlobalBlockSize() const;
	int32_t getGlobalBlockMetalIndex() const;
	uint32_t getGlobalBlockShaderStages() const;
	const BlockMemberInfo* getDefaultBlockMemberInfo(int32_t index) const;
	int32_t getVsUniformBlcokMetalIndex(int32_t index) const;
	int32_t getFsUniformBlockMetalIndex(int32_t index) const;
	int32_t getUniformBlockBinding(int32_t index) const;
	int32_t getNumTextureSamplers() const;
	int32_t getVsTextureSamplerMetalIndex(int32_t index) const;
	int32_t getFsTextureSamplerMetalIndex(int32_t index) const;
	int32_t getTextureSamplerType(int32_t index) const;
	int32_t getTextureSamplerBinding(int32_t index) const;

private:
	static uint32_t getShaderStageMask(GLenum type);
	static bool isSamplerType(GLenum type);
	int32_t getNumUniformVariablesOfStages(uint32_t mask) const;
	int32_t getNumUniformBlocksOfStages(uint32_t mask) const;
	int32_t getNumActiveUniformBlockMemberOfStages(uint32_t mask) const;
	int32_t getNumSamplerOfStages(uint32_t mask) const;
	static uint32_t convertShaderStages(uint32_t orgStages);
	void getUniformBlockMetalIndicesFromMSL(AXGLVector<int32_t>& indices, const AXGLString& msl);
	void getTextureMetalIndicesFromMSL(AXGLVector<int32_t>& indices, const AXGLString& msl, int32_t numTextures);

private:
    glslang::TProgram* m_pProgram = nullptr;
	ShaderSpirvMsl* m_pVertexShader = nullptr;
	ShaderSpirvMsl* m_pFragmentShader = nullptr;
	AXGLString m_vsMsl; // NOTE:VSのMSL、最終的には保持しておく必要がない
	AXGLString m_fsMsl; // NOTE:FSのMSL、最終的には保持しておく必要がない
	// NOTE: default uniform blockを除いた配列
	AXGLVector<int32_t> m_uniformBlockIndices;
	AXGLVector<int32_t> m_uniformBlockBinding;
	AXGLVector<int32_t> m_vsUniformBlockMetalIndices;
	AXGLVector<int32_t> m_fsUniformBlockMetalIndices;
	AXGLVector<AXGLString> m_uniformNames;
	AXGLVector<bool> m_isDefaultBlockMember;
	AXGLUnorderedMap<int32_t, BlockMemberInfo> m_defaultBlockMemberInfo;
	int32_t m_defaultBlockIndex = -1;
	int32_t m_defaultBlockSize = 0;
	int32_t m_defaultBlockMetalIndex = -1;
	uint32_t m_defaultBlockStages = 0;
	// 全シェーダ共通のGL sampler type/binding
	AXGLVector<int32_t> m_samplerNativeIndices; // uniform indexで参照し、m_samplerTypeとm_samplerBindingを得るためのインデックス
	AXGLVector<int32_t> m_samplerType;
	AXGLVector<int32_t> m_samplerBinding;
	// 各シェーダのMetal texture/samplerインデックス
	AXGLVector<int32_t> m_vsTextureSamplerMetalIndices;
	AXGLVector<int32_t> m_fsTextureSamplerMetalIndices;
};

} // namespace axgl

#endif // __ProgramSpirvMsl_h_
