// ShaderMetal.mm
#include "ShaderMetal.h"
#include "ContextMetal.h"
#include "../spirv_msl/SpirvMsl.h"
#include "../spirv_msl/ShaderSpirvMsl.h"
#include "../../AXGLAllocatorImpl.h"

namespace axgl {

// BackendShaderクラスの実装 --------
BackendShader* BackendShader::create()
{
	ShaderMetal* shader = AXGL_NEW(ShaderMetal);
	return shader;
}
	
void BackendShader::destroy(BackendShader* shader)
{
	if (shader == nullptr) {
		return;
	}
	AXGL_DELETE(shader);
	return;
}

// ShaderMetalクラスの実装 --------
ShaderMetal::ShaderMetal()
{
}

ShaderMetal::~ShaderMetal()
{
}

bool ShaderMetal::initialize(BackendContext* context, GLenum type)
{
	if (context == nullptr) {
		return false;
	}
	m_type = type;
	ContextMetal* ctx = static_cast<ContextMetal*>(context);
	SpirvMsl* spirv_msl = ctx->getBackendSpirvMsl();
	AXGL_ASSERT(spirv_msl != nullptr);
	m_pShaderMsl = spirv_msl->createShader(type);
	return (m_pShaderMsl != nullptr);
}

void ShaderMetal::terminate(BackendContext* context)
{
	if ((context == nullptr) || (m_pShaderMsl == nullptr)) {
		return;
	}
	ContextMetal* ctx = static_cast<ContextMetal*>(context);
	SpirvMsl* spirv_msl = ctx->getBackendSpirvMsl();
	AXGL_ASSERT(spirv_msl != nullptr);
	spirv_msl->destroyShader(m_pShaderMsl);
	m_pShaderMsl = nullptr;
	return;
}

bool ShaderMetal::compileSource(BackendContext* context, const char* source)
{
	if (context == nullptr) {
		return true;
	}
	if ((source == nullptr) || (m_pShaderMsl == nullptr)) {
		return false;
	}
	ContextMetal* ctx = static_cast<ContextMetal*>(context);
	SpirvMsl* spirv_msl = ctx->getBackendSpirvMsl();
	AXGL_ASSERT(spirv_msl != nullptr);
	bool result = m_pShaderMsl->compileSource(spirv_msl, source);
	if (!result) {
		AXGL_DBGOUT("compileSource() FAILED¥n");
	}
	return result;
}

} // namespace axgl
