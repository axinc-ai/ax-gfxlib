// ProgramMetal.mm
#include "ProgramMetal.h"
#include "ContextMetal.h"
#include "ShaderMetal.h"
#include "../spirv_msl/ProgramSpirvMsl.h"
#include "../spirv_msl/ShaderSpirvMsl.h"
#include "../../AXGLAllocatorImpl.h"

#if 1
#define BACKEND_PROGRAM_DBGOUT(...)
#else
#define BACKEND_PROGRAM_DBGOUT AXGL_DBGOUT
#endif

namespace axgl {

// BackendProgramクラスの実装 --------
BackendProgram* BackendProgram::create()
{
	ProgramMetal* program = AXGL_NEW(ProgramMetal);
	return program;
}
	
void BackendProgram::destroy(BackendProgram* program)
{
	if (program == nullptr) {
		return;
	}
	AXGL_DELETE(program);
	return;
}

// ProgramMetalクラスの実装 --------
static constexpr char c_gl_FragColorStr[] = {"gl_FragColor"};
static constexpr char c_gl_FragData0Str[] = {"gl_FragData[0]"};
static constexpr char c_gl_FragData1Str[] = {"gl_FragData[1]"};
static constexpr char c_gl_FragData2Str[] = {"gl_FragData[2]"};
static constexpr char c_gl_FragData3Str[] = {"gl_FragData[3]"};

ProgramMetal::ProgramMetal()
{
}

ProgramMetal::~ProgramMetal()
{
}

bool ProgramMetal::initialize(BackendContext* context)
{
	if (context == nullptr) {
		return false;
	}
	ContextMetal* ctx = static_cast<ContextMetal*>(context);
	SpirvMsl* spirv_msl = ctx->getBackendSpirvMsl();
	AXGL_ASSERT(spirv_msl != nullptr);
	m_pProgramMsl = spirv_msl->createProgram();
	if (m_pProgramMsl == nullptr) {
		return false;
	}
	m_vsLibrary = nil;
	m_fsLibrary = nil;
	m_vsFunction = nil;
	m_fsFunction = nil;
	m_globalBlockIndex = 0;
	m_globalBlockMemory = nullptr;
	m_globalBlockSize = 0;
	// set invalid index to uniform block binding
	for (int32_t i = 0; i < AXGL_MAX_UNIFORM_BUFFER_BINDINGS; i++) {
		m_uniformBlockBinding[i] = -1;
		m_vsUniformBlockIndices[i] = -1;
		m_fsUniformBlockIndices[i] = -1;
	}
	// set invalid location to attributes
	for (int32_t i = 0; i < AXGL_MAX_VERTEX_ATTRIBS; i++) {
		m_attribLocations[i] = -1;
	}
	// set invalid indices to texture/sampler
	for (int32_t i = 0; i < AXGL_MAX_COMBINED_TEXTURE_IMAGE_UNITS; i++) {
		m_vsTextureSamplerIndices[i] = -1;
		m_fsTextureSamplerIndices[i] = -1;
	}
	m_programDirty = false;
	m_uniformBlockBindingDirty = false;
	m_samplerUnitDirty = false;
	return true;
}

void ProgramMetal::terminate(BackendContext* context)
{
	if ((context == nullptr) || (m_pProgramMsl == nullptr)) {
		return;
	}
	ContextMetal* ctx = static_cast<ContextMetal*>(context);
	SpirvMsl* spirv_msl = ctx->getBackendSpirvMsl();
	AXGL_ASSERT(spirv_msl != nullptr);
	spirv_msl->destroyProgram(m_pProgramMsl);
	m_pProgramMsl = nullptr;
	releaseGlobalBuffers();
	m_globalBlockIndex = 0;
	m_globalBlockMemory = nullptr;
	m_globalBlockSize = 0;
	m_vsFunction = nil;
	m_fsFunction = nil;
	m_vsLibrary = nil;
	m_fsLibrary = nil;
	m_programDirty = true;
	m_uniformBlockBindingDirty = true;
	m_samplerUnitDirty = true;
	return;
}

bool ProgramMetal::link(BackendContext* context, BackendShader* vs, BackendShader* fs)
{
	if ((context == nullptr) || (m_pProgramMsl == nullptr)) {
		return false;
	}
	ShaderMetal* vs_metal = nullptr;
	if (vs != nullptr) {
		vs_metal = static_cast<ShaderMetal*>(vs);
		m_pProgramMsl->setVertexShader(vs_metal->getShaderMsl());
	}
	ShaderMetal* fs_metal = nullptr;
	if (fs != nullptr) {
		fs_metal = static_cast<ShaderMetal*>(fs);
		m_pProgramMsl->setFragmentShader(fs_metal->getShaderMsl());
	}
	// link処理
	ContextMetal* mtl_context = static_cast<ContextMetal*>(context);
	bool link_result = m_pProgramMsl->link(mtl_context->getBackendSpirvMsl());
	if (link_result) {
		AXGL_ASSERT(mtl_context != nullptr);
		MTLCompileOptions* options = mtl_context->getCompileOptions();
		NSString* entry_name = @"main0";
		// VSのMSLから、MTLFunctionを作成する
		const char* vs_c_str = m_pProgramMsl->getVsMsl().c_str();
		NSString* vs_msl = [NSString stringWithCString:vs_c_str encoding:NSUTF8StringEncoding];
		if (vs_msl != nullptr) {
			NSError* error_obj = nullptr;
			id<MTLDevice> device = mtl_context->getDevice();
			m_vsLibrary = [device newLibraryWithSource:vs_msl options:options error:&error_obj];
			if (m_vsLibrary != nil) {
				m_vsFunction = [m_vsLibrary newFunctionWithName:entry_name];
				if (m_vsFunction == nil) {
					AXGL_DBGOUT("VS newFunctionWithName failed¥n");
				}
			} else {
				if (error_obj != nullptr) {
					NSLog(@"%@", [error_obj localizedDescription]);
				}
			}
		}
		// FSのMSLから、MTLFunctionを作成する
		const char* fs_c_str = m_pProgramMsl->getFsMsl().c_str();
		NSString* fs_msl = [NSString stringWithCString:fs_c_str encoding:NSUTF8StringEncoding];
		if (fs_msl != nullptr) {
			NSError* error_obj = nullptr;
			id<MTLDevice> device = mtl_context->getDevice();
			m_fsLibrary = [device newLibraryWithSource:fs_msl options:options error:&error_obj];
			if (m_fsLibrary != nil) {
				m_fsFunction = [m_fsLibrary newFunctionWithName:entry_name];
				if (m_fsFunction == nil) {
					AXGL_DBGOUT("FS newFunctionWithName failed¥n");
				}
			} else {
				if (error_obj != nullptr) {
					NSLog(@"%@", [error_obj localizedDescription]);
				}
			}
		}
		// uniform block
		int32_t num_uniform_block = m_pProgramMsl->getNumUniformBlocks();
		for (int32_t i = 0; i < num_uniform_block; i++) {
			// uniform blockの metal index を取得
			m_vsUniformBlockIndices[i] = m_pProgramMsl->getVsUniformBlcokMetalIndex(i);
			m_fsUniformBlockIndices[i] = m_pProgramMsl->getFsUniformBlockMetalIndex(i);
			// uniform blockのGL bindingの初期値取得
			m_uniformBlockBinding[i] = m_pProgramMsl->getUniformBlockBinding(i);
		}
		// texture/sampler
		int32_t num_texture_sampler = m_pProgramMsl->getNumTextureSamplers();
		for (int32_t i = 0; i < num_texture_sampler; i++) {
			// texture/samplerのmetal indexを取得
			m_vsTextureSamplerIndices[i] = m_pProgramMsl->getVsTextureSamplerMetalIndex(i);
			m_fsTextureSamplerIndices[i] = m_pProgramMsl->getFsTextureSamplerMetalIndex(i);
			// type
			m_samplerParams[i].type = m_pProgramMsl->getTextureSamplerType(i);
			// sampler layout binding
			m_samplerParams[i].unit = m_pProgramMsl->getTextureSamplerBinding(i);
		}
	} else {
		// TODO: glslangリンク失敗時の処理
	}
	// Global uniform blockのバッファを作成
	if (!setupGlobalBuffers(mtl_context, vs_metal, fs_metal)) {
		AXGL_DBGOUT("BackendProgramMetal::link> setupGlobalBuffers FAILED\n");
		link_result = false;
	}
	// リンクを実行したため、全要素をダーティに設定
	m_programDirty = true;
	m_uniformBlockBindingDirty = true;
	m_samplerUnitDirty = true;
	return link_result;
}

int32_t ProgramMetal::getNumActiveAttribs() const
{
	if (m_pProgramMsl == nullptr) {
		return 0;
	}
	return m_pProgramMsl->getNumPipeInputs();
}

int32_t ProgramMetal::getNumActiveUniforms() const
{
	if (m_pProgramMsl == nullptr) {
		return 0;
	}
	return m_pProgramMsl->getNumUniformVariables();
}

int32_t ProgramMetal::getNumActiveUniformBlocks() const
{
	if (m_pProgramMsl == nullptr) {
		return 0;
	}
	return m_pProgramMsl->getNumUniformBlocks();
}

int32_t ProgramMetal::getNumFragData() const
{
	if (m_pProgramMsl == nullptr) {
		return 0;
	}
	return m_pProgramMsl->getNumPipeOutputs();
}

const char* ProgramMetal::getActiveVertexAttribName(int32_t index) const
{
	if (m_pProgramMsl == nullptr) {
		return nullptr;
	}
	return m_pProgramMsl->getPipeInputName(index);
}

const char* ProgramMetal::getActiveUniformName(int32_t index) const
{
	if (m_pProgramMsl == nullptr) {
		return nullptr;
	}
	return m_pProgramMsl->getUniformVariableName(index);
}

const char* ProgramMetal::getActiveUniformBlockName(int32_t index) const
{
	if (m_pProgramMsl == nullptr) {
		return nullptr;
	}
	return m_pProgramMsl->getUniformBlockName(index);
}

const char* ProgramMetal::getFragDataName(int32_t index) const
{
	if (m_pProgramMsl == nullptr) {
		return nullptr;
	}
	// gl_FragColor/glFragDataの置き換え
	const char* name = m_pProgramMsl->getPipeOutputName(index);
	if (strcmp(name, ShaderSpirvMsl::getDefaultFragColorName()) == 0) {
		name = c_gl_FragColorStr;
	} else if (strcmp(name, ShaderSpirvMsl::getDefaultFragDataName(0)) == 0) {
		name = c_gl_FragData0Str;
	} else if (strcmp(name, ShaderSpirvMsl::getDefaultFragDataName(1)) == 0) {
		name = c_gl_FragData1Str;
	} else if (strcmp(name, ShaderSpirvMsl::getDefaultFragDataName(2)) == 0) {
		name = c_gl_FragData2Str;
	} else if (strcmp(name, ShaderSpirvMsl::getDefaultFragDataName(3)) == 0) {
		name = c_gl_FragData3Str;
	}
	return name;
}

int32_t ProgramMetal::getActiveVertexAttribIndex(int32_t index) const
{
	if (m_pProgramMsl == nullptr) {
		return -1;
	}
	return m_pProgramMsl->getPipeInputLocation(index);
}

bool ProgramMetal::getActiveVertexAttrib(int32_t index, ShaderVariable* attrib) const
{
	if (m_pProgramMsl == nullptr) {
		return false;
	}
	return m_pProgramMsl->getPipeInput(index, attrib);
}

bool ProgramMetal::getActiveUniform(int32_t index, ShaderUniform* uniform) const
{
	if (m_pProgramMsl == nullptr) {
		return false;
	}
	return m_pProgramMsl->getUniformVariable(index, uniform);
}

bool ProgramMetal::getActiveUniformBlock(BackendContext* context, int32_t index, ShaderUniformBlock* uniformBlock) const
{
	AXGL_UNUSED(context);
	if (m_pProgramMsl == nullptr) {
		return false;
	}
	return m_pProgramMsl->getUniformBlock(index, uniformBlock);
}

uint32_t ProgramMetal::getUniformBlockBinding(int32_t index) const
{
	uint32_t binding = 0;
	if ((index >= 0) && (index < AXGL_MAX_UNIFORM_BUFFER_BINDINGS)) {
		binding = m_uniformBlockBinding[index];
	}
	return binding;
}

int32_t ProgramMetal::getFragDataLocation(int32_t index) const
{
	if (m_pProgramMsl == nullptr) {
		return -1;
	}
	return m_pProgramMsl->getPipeOutputLocation(index);
}

bool ProgramMetal::setUniformf(GLenum type, int32_t index, int num, const float* value)
{
	if ((index < 0) || (value == nullptr)) {
		return false;
	}
	// size and stride
	uint32_t size = 0;
	uint32_t stride = 0;
	uint32_t count = 0;
	get_shader_constant_copy_params(type, &size, &stride, &count);
	// bool uniforms are not supported
	AXGL_ASSERT((type != GL_BOOL) && (type != GL_BOOL_VEC2) && (type != GL_BOOL_VEC3) && (type != GL_BOOL_VEC4));
	if (m_globalBlockMemory != nullptr) {
		// write to MTLBuffer for vertex global uniforms
		uint8_t* buffer_ptr = static_cast<uint8_t*>(m_globalBlockMemory);
		const ProgramSpirvMsl::BlockMemberInfo* block_member =  m_pProgramMsl->getDefaultBlockMemberInfo(index);
		if ((block_member != nullptr) && (buffer_ptr != nullptr)) {
			// pointer
			const uint8_t* src = reinterpret_cast<const uint8_t*>(value);
			uint8_t* dst = buffer_ptr + block_member->offset;
			// copy
			for (int i = 0; i < num; i++) {
				for (uint32_t j = 0; j < count; j++) {
					memcpy(dst, src, size);
					dst += stride;
					src += size;
				}
			}
		}
	}
	return true;
}

bool ProgramMetal::setUniformi(GLenum type, int32_t index, int32_t num, const int32_t* value)
{
	if ((index < 0) || (value == nullptr)) {
		return false;
	}
	// size and stride
	uint32_t size = 0;
	uint32_t stride = 0;
	uint32_t count = 0;
	get_shader_constant_copy_params(type, &size, &stride, &count);
	// bool uniforms are not supported
	AXGL_ASSERT((type != GL_BOOL) && (type != GL_BOOL_VEC2) && (type != GL_BOOL_VEC3) && (type != GL_BOOL_VEC4));
	if (m_globalBlockMemory != nullptr) {
		// write to MTLBuffer for vertex global uniforms
		uint8_t* buffer_ptr = static_cast<uint8_t*>(m_globalBlockMemory);
		const ProgramSpirvMsl::BlockMemberInfo* block_member =  m_pProgramMsl->getDefaultBlockMemberInfo(index);
		if ((block_member != nullptr) && (buffer_ptr != nullptr)) {
			// pointer
			const uint8_t* src = reinterpret_cast<const uint8_t*>(value);
			uint8_t* dst = buffer_ptr + block_member->offset;
			// copy
			for (int i = 0; i < num; i++) {
				for (uint32_t j = 0; j < count; j++) {
					memcpy(dst, src, size);
					dst += stride;
					src += size;
				}
			}
		}
	}
	return true;
}

bool ProgramMetal::setUniformui(GLenum type, int32_t index, int32_t num, const uint32_t* value)
{
	if ((index < 0) || (value == nullptr)) {
		return false;
	}
	// size and stride
	uint32_t size = 0;
	uint32_t stride = 0;
	uint32_t count = 0;
	get_shader_constant_copy_params(type, &size, &stride, &count);
	// bool uniforms are not supported
	AXGL_ASSERT((type != GL_BOOL) && (type != GL_BOOL_VEC2) && (type != GL_BOOL_VEC3) && (type != GL_BOOL_VEC4));
	if (m_globalBlockMemory != nullptr) {
		// write to MTLBuffer for vertex global uniforms
		uint8_t* buffer_ptr = static_cast<uint8_t*>(m_globalBlockMemory);
		const ProgramSpirvMsl::BlockMemberInfo* block_member =  m_pProgramMsl->getDefaultBlockMemberInfo(index);
		if ((block_member != nullptr) && (buffer_ptr != nullptr)) {
			// pointer
			const uint8_t* src = reinterpret_cast<const uint8_t*>(value);
			uint8_t* dst = buffer_ptr + block_member->offset;
			// copy
			for (int i = 0; i < num; i++) {
				for (uint32_t j = 0; j < count; j++) {
					memcpy(dst, src, size);
					dst += stride;
					src += size;
				}
			}
		}
	}
	return true;
}

bool ProgramMetal::setUniformSampler(int32_t index, int32_t value)
{
	if ((index >= 0) && (index < AXGL_MAX_COMBINED_TEXTURE_IMAGE_UNITS)) {
		m_samplerParams[index].unit = value;
		m_samplerUnitDirty = true;
	}
	return true;
}

bool ProgramMetal::setUniformBlockBinding(int32_t index, uint32_t binding)
{
	if ((index >= 0) && (index < AXGL_MAX_UNIFORM_BUFFER_BINDINGS)) {
		m_uniformBlockBinding[index] = binding;
		m_uniformBlockBindingDirty = true;
	}
	return true;
}

void ProgramMetal::setAttribLocation(int index, int location)
{
	if ((index >= 0) && (index < AXGL_MAX_VERTEX_ATTRIBS)) {
		m_attribLocations[index] = location;
	}
	return;
}

id<MTLFunction> ProgramMetal::getVertexFunction() const
{
	return m_vsFunction;
}

id<MTLFunction> ProgramMetal::getFragmentFunction() const
{
	return m_fsFunction;
}

const void* ProgramMetal::getGlobalBlockMemory() const
{
	return m_globalBlockMemory;
}

size_t ProgramMetal::getGlobalBlockSize() const
{
	return m_globalBlockSize;
}

int32_t ProgramMetal::getGlobalBlockMetalIndex(int32_t type) const
{
	if (m_pProgramMsl == nullptr) {
		return -1;
	}	
	int32_t metal_index = m_pProgramMsl->getGlobalBlockMetalIndex();
	uint32_t shader_stages = m_pProgramMsl->getGlobalBlockShaderStages();
	if ((type == GL_VERTEX_SHADER) && ((shader_stages & ProgramSpirvMsl::VertexShaderBit) == 0)) {
		metal_index = -1; // unused in VS
	} else if ((type == GL_FRAGMENT_SHADER) && ((shader_stages & ProgramSpirvMsl::FragmentShaderBit) == 0)) {
		metal_index = -1; // unused in FS
	}
	return metal_index;
}

const ProgramMetal::SamplerDrawParams* ProgramMetal::getSamplerDrawParams() const
{
	return m_samplerParams;
}

const int32_t* ProgramMetal::getUniformBlockBinding() const
{
	return m_uniformBlockBinding;
}

const int32_t* ProgramMetal::getVsUniformBlockMetalIndices() const
{
	return m_vsUniformBlockIndices;
}

const int32_t* ProgramMetal::getFsUniformBlockMetalIndices() const
{
	return m_fsUniformBlockIndices;
}

const int32_t* ProgramMetal::getAttribLocations() const
{
	return m_attribLocations;
}

bool ProgramMetal::isProgramDirty() const
{
	return m_programDirty;
}

bool ProgramMetal::isUniformBlockBindingDirty() const
{
	return m_uniformBlockBindingDirty;
}

bool ProgramMetal::isSamplerUnitDirty() const
{
	return m_samplerUnitDirty;
}

void ProgramMetal::clearDirty()
{
	m_programDirty = false;
	m_uniformBlockBindingDirty = false;
	m_samplerUnitDirty = false;
}

const int32_t* ProgramMetal::getVsTextureSamplerMetalIndices() const
{
	return m_vsTextureSamplerIndices;
}

const int32_t* ProgramMetal::getFsTextureSamplerMetalIndices() const
{
	return m_fsTextureSamplerIndices;
}

int32_t ProgramMetal::getNumSampler() const
{
	if (m_pProgramMsl == nullptr) {
		return 0;
	}
	return m_pProgramMsl->getNumTextureSamplers();
}

bool ProgramMetal::setupGlobalBuffers(ContextMetal* context, ShaderMetal* vs, ShaderMetal* fs)
{
	AXGL_ASSERT((context != nullptr) && (vs != nullptr) && (fs != nullptr));
	AXGL_UNUSED(context);
	// Default uniform block用のバッファをリリース
	releaseGlobalBuffers();
	// Default uniform block用のバッファを確保
	m_globalBlockIndex = m_pProgramMsl->getGlobalBlockIndex();
	uint32_t global_block_size = m_pProgramMsl->getGlobalBlockSize();
	if (global_block_size > 0) {
		m_globalBlockMemory = AXGL_ALLOC(global_block_size);
		AXGL_ASSERT(m_globalBlockMemory != nullptr);
		// GL仕様から0クリア
		memset(m_globalBlockMemory, 0, global_block_size);
	}
	m_globalBlockSize = global_block_size;
	return true;
}

void ProgramMetal::releaseGlobalBuffers()
{
	if (m_globalBlockMemory != nullptr) {
		AXGL_FREE(m_globalBlockMemory);
		m_globalBlockMemory = nullptr;
	}
	m_globalBlockSize = 0;

	return;
}

} // namespace axgl
