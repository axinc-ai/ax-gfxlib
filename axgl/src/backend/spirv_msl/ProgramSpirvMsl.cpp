// ProgramSpirvMsl.cpp
// SPIRV-Crossを使用したProgramクラスの実装
#include "glslang/Public/ShaderLang.h"
#include "ProgramSpirvMsl.h"
#include "ShaderSpirvMsl.h"
#include "SpirvMsl.h"
#include "glslang/Public/ResourceLimits.h"
#include "SPIRV/GlslangToSpv.h"
#include "spirv_cross/spirv_msl.hpp"

#include "../../common/axglCommon.h"
#include "../../AXGLAllocatorImpl.h"

#if 1
#define AXGL_PROGRAM_MSL_DBGOUT(...)
#else
#define AXGL_PROGRAM_MSL_DBGOUT AXGL_DBGOUT
#endif

namespace axgl {

ProgramSpirvMsl::ProgramSpirvMsl()
{
}

ProgramSpirvMsl::~ProgramSpirvMsl()
{
}

bool ProgramSpirvMsl::initialize(SpirvMsl* context)
{
	AXGL_UNUSED(context);
	if (m_pProgram != nullptr) {
		return true;
	}
	m_pProgram = AXGL_NEW(glslang::TProgram);
	m_defaultBlockMemberInfo.clear();
	m_uniformBlockIndices.clear();
	m_vsUniformBlockMetalIndices.clear();
	m_fsUniformBlockMetalIndices.clear();
	m_uniformNames.clear();
	m_isDefaultBlockMember.clear();
    m_defaultBlockIndex = -1;
	m_defaultBlockMetalIndex = -1;
	return (m_pProgram != nullptr);
}

void ProgramSpirvMsl::terminate(SpirvMsl* context)
{
	AXGL_UNUSED(context);
	if (m_pProgram == nullptr) {
		return;
	}
	AXGL_DELETE(m_pProgram);
	m_pProgram = nullptr;
	m_defaultBlockMemberInfo.clear();
	m_uniformBlockIndices.clear();
	m_vsUniformBlockMetalIndices.clear();
	m_fsUniformBlockMetalIndices.clear();
	m_uniformNames.clear();
	m_isDefaultBlockMember.clear();
	m_defaultBlockIndex = -1;
	m_defaultBlockMetalIndex = -1;
	return;
}

void ProgramSpirvMsl::setVertexShader(ShaderSpirvMsl* vs)
{
	if ((vs != nullptr) && (vs->getLanguageType() != EShLangVertex)) {
		return;
	}
	m_pVertexShader = vs;
	return;
}

void ProgramSpirvMsl::setFragmentShader(ShaderSpirvMsl* fs)
{
	if ((fs != nullptr) && (fs->getLanguageType() != EShLangFragment)) {
		return;
	}
	m_pFragmentShader = fs;
	return;
}

bool ProgramSpirvMsl::link(const SpirvMsl* spirvMsl)
{
	if ((spirvMsl == nullptr) || (m_pProgram == nullptr)) {
		return false;
	}
	if (m_pVertexShader != nullptr) {
		if (!m_pVertexShader->isCompiled()) {
			return false;
		}
		m_pProgram->addShader(m_pVertexShader->getGlslangShader());
	}
	if (m_pFragmentShader != nullptr) {
		if (!m_pFragmentShader->isCompiled()) {
			return false;
		}
		m_pProgram->addShader(m_pFragmentShader->getGlslangShader());
	}
	// clear
	m_defaultBlockMemberInfo.clear();
	m_uniformBlockIndices.clear();
	m_vsUniformBlockMetalIndices.clear();
	m_fsUniformBlockMetalIndices.clear();
	m_uniformNames.clear();
	m_isDefaultBlockMember.clear();
	m_samplerNativeIndices.clear(); // uniform indexで参照して、m_samplerTypeとm_samplerBindingを得るためのインデックス
	m_samplerType.clear();
	m_samplerBinding.clear();
	m_vsTextureSamplerMetalIndices.clear();
	m_fsTextureSamplerMetalIndices.clear();
	// link
	EShMessages link_messages = EShMsgDefault;
	bool link_result = m_pProgram->link(link_messages);
	if (link_result) {
		// map I/O
		if (!m_pProgram->mapIO()) {
			link_result = false;
		}
		if (link_result) {
			// build reflection
			link_result = m_pProgram->buildReflection(EShReflectionDefault);
		}
		glslang::SpvOptions spv_options;
		std::vector<unsigned int> vs_spirv;
		std::vector<unsigned int> fs_spirv;
		if (m_pVertexShader != nullptr) {
			glslang::GlslangToSpv(*m_pProgram->getIntermediate(EShLangVertex), vs_spirv, &spv_options);
		}
		if (m_pFragmentShader != nullptr) {
			glslang::GlslangToSpv(*m_pProgram->getIntermediate(EShLangFragment), fs_spirv, &spv_options);
		}
		// convert to MSL
		if (!vs_spirv.empty()) {
			spirv_cross::CompilerMSL msl(vs_spirv.data(), vs_spirv.size());
			spirv_cross::CompilerMSL::Options msl_options;
			msl_options.platform = spirv_cross::CompilerMSL::Options::iOS;
			msl.set_msl_options(msl_options);
			m_vsMsl.assign(msl.compile().c_str());
			AXGL_PROGRAM_MSL_DBGOUT("VS MSL====\n%s\n", m_vsMsl.c_str());
			AXGLString::size_type pos = m_vsMsl.find(ShaderSpirvMsl::getDefaultBlockInputName());
			if (pos != AXGLString::npos) {
				AXGLString::size_type pos_l = m_vsMsl.find("[[buffer(", pos);
				AXGLString::size_type pos_r = m_vsMsl.find(")]]", pos);
				if ((pos_l != AXGLString::npos) && (pos_r != AXGLString::npos)) {
					unsigned long len = pos_r - pos_l - 9;
					m_vsMsl.replace(pos_l + 9, len, spirvMsl->getDefaultUniformBlockIndexStr());
					AXGL_PROGRAM_MSL_DBGOUT("VS MSL(Buffer)====\n%s\n", m_vsMsl.c_str());
				}
			}
			// GL正規化デバイス座標のZ軸をMetalの座標系に収まるように変換
			AXGLString::size_type pos_return = m_vsMsl.find("return out;");
			if (pos_return != AXGLString::npos) {
				AXGLString::size_type pos_nl = m_vsMsl.rfind("\n", pos_return);
				if ((pos_nl != AXGLString::npos)) {
					m_vsMsl.insert(pos_nl + 1, "    out.gl_Position.z = 0.5 * (out.gl_Position.z + out.gl_Position.w);\n");
					AXGL_PROGRAM_MSL_DBGOUT("VS MSL(Z scale)====\n%s\n", m_vsMsl.c_str());
				}
			}
		}
		if (!fs_spirv.empty()) {
			spirv_cross::CompilerMSL msl(fs_spirv.data(), fs_spirv.size());
			spirv_cross::CompilerMSL::Options msl_options;
			msl_options.platform = spirv_cross::CompilerMSL::Options::iOS;
			msl.set_msl_options(msl_options);
			m_fsMsl.assign(msl.compile().c_str());
			AXGL_PROGRAM_MSL_DBGOUT("FS MSL====\n%s\n", m_fsMsl.c_str());
			AXGLString::size_type pos = m_fsMsl.find(ShaderSpirvMsl::getDefaultBlockInputName());
			if (pos != AXGLString::npos) {
				AXGLString::size_type pos_l = m_fsMsl.find("[[buffer(", pos);
				AXGLString::size_type pos_r = m_fsMsl.find(")]]", pos);
				if ((pos_l != AXGLString::npos) && (pos_r != AXGLString::npos)) {
					unsigned long len = pos_r - pos_l - 9;
					m_fsMsl.replace(pos_l + 9, len, spirvMsl->getDefaultUniformBlockIndexStr());
				}
				AXGL_PROGRAM_MSL_DBGOUT("FS MSL(Mod)====\n%s\n", m_fsMsl.c_str());
			}
		}
		int32_t default_block_index = -1;
		int32_t default_block_size = 0;
		int32_t default_block_metal_index = -1;
		uint32_t default_block_stages = 0;
		static const char* default_block_name = ShaderSpirvMsl::getDefaultBlockName();
		const int32_t num_block = m_pProgram->getNumUniformBlocks();
		for (int32_t i = 0; i < num_block; i++) {
			const glslang::TObjectReflection& obj_ref = m_pProgram->getUniformBlock(i);
			if (strcmp(default_block_name, obj_ref.name.c_str()) == 0) {
				default_block_index = i;
				default_block_size = obj_ref.size;
				default_block_stages = convertShaderStages(static_cast<uint32_t>(obj_ref.stages));
				// NOTE: layoutBindingはMSLに反映されない
				default_block_metal_index = AXGL_MSL_DEFAULT_UNIFORM_BLOCK_INDEX;
			} else {
				// uniform block index
				m_uniformBlockIndices.emplace_back(i);
				int32_t binding = 0;
				const glslang::TObjectReflection& obj_ref = m_pProgram->getUniformBlock(i);
				const glslang::TType* p_type = obj_ref.getType();
				if (p_type != nullptr) {
					const glslang::TQualifier& qualifier = p_type->getQualifier();
					binding = static_cast<int32_t>(qualifier.layoutBinding);
				}
				m_uniformBlockBinding.emplace_back(binding);
			}
		}
		m_defaultBlockIndex = default_block_index;
		m_defaultBlockSize = default_block_size;
		m_defaultBlockMetalIndex = default_block_metal_index;
		m_defaultBlockStages = default_block_stages;
		// default uniform block判別後に、他のuniform blockのmetal indexを処理
		// NOTE: layoutBindingはMSLに反映されないため、SPIRV-Crossが割り当てたindexをMSLから読み取る
		if (!m_vsMsl.empty()) {
			getUniformBlockMetalIndicesFromMSL(m_vsUniformBlockMetalIndices, m_vsMsl);
		}
		if (!m_fsMsl.empty()) {
			getUniformBlockMetalIndicesFromMSL(m_fsUniformBlockMetalIndices, m_fsMsl);
		}
		
		if (default_block_index >= 0) {
			int32_t num_uniform = m_pProgram->getNumUniformVariables();
			static const char block_separator = '.';
			for (int32_t i = 0; i < num_uniform; i++) {
				const glslang::TObjectReflection& obj_ref = m_pProgram->getUniform(i);
				bool is_default_block_member = (strchr(obj_ref.name.c_str(), block_separator) == nullptr);
				if (is_default_block_member) {
					BlockMemberInfo member_info {
						obj_ref.offset,
						obj_ref.glDefineType,
						obj_ref.size
					};
					m_defaultBlockMemberInfo.emplace(i, member_info);
				}
				m_uniformNames.emplace_back(obj_ref.name.c_str());
				m_isDefaultBlockMember.emplace_back(is_default_block_member);
			}
		}
		// Sampler
		{
			// uniform variablesからsamplerの名前とlayoutBindingを取得
			const int32_t num_uniforms = m_pProgram->getNumUniformVariables();
			int32_t num_samplers = 0;
			m_samplerNativeIndices.resize(num_uniforms);
			for (int32_t i = 0; i < num_uniforms; i++) {
				const glslang::TObjectReflection& obj_ref = m_pProgram->getUniform(i);
				if (isSamplerType(obj_ref.glDefineType)) {
					const glslang::TType* p_type = obj_ref.getType();
					int32_t binding = 0;
					if (p_type != nullptr) {
						const glslang::TQualifier& qualifier = p_type->getQualifier();
						binding = static_cast<int32_t>(qualifier.layoutBinding);
					}
					m_samplerNativeIndices[i] = num_samplers;
					m_samplerType.emplace_back(obj_ref.glDefineType);
					m_samplerBinding.emplace_back(binding);
					num_samplers++;
					AXGL_PROGRAM_MSL_DBGOUT("sampler: name:%s binding:%d\n", obj_ref.name.c_str(), binding);
				} else {
					m_samplerNativeIndices[i] = -1;
				}
			}
			// MSLからtextureのインデックスを読み取る(SPIRV-Cross出力MSLのsamplerはtextureと同じインデックス)
			if (num_samplers > 0) {
				if (!m_vsMsl.empty()) {
					AXGL_PROGRAM_MSL_DBGOUT("VS textures ----\n");
					getTextureMetalIndicesFromMSL(m_vsTextureSamplerMetalIndices, m_vsMsl, num_samplers);
				}
				if (!m_fsMsl.empty()) {
					AXGL_PROGRAM_MSL_DBGOUT("FS textures ----\n");
					getTextureMetalIndicesFromMSL(m_fsTextureSamplerMetalIndices, m_fsMsl, num_samplers);
				}
			}
		}
	}
	return link_result;
}

int32_t ProgramSpirvMsl::getNumPipeInputs() const
{
	if (m_pProgram == nullptr) {
		return 0;
	}
	return m_pProgram->getNumPipeInputs();
}

int32_t ProgramSpirvMsl::getNumUniformVariables() const
{
	if (m_pProgram == nullptr) {
		return 0;
	}
	return m_pProgram->getNumUniformVariables();
}

int32_t ProgramSpirvMsl::getNumUniformBlocks() const
{
	if (m_pProgram == nullptr) {
		return 0;
	}
	// NOTE: default uniform blockを除いた index 配列のサイズ
	return static_cast<int32_t>(m_uniformBlockIndices.size());
}

int32_t ProgramSpirvMsl::getNumPipeOutputs() const
{
	if (m_pProgram == nullptr) {
		return 0;
	}
	return m_pProgram->getNumPipeOutputs();
}

const char* ProgramSpirvMsl::getPipeInputName(int32_t index) const
{
	if (m_pProgram == nullptr) {
		return nullptr;
	}
	return m_pProgram->getPipeInput(index).name.c_str();
}

const char* ProgramSpirvMsl::getUniformVariableName(int32_t index) const
{
	if (m_pProgram == nullptr) {
		return nullptr;
	}
	AXGL_ASSERT(index < static_cast<int32_t>(m_uniformNames.size()));
	return m_uniformNames[index].c_str();
}

const char* ProgramSpirvMsl::getUniformBlockName(int32_t index) const
{
	if (m_pProgram == nullptr) {
		return nullptr;
	}
	AXGL_ASSERT(index < static_cast<int32_t>(m_uniformBlockIndices.size()));
	int32_t source_index = m_uniformBlockIndices[index];
	AXGL_ASSERT(source_index < static_cast<int32_t>(m_pProgram->getNumUniformBlocks()));
	return m_pProgram->getUniformBlock(source_index).name.c_str();
}

const char* ProgramSpirvMsl::getPipeOutputName(int32_t index) const
{
	if (m_pProgram == nullptr) {
		return nullptr;
	}
	return m_pProgram->getPipeOutput(index).name.c_str();
}

int32_t ProgramSpirvMsl::getPipeInputLocation(int32_t index) const
{
	if ((m_pProgram == nullptr) || (index >= m_pProgram->getNumPipeInputs())) {
		return -1;
	}
	const glslang::TObjectReflection& obj_ref = m_pProgram->getPipeInput(index);
	const glslang::TType* p_type = obj_ref.getType();
	if (p_type == nullptr) {
		return -1;
	}
	const glslang::TQualifier& qualifier = p_type->getQualifier();
	return static_cast<int32_t>(qualifier.layoutLocation);
}

int32_t ProgramSpirvMsl::getPipeOutputLocation(int32_t index) const
{
	if ((m_pProgram == nullptr) || (index >= m_pProgram->getNumPipeOutputs())) {
		return -1;
	}
	const glslang::TObjectReflection& obj_ref = m_pProgram->getPipeOutput(index);
	const glslang::TType* p_type = obj_ref.getType();
	if (p_type == nullptr) {
		return -1;
	}
	const glslang::TQualifier& qualifier = p_type->getQualifier();
	return static_cast<int32_t>(qualifier.layoutLocation);	
}

bool ProgramSpirvMsl::getPipeInput(int32_t index, BackendProgram::ShaderVariable* sv) const
{
	if ((m_pProgram == nullptr) || (index >= m_pProgram->getNumPipeInputs()) || (sv == nullptr)) {
		return false;
	}
	const glslang::TObjectReflection& obj_ref = m_pProgram->getPipeInput(index);
	sv->m_Type = obj_ref.glDefineType;
	sv->m_Size = obj_ref.size;
	const glslang::TType* p_type = obj_ref.getType();
	if (p_type != nullptr) {
		const glslang::TQualifier& qualifier = p_type->getQualifier();
		sv->m_NativeIndex = static_cast<int32_t>(qualifier.layoutLocation);
		sv->m_LayoutSpecified = qualifier.hasLocation() ? GL_TRUE : GL_FALSE;
	}
	return true;
}

bool ProgramSpirvMsl::getUniformVariable(int32_t index, BackendProgram::ShaderUniform* su) const
{
	if ((m_pProgram == nullptr) || (su == nullptr)) {
		return false;
	}
	AXGL_ASSERT(index < static_cast<int32_t>(m_pProgram->getNumUniformVariables()));
	const glslang::TObjectReflection& obj_ref = m_pProgram->getUniform(index);

	su->m_Type = obj_ref.glDefineType;
	su->m_Size = obj_ref.size;
	su->m_BlockIndex = obj_ref.index;
	su->m_Offset = obj_ref.offset;
	su->m_ArrayStride = obj_ref.arrayStride;
	su->m_MatrixStride = obj_ref.getType()->isMatrix() ? 16 : 0;
	su->m_IsRowMajor = GL_FALSE;
	const bool is_default_block_member = m_isDefaultBlockMember[index];
	const bool is_sampler = isSamplerType(obj_ref.glDefineType);
	su->m_IsDefaultBlock = is_default_block_member ? GL_TRUE : GL_FALSE;
	su->m_IsUniformBlockMember = (!is_default_block_member && !is_sampler) ? GL_TRUE : GL_FALSE;
	su->m_IsSampler = is_sampler ? GL_TRUE : GL_FALSE;
	if (is_sampler) {
		su->m_NativeIndex = m_samplerNativeIndices[index];
	} else {
		su->m_NativeIndex = index;
	}

	return true;
}

bool ProgramSpirvMsl::getUniformBlock(int32_t index, BackendProgram::ShaderUniformBlock* sub) const
{
	if ((m_pProgram == nullptr) || (sub == nullptr)) {
		return false;
	}
	AXGL_ASSERT(index < static_cast<int32_t>(m_uniformBlockIndices.size()));
	int32_t source_index = m_uniformBlockIndices[index];
	AXGL_ASSERT(source_index < static_cast<int32_t>(m_pProgram->getNumUniformBlocks()));
	const glslang::TObjectReflection& obj_ref = m_pProgram->getUniformBlock(source_index);

	sub->m_DataSize = obj_ref.size;
	sub->m_ActiveUniforms = obj_ref.numMembers;
	sub->m_NativeIndex = index;

	return true;
}

int32_t ProgramSpirvMsl::getGlobalBlockIndex() const
{
    return m_defaultBlockIndex;
}

int32_t ProgramSpirvMsl::getGlobalBlockSize() const
{
    return m_defaultBlockSize;
}

int32_t ProgramSpirvMsl::getGlobalBlockMetalIndex() const
{
	return m_defaultBlockMetalIndex;
}

uint32_t ProgramSpirvMsl::getGlobalBlockShaderStages() const
{
	return m_defaultBlockStages;
}

const ProgramSpirvMsl::BlockMemberInfo* ProgramSpirvMsl::getDefaultBlockMemberInfo(int32_t index) const
{
	auto it = m_defaultBlockMemberInfo.find(index);
	if (it == m_defaultBlockMemberInfo.end()) {
		return nullptr;
	}
	return &(it->second);	
}

int32_t ProgramSpirvMsl::getVsUniformBlcokMetalIndex(int32_t index) const
{
	if (index >= m_vsUniformBlockMetalIndices.size()) {
		return -1;
	}
	return m_vsUniformBlockMetalIndices[index];
}

int32_t ProgramSpirvMsl::getFsUniformBlockMetalIndex(int32_t index) const
{
	if (index >= m_fsUniformBlockMetalIndices.size()) {
		return -1;
	}
	return m_fsUniformBlockMetalIndices[index];
}

int32_t ProgramSpirvMsl::getUniformBlockBinding(int32_t index) const
{
	if (index >= m_uniformBlockBinding.size()) {
		return 0;
	}
	return m_uniformBlockBinding[index];
}

int32_t ProgramSpirvMsl::getNumTextureSamplers() const
{
	if (m_pProgram == nullptr) {
		return 0;
	}
	return static_cast<int32_t>(m_samplerBinding.size());
}

int32_t ProgramSpirvMsl::getVsTextureSamplerMetalIndex(int32_t index) const
{
	if (index >= m_vsTextureSamplerMetalIndices.size()) {
		return -1;
	}
	return m_vsTextureSamplerMetalIndices[index];
}

int32_t ProgramSpirvMsl::getFsTextureSamplerMetalIndex(int32_t index) const
{
	if (index >= m_fsTextureSamplerMetalIndices.size()) {
		return -1;
	}
	return m_fsTextureSamplerMetalIndices[index];
}

int32_t ProgramSpirvMsl::getTextureSamplerType(int32_t index) const
{
	if (index >= m_samplerType.size()) {
		return 0;
	}
	return m_samplerType[index];
}

int32_t ProgramSpirvMsl::getTextureSamplerBinding(int32_t index) const
{
	if (index >= m_samplerBinding.size()) {
		return 0;
	}
	return m_samplerBinding[index];
}

// private methods ========================================================================

uint32_t ProgramSpirvMsl::getShaderStageMask(GLenum type)
{
	uint32_t mask = 0;
	switch (type) {
	case GL_VERTEX_SHADER:
		mask = EShLangVertexMask;
		break;
	case GL_FRAGMENT_SHADER:
		mask = EShLangFragmentMask;
		break;
	default:
		AXGL_ASSERT(0);
		break;
	}
	return mask;
}

bool ProgramSpirvMsl::isSamplerType(GLenum type)
{
	bool is_sampler = false;
	switch (type) {
	case GL_SAMPLER_2D:
	case GL_SAMPLER_2D_ARRAY:
	case GL_SAMPLER_2D_ARRAY_SHADOW:
	case GL_SAMPLER_2D_SHADOW:
	case GL_SAMPLER_3D:
	case GL_SAMPLER_CUBE:
	case GL_SAMPLER_CUBE_SHADOW:
	case GL_INT_SAMPLER_2D:
	case GL_INT_SAMPLER_3D:
	case GL_INT_SAMPLER_CUBE:
	case GL_INT_SAMPLER_2D_ARRAY:
	case GL_UNSIGNED_INT_SAMPLER_2D:
	case GL_UNSIGNED_INT_SAMPLER_3D:
	case GL_UNSIGNED_INT_SAMPLER_CUBE:
	case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
		is_sampler = true;
		break;
	default:
		break;
	}
	return is_sampler;
}

int32_t ProgramSpirvMsl::getNumUniformVariablesOfStages(uint32_t mask) const
{
	if (m_pProgram == nullptr) {
		return 0;
	}
	int32_t stage_uniform_count = 0;
	int32_t num_uniform_variables = m_pProgram->getNumUniformVariables();
	for (int32_t i = 0; i < num_uniform_variables; i++) {
		if ((m_pProgram->getUniform(i).stages & mask) == mask) {
			stage_uniform_count++;
		}
	}
	return stage_uniform_count;
}

int32_t ProgramSpirvMsl::getNumUniformBlocksOfStages(uint32_t mask) const
{
	if (m_pProgram == nullptr) {
		return 0;
	}
	int32_t stage_block_count = 0;
	int32_t num_uniform_blocks = m_pProgram->getNumUniformBlocks();
	for (int32_t i = 0; i < num_uniform_blocks; i++) {
		if ((m_pProgram->getUniformBlock(i).stages & mask) == mask) {
			stage_block_count++;
		}
	}
	return stage_block_count;
}

int32_t ProgramSpirvMsl::getNumActiveUniformBlockMemberOfStages(uint32_t mask) const
{
	if (m_pProgram == nullptr) {
		return 0;
	}
	int32_t active_member_count = 0;
	int32_t num_blocks = m_pProgram->getNumUniformBlocks();
	for (int32_t i = 0; i < num_blocks; i++) {
		const glslang::TObjectReflection& obj_ref = m_pProgram->getUniformBlock(i);
		if ((obj_ref.stages & mask) == mask) {
			active_member_count += obj_ref.numMembers;
		}
	}
	return active_member_count;
}

int32_t ProgramSpirvMsl::getNumSamplerOfStages(uint32_t mask) const
{
	if (m_pProgram == nullptr) {
		return 0;
	}
	int32_t sampler_count = 0;
	int32_t num_uniform_variables = m_pProgram->getNumUniformVariables();
	for (int32_t i = 0; i < num_uniform_variables; i++) {
		const glslang::TObjectReflection& obj_ref = m_pProgram->getUniform(i);
		if ((obj_ref.stages & mask) == mask) {
			if (isSamplerType(obj_ref.glDefineType)) {
				sampler_count++;
			}
		}
	}
	return sampler_count;
}

uint32_t ProgramSpirvMsl::convertShaderStages(uint32_t orgStages)
{
	uint32_t stages = 0;
	if ((orgStages & EShLangVertexMask) != 0) {
		stages |= VertexShaderBit;
	}
	if ((orgStages & EShLangFragmentMask) != 0) {
		stages |= FragmentShaderBit;
	}
	return stages;
}

void ProgramSpirvMsl::getUniformBlockMetalIndicesFromMSL(AXGLVector<int32_t>& indices, const AXGLString& msl)
{
	// - "main0("から"{"までを uniform block 名で検索
	// - [[buffer(X)]] の X を取得
	// - uniform block数をサイズとした配列に metal index として保持しておく
	AXGL_ASSERT(m_pProgram != nullptr);
	int32_t num_blocks = static_cast<int32_t>(m_uniformBlockIndices.size()); // default uniform blockを除いた数
	AXGLString::size_type pos_main = msl.find("main0(");
	indices.resize(num_blocks);
	std::fill(indices.begin(), indices.end(), -1);
	if (pos_main != AXGLString::npos) {
		AXGLString::size_type pos_func_start = msl.find("{", pos_main);
		for (int32_t i = 0; i < num_blocks; i++) {
			char tmp[16];
			int32_t msl_index = m_uniformBlockIndices[i];
			const char* ub_name = m_pProgram->getUniformBlockName(msl_index);
			AXGLString::size_type pos_ub_name = msl.find(ub_name, pos_main + 6);
			if ((pos_ub_name != AXGLString::npos) && (pos_ub_name < pos_func_start)) {
				AXGLString::size_type pos_buf_l = msl.find("[[buffer(", pos_ub_name);
				AXGLString::size_type pos_buf_r = msl.find(")]]", pos_ub_name);
				if ((pos_buf_l != AXGLString::npos) && (pos_buf_r != AXGLString::npos)) {
					int32_t copy_pos = static_cast<int32_t>(pos_buf_l + 9);
					int32_t copy_len = static_cast<int32_t>(pos_buf_r) - copy_pos;
					msl.copy(tmp, copy_len, copy_pos);
					tmp[copy_len] = '\0';
					int index_val = atoi(tmp);
					AXGL_PROGRAM_MSL_DBGOUT("BufferMetalIndex[%d]> %s : %d\n", i, ub_name, index_val);
					indices[i] = index_val;
				}
			}
		}
	}
	return;
}

void ProgramSpirvMsl::getTextureMetalIndicesFromMSL(AXGLVector<int32_t>& indices, const AXGLString& msl, int32_t numTextures)
{
	// - "main0("から"{"までを texture 名で検索
	// - [[texture(X)]] の X を取得
	// - texture数をサイズとした配列に metal index として保持しておく
	if (numTextures <= 0) {
		return;
	}
	const int32_t num_uniform = m_pProgram->getNumUniformVariables();
	indices.resize(numTextures);
	std:fill(indices.begin(), indices.end(), -1);
	AXGLString::size_type pos_main = msl.find("main0(");
	if (pos_main != AXGLString::npos) {
		AXGLString::size_type pos_func_start = msl.find("{", pos_main);
		int32_t dst_index = 0;
		for (int32_t i = 0; i < num_uniform; i++) {
			if (m_samplerNativeIndices[i] >= 0) { // samplerのみを対象
				char tmp[16];
				const char* tex_name = m_pProgram->getUniformName(i);
				AXGLString::size_type pos_tex_name = msl.find(tex_name, pos_main + 6);
				if ((pos_tex_name != AXGLString::npos) && (pos_tex_name < pos_func_start)) {
					AXGLString::size_type pos_buf_l = msl.find("[[texture(", pos_tex_name);
					AXGLString::size_type pos_buf_r = msl.find(")]]", pos_tex_name);
					if ((pos_buf_l != AXGLString::npos) && (pos_buf_r != AXGLString::npos)) {
						int32_t copy_pos = static_cast<int32_t>(pos_buf_l + 10);
						int32_t copy_len = static_cast<int32_t>(pos_buf_r) - copy_pos;
						msl.copy(tmp, copy_len, copy_pos);
						tmp[copy_len] = '\0';
						int index_val = atoi(tmp);
						AXGL_PROGRAM_MSL_DBGOUT("TextureMetalIndex[%d]> %s : %d\n", i, tex_name, index_val);
						indices[dst_index] = index_val;
						dst_index++;
					}
				}
			}
		}
	}
	return;
}

} // namespace axgl
