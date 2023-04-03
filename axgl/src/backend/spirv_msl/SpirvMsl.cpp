// BackendSpirvCross.cpp
// glslang,SPIRV-Crossを使用するための実装
#include "SpirvMsl.h"
#include "ShaderSpirvMsl.h"
#include "ProgramSpirvMsl.h"
#include "glslang/Public/ShaderLang.h"

#include "../../AXGLAllocatorImpl.h"

namespace axgl {

SpirvMsl::SpirvMsl()
{
}

SpirvMsl::~SpirvMsl()
{
	terminate();
}

bool SpirvMsl::initialize()
{
	bool init_result = glslang::InitializeProcess();
	if (!init_result) {
		AXGL_DBGOUT("glslang::InitializeProcess FAILED\n");
	}
	snprintf(m_defaultUniformBlockIndexStr, sizeof(m_defaultUniformBlockIndexStr), "%d", AXGL_MSL_DEFAULT_UNIFORM_BLOCK_INDEX);
	return init_result;
}

void SpirvMsl::terminate()
{
	glslang::FinalizeProcess();
	return;
}

ShaderSpirvMsl* SpirvMsl::createShader(GLenum type)
{
	ShaderSpirvMsl* p_shader = AXGL_NEW(ShaderSpirvMsl);
	if (p_shader != nullptr) {
		bool shader_init = p_shader->initialize(this, type);
		if (!shader_init) {
			AXGL_DELETE(p_shader);
			p_shader = nullptr;
		}
	}
	return p_shader;
}

void SpirvMsl::destroyShader(ShaderSpirvMsl* shader)
{
	if (shader == nullptr) {
		return;
	}
	shader->terminate(this);
	AXGL_DELETE(shader);
	return;
}

ProgramSpirvMsl* SpirvMsl::createProgram()
{
	ProgramSpirvMsl* p_program = AXGL_NEW(ProgramSpirvMsl);
	if (p_program != nullptr) {
		bool program_init = p_program->initialize(this);
		if (!program_init) {
			AXGL_DELETE(p_program);
			p_program = nullptr;
		}
	}
	return p_program;
}

void SpirvMsl::destroyProgram(ProgramSpirvMsl* program)
{
	if (program == nullptr) {
		return;
	}
	program->terminate(this);
	AXGL_DELETE(program);
	return;
}

} // namespace axgl
