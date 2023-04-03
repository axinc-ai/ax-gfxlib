// SamplerMetal.mm
#include "SamplerMetal.h"
#include "ContextMetal.h"
#include "../../AXGLAllocatorImpl.h"

namespace axgl {

// BackendSamplerクラスの実装 --------
BackendSampler* BackendSampler::create()
{
	SamplerMetal* sampler = AXGL_NEW(SamplerMetal);
	return sampler;
}
	
void BackendSampler::destroy(BackendSampler* sampler)
{
	if (sampler == nullptr) {
		return;
	}
	AXGL_DELETE(sampler);
	return;
}

// SamplerMetalクラスの実装 --------
static MTLSamplerAddressMode convert_address_mode(GLenum wrap)
{
	MTLSamplerAddressMode rval = MTLSamplerAddressModeRepeat;
	switch (wrap) {
	case GL_CLAMP_TO_EDGE:
		rval = MTLSamplerAddressModeClampToEdge;
		break;
	case GL_MIRRORED_REPEAT:
		rval = MTLSamplerAddressModeMirrorRepeat;
		break;
	case GL_REPEAT:
	default:
		break;
	}

	return rval;
}

// convert sampler mag filter
static MTLSamplerMinMagFilter convert_mag_filter(GLenum mag_filter)
{
	MTLSamplerMinMagFilter rval = MTLSamplerMinMagFilterLinear;
	switch (mag_filter) {
	case GL_NEAREST:
		rval = MTLSamplerMinMagFilterNearest;
		break;
	case GL_LINEAR:
	default:
		break;
	}
	return rval;
}

// convert sampler min filter
static MTLSamplerMinMagFilter convert_min_filter(GLenum min_filter)
{
	MTLSamplerMinMagFilter rval = MTLSamplerMinMagFilterNearest;
	switch (min_filter) {
	case GL_LINEAR:
	case GL_LINEAR_MIPMAP_NEAREST:
	case GL_LINEAR_MIPMAP_LINEAR:
		rval = MTLSamplerMinMagFilterLinear;
		break;
	case GL_NEAREST:
	case GL_NEAREST_MIPMAP_NEAREST:
	case GL_NEAREST_MIPMAP_LINEAR:
	default:
		break;
	}
	return rval;
}

// convert sampler mipmap filter
static MTLSamplerMipFilter convert_mip_filter(GLenum min_filter)
{
	MTLSamplerMipFilter rval = MTLSamplerMipFilterLinear;
	switch (min_filter) {
	case GL_LINEAR:
	case GL_NEAREST:
		rval = MTLSamplerMipFilterNotMipmapped;
		break;
	case GL_LINEAR_MIPMAP_NEAREST:
	case GL_NEAREST_MIPMAP_NEAREST:
		rval = MTLSamplerMipFilterNearest;
		break;
	case GL_LINEAR_MIPMAP_LINEAR:
	case GL_NEAREST_MIPMAP_LINEAR:
	default:
		break;
	}
	return rval;
}

// convert compare function
static MTLCompareFunction convert_compare_function(GLenum mode, GLenum func)
{
	MTLCompareFunction rval = MTLCompareFunctionNever;
	if (mode == GL_COMPARE_REF_TO_TEXTURE) {
		switch (func) {
		case GL_GEQUAL:
			rval = MTLCompareFunctionGreaterEqual;
			break;
		case GL_LESS:
			rval = MTLCompareFunctionLess;
			break;
		case GL_GREATER:
			rval = MTLCompareFunctionGreater;
			break;
		case GL_EQUAL:
			rval = MTLCompareFunctionEqual;
			break;
		case GL_NOTEQUAL:
			rval = MTLCompareFunctionNotEqual;
			break;
		case GL_ALWAYS:
			rval = MTLCompareFunctionAlways;
			break;
		case GL_NEVER:
			rval = MTLCompareFunctionNever;
			break;
		case GL_LEQUAL: // GL default value
		default:
			rval = MTLCompareFunctionLessEqual;
			break;
		}
	}
	return rval;
}

SamplerMetal::SamplerMetal()
{
}

SamplerMetal::~SamplerMetal()
{
}

bool SamplerMetal::initialize(BackendContext* context)
{
	m_pSamplerDesc = [[MTLSamplerDescriptor alloc] init];
	m_dirty = false;
	return true;
}

bool SamplerMetal::terminate(BackendContext* context)
{
	m_pSamplerDesc = nil;
	m_samplerState = nil;
	m_dirty = true;
	return true;
}

bool SamplerMetal::setupSampler(BackendContext* context, const SamplerParameters& params)
{
	AXGL_ASSERT(context != nullptr);
	m_pSamplerDesc.sAddressMode = convert_address_mode(params.wrapS);
	m_pSamplerDesc.tAddressMode = convert_address_mode(params.wrapT);
	m_pSamplerDesc.rAddressMode = convert_address_mode(params.wrapR);
	m_pSamplerDesc.magFilter = convert_mag_filter(params.magFilter);
	m_pSamplerDesc.minFilter = convert_min_filter(params.minFilter);
	m_pSamplerDesc.mipFilter = convert_mip_filter(params.minFilter);
	m_pSamplerDesc.lodMinClamp = params.minLod;
	m_pSamplerDesc.lodMaxClamp = params.maxLod;
	m_pSamplerDesc.compareFunction = convert_compare_function(params.compareMode, params.compareFunc);

	ContextMetal* mtl_context = static_cast<ContextMetal*>(context);
	AXGL_ASSERT(mtl_context != nullptr);
	id<MTLDevice> device = mtl_context->getDevice();
	m_samplerState = [device newSamplerStateWithDescriptor:m_pSamplerDesc];
	// MTLSamplerStateを再生成したことからdirty
	m_dirty = true;

	return true;
}

id<MTLSamplerState> SamplerMetal::getMtlSamplerState() const
{
	return m_samplerState;
}

MTLSamplerDescriptor* SamplerMetal::getMtlSamplerDescriptor() const
{
	return m_pSamplerDesc;
}

bool SamplerMetal::isDirty() const
{
	return m_dirty;
}

void SamplerMetal::clearDirty()
{
	m_dirty = false;
	return;
}

} // namespace axgl
