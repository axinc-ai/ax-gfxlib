// BackendMetal.mm
#include "BackendMetal.h"
#include "../../common/axglCommon.h"
#include "../../common/axglDebug.h"

namespace axgl {

MTLPrimitiveType convert_primitive_type(int32_t mode)
{
	MTLPrimitiveType mtl_type = MTLPrimitiveTypePoint;
	switch (mode) {
	case GL_POINTS:
		break;
	case GL_LINE_STRIP:
		mtl_type = MTLPrimitiveTypeLineStrip;
		break;
	case GL_LINE_LOOP:
		AXGL_DBGOUT("GL_LINE_LOOP is not supported\n");
		mtl_type = MTLPrimitiveTypeLine;
		break;
	case GL_LINES:
		mtl_type = MTLPrimitiveTypeLine;
		break;
	case GL_TRIANGLE_STRIP:
		mtl_type = MTLPrimitiveTypeTriangleStrip;
		break;
	case GL_TRIANGLE_FAN:
		AXGL_DBGOUT("GL_TRIANGLE_FAN is not supported\n");
		mtl_type = MTLPrimitiveTypeTriangleStrip;
		break;
	case GL_TRIANGLES:
		mtl_type = MTLPrimitiveTypeTriangle;
		break;
	default:
		AXGL_ASSERT(0);
		break;
	}

	return mtl_type;
}

MTLCompareFunction convert_compare_func(int32_t func)
{
	MTLCompareFunction mtl_func = MTLCompareFunctionAlways;
	switch (func) {
	case GL_NEVER:
		mtl_func = MTLCompareFunctionNever;
		break;
	case GL_LESS:
		mtl_func = MTLCompareFunctionLess;
		break;
	case GL_EQUAL:
		mtl_func = MTLCompareFunctionEqual;
		break;
	case GL_LEQUAL:
		mtl_func = MTLCompareFunctionLessEqual;
		break;
	case GL_GREATER:
		mtl_func = MTLCompareFunctionGreater;
		break;
	case GL_NOTEQUAL:
		mtl_func = MTLCompareFunctionNotEqual;
		break;
	case GL_GEQUAL:
		mtl_func = MTLCompareFunctionGreaterEqual;
		break;
	case GL_ALWAYS:
		break;
	default:
		AXGL_ASSERT(0);
		break;
	}

	return mtl_func;
}

MTLCompareFunction convert_depth_stencil_compare_func(bool enable, int32_t func)
{
	MTLCompareFunction mtl_func = MTLCompareFunctionAlways;
	if (enable) {
		mtl_func = convert_compare_func(func);
	}
	return mtl_func;
}

BOOL convert_depth_write_mask(bool enable, int32_t writeMask)
{
	return (!enable || (writeMask == GL_FALSE)) ? NO : YES;
}

MTLStencilOperation convert_stencil_op(bool enable, int32_t op)
{
	MTLStencilOperation mtl_op = MTLStencilOperationKeep;
	if (enable) {
		switch (op) {
		case GL_KEEP:
			break;
		case GL_ZERO:
			mtl_op = MTLStencilOperationZero;
			break;
		case GL_REPLACE:
			mtl_op = MTLStencilOperationReplace;
			break;
		case GL_INCR:
			mtl_op = MTLStencilOperationIncrementClamp;
			break;
		case GL_INCR_WRAP:
			mtl_op = MTLStencilOperationIncrementWrap;
			break;
		case GL_DECR:
			mtl_op = MTLStencilOperationDecrementClamp;
			break;
		case GL_DECR_WRAP:
			mtl_op = MTLStencilOperationDecrementWrap;
			break;
		case GL_INVERT:
			mtl_op = MTLStencilOperationInvert;
			break;
		default:
			AXGL_ASSERT(0);
			break;
		}
	}

	return mtl_op;
}

uint32_t convert_stencil_mask(bool enable, uint32_t mask)
{
	return (enable) ? mask : 0;
}

MTLVertexFormat convert_vertex_format(int32_t type, int32_t size, bool normalized)
{
	MTLVertexFormat vertex_format = MTLVertexFormatInvalid;
	if (size == 1) {
		switch (type) {
		case GL_BYTE:
			vertex_format = (normalized) ? MTLVertexFormatCharNormalized : MTLVertexFormatChar;
			break;
		case GL_UNSIGNED_BYTE:
			vertex_format = (normalized) ? MTLVertexFormatUCharNormalized : MTLVertexFormatUChar;
			break;
		case GL_SHORT:
			vertex_format = (normalized) ? MTLVertexFormatShortNormalized : MTLVertexFormatShort;
			break;
		case GL_UNSIGNED_SHORT:
			vertex_format = (normalized) ? MTLVertexFormatUShortNormalized : MTLVertexFormatUShort;
			break;
		case GL_INT:
			vertex_format = MTLVertexFormatInt; // ignore normalized
			break;
		case GL_UNSIGNED_INT:
			vertex_format = MTLVertexFormatUInt; // ignore normalized
			break;
		case GL_HALF_FLOAT:
			vertex_format = MTLVertexFormatHalf; // ignore normalized
			break;
		case GL_FLOAT:
			vertex_format = MTLVertexFormatFloat; // ignore normalized
			break;
		case GL_FIXED:
			// Unsupported
			AXGL_ASSERT(0);
			break;
		case GL_INT_2_10_10_10_REV:
			// Incorrect: size==4 only
			AXGL_ASSERT(0);
			vertex_format = MTLVertexFormatInt1010102Normalized; // force normalized
			break;
		case GL_UNSIGNED_INT_2_10_10_10_REV:
			// Incorrect: size==4 only
			AXGL_ASSERT(0);
			vertex_format = MTLVertexFormatUInt1010102Normalized; // force normalized
			break;
		default:
			break;
		}
	} else if(size == 2) {
		switch (type) {
		case GL_BYTE:
			vertex_format = (normalized) ? MTLVertexFormatChar2Normalized : MTLVertexFormatChar2;
			break;
		case GL_UNSIGNED_BYTE:
			vertex_format = (normalized) ? MTLVertexFormatUChar2Normalized : MTLVertexFormatUChar2;
			break;
		case GL_SHORT:
			vertex_format = (normalized) ? MTLVertexFormatShort2Normalized : MTLVertexFormatShort2;
			break;
		case GL_UNSIGNED_SHORT:
			vertex_format = (normalized) ? MTLVertexFormatUShort2Normalized : MTLVertexFormatUShort2;
			break;
		case GL_INT:
			vertex_format = MTLVertexFormatInt2; // ignore normalized
			break;
		case GL_UNSIGNED_INT:
			vertex_format = MTLVertexFormatUInt2; // ignore normalized
			break;
		case GL_HALF_FLOAT:
			vertex_format = MTLVertexFormatHalf2; // ignore normalized
			break;
		case GL_FLOAT:
			vertex_format = MTLVertexFormatFloat2; // ignore normalized
			break;
		case GL_FIXED:
			// Unsupported
			AXGL_ASSERT(0);
			break;
		case GL_INT_2_10_10_10_REV:
			// Incorrect: size==4 only
			AXGL_ASSERT(0);
			vertex_format = MTLVertexFormatInt1010102Normalized; // force normalized
			break;
		case GL_UNSIGNED_INT_2_10_10_10_REV:
			// Incorrect: size==4 only
			AXGL_ASSERT(0);
			vertex_format = MTLVertexFormatUInt1010102Normalized; // force normalized
			break;
		default:
			break;
		}
	} else if(size == 3) {
		switch (type) {
		case GL_BYTE:
			vertex_format = (normalized) ? MTLVertexFormatChar3Normalized : MTLVertexFormatChar3;
			break;
		case GL_UNSIGNED_BYTE:
			vertex_format = (normalized) ? MTLVertexFormatUChar3Normalized : MTLVertexFormatUChar3;
			break;
		case GL_SHORT:
			vertex_format = (normalized) ? MTLVertexFormatShort3Normalized : MTLVertexFormatShort3;
			break;
		case GL_UNSIGNED_SHORT:
			vertex_format = (normalized) ? MTLVertexFormatUShort3Normalized : MTLVertexFormatUShort3;
			break;
		case GL_INT:
			vertex_format = MTLVertexFormatInt3; // ignore normalized
			break;
		case GL_UNSIGNED_INT:
			vertex_format = MTLVertexFormatUInt3; // ignore normalized
			break;
		case GL_HALF_FLOAT:
			vertex_format = MTLVertexFormatHalf3; // ignore normalized
			break;
		case GL_FLOAT:
			vertex_format = MTLVertexFormatFloat3; // ignore normalized
			break;
		case GL_FIXED:
			// Unsupported
			AXGL_ASSERT(0);
			break;
		case GL_INT_2_10_10_10_REV:
			// Incorrect: size==4 only
			AXGL_ASSERT(0);
			vertex_format = MTLVertexFormatInt1010102Normalized; // force normalized
			break;
		case GL_UNSIGNED_INT_2_10_10_10_REV:
			// Incorrect: size==4 only
			AXGL_ASSERT(0);
			vertex_format = MTLVertexFormatUInt1010102Normalized; // force normalized
			break;
		default:
			break;
		}
	} else if(size == 4) {
		switch (type) {
		case GL_BYTE:
			vertex_format = (normalized) ? MTLVertexFormatChar4Normalized : MTLVertexFormatChar4;
			break;
		case GL_UNSIGNED_BYTE:
			vertex_format = (normalized) ? MTLVertexFormatUChar4Normalized : MTLVertexFormatUChar4;
			break;
		case GL_SHORT:
			vertex_format = (normalized) ? MTLVertexFormatShort4Normalized : MTLVertexFormatShort4;
			break;
		case GL_UNSIGNED_SHORT:
			vertex_format = (normalized) ? MTLVertexFormatUShort4Normalized : MTLVertexFormatUShort4;
			break;
		case GL_INT:
			vertex_format = MTLVertexFormatInt4; // ignore normalized
			break;
		case GL_UNSIGNED_INT:
			vertex_format = MTLVertexFormatUInt4; // ignore normalized
			break;
		case GL_HALF_FLOAT:
			vertex_format = MTLVertexFormatHalf4; // ignore normalized
			break;
		case GL_FLOAT:
			vertex_format = MTLVertexFormatFloat4; // ignore normalized
			break;
		case GL_FIXED:
			// Unsupported
			AXGL_ASSERT(0);
			break;
		case GL_INT_2_10_10_10_REV:
			vertex_format = MTLVertexFormatInt1010102Normalized; // force normalized
			break;
		case GL_UNSIGNED_INT_2_10_10_10_REV:
			vertex_format = MTLVertexFormatUInt1010102Normalized; // force normalized
			break;
		default:
			break;
		}
	}

	return vertex_format;
}

MTLPixelFormat convert_internalformat(int32_t internalformat)
{
	MTLPixelFormat pixel_format = MTLPixelFormatInvalid;
	switch (internalformat) {
	case GL_R8:
		pixel_format = MTLPixelFormatR8Unorm;
		break;
	case GL_R8UI:
		pixel_format = MTLPixelFormatR8Uint;
		break;
	case GL_R8I:
		pixel_format = MTLPixelFormatR8Sint;
		break;
	case GL_R16UI:
		pixel_format = MTLPixelFormatR16Uint;
		break;
	case GL_R16I:
		pixel_format = MTLPixelFormatR16Sint;
		break;
	case GL_R32UI:
		pixel_format = MTLPixelFormatR32Uint;
		break;
	case GL_RG8:
		pixel_format = MTLPixelFormatRG8Unorm;
		break;
	case GL_RG8UI:
		pixel_format = MTLPixelFormatRG8Uint;
		break;
	case GL_RG8I:
		pixel_format = MTLPixelFormatRG8Sint;
		break;
	case GL_RG16UI:
		pixel_format = MTLPixelFormatRG16Uint;
		break;
	case GL_RG16I:
		pixel_format = MTLPixelFormatRG16Sint;
		break;
	case GL_RG32UI:
		pixel_format = MTLPixelFormatRG32Uint;
		break;
	case GL_RG32I:
		pixel_format = MTLPixelFormatRG32Sint;
		break;
	case GL_RGB8:
	case GL_RGB: // unsized internal format
		// RGBA8
		pixel_format = MTLPixelFormatRGBA8Unorm;
		break;
	case GL_SRGB8:
	case GL_SRGB:
		pixel_format = MTLPixelFormatRGBA8Unorm_sRGB;
		break;
	case GL_RGB8I:
		// RGBA8
		pixel_format = MTLPixelFormatRGBA8Sint;
		break;
	case GL_RGB8UI:
		// RGBA8
		pixel_format = MTLPixelFormatRGBA8Uint;
		break;
	case GL_RGB565:
		// RGBA8
		pixel_format = MTLPixelFormatRGBA8Unorm;
		break;
	case GL_RGBA8:
	case GL_RGBA: // unsized internal format
		pixel_format = MTLPixelFormatRGBA8Unorm;
		break;
	case GL_BGRA8_EXT:
		pixel_format = MTLPixelFormatBGRA8Unorm;
		break;
	case GL_SRGB8_ALPHA8:
		pixel_format = MTLPixelFormatRGBA8Unorm_sRGB;
		break;
	case GL_RGB5_A1:
		// RGBA8
		pixel_format = MTLPixelFormatRGBA8Unorm;
		break;
	case GL_RGBA4:
		// RGBA8
		pixel_format = MTLPixelFormatRGBA8Unorm;
		break;
	case GL_RGB10_A2:
		pixel_format = MTLPixelFormatRGB10A2Unorm;
		break;
	case GL_RGBA8UI:
		pixel_format = MTLPixelFormatRGBA8Uint;
		break;
	case GL_RGBA8I:
		pixel_format = MTLPixelFormatRGBA8Sint;
		break;
	case GL_RGB10_A2UI:
		pixel_format = MTLPixelFormatRGB10A2Uint;
		break;
	case GL_RGBA16UI:
		pixel_format = MTLPixelFormatRGBA16Uint;
		break;
	case GL_RGBA16I:
		pixel_format = MTLPixelFormatRGBA16Sint;
		break;
	case GL_RGBA32I:
		pixel_format = MTLPixelFormatRGBA32Sint;
		break;
	case GL_RGBA32UI:
		pixel_format = MTLPixelFormatRGBA32Uint;
		break;
	case GL_R16F:
		pixel_format = MTLPixelFormatR16Float;
		break;
	case GL_RG16F:
		pixel_format = MTLPixelFormatRG16Float;
		break;
	case GL_R32F:
		pixel_format = MTLPixelFormatR32Float;
		break;
	case GL_RGBA16F:
		pixel_format = MTLPixelFormatRGBA16Float;
		break;
	case GL_RGB16F:
		// NOTE: RGBA16F
		pixel_format = MTLPixelFormatRGBA16Float;
		break;
	case GL_RG32F:
		pixel_format = MTLPixelFormatRG32Float;
		break;
	case GL_RGBA32F:
		pixel_format = MTLPixelFormatRGBA32Float;
		break;
	case GL_RGB32F:
		// RGBA32F
		pixel_format = MTLPixelFormatRGBA32Float;
		break;
	case GL_DEPTH_COMPONENT16:
#if defined(__IPHONE_13_0) && (__IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_13_0)
		pixel_format = MTLPixelFormatDepth16Unorm;
#else
		pixel_format = MTLPixelFormatDepth32Float;
#endif
		break;
	case GL_DEPTH_COMPONENT24:
#if TARGET_OS_IPHONE
		pixel_format = MTLPixelFormatDepth32Float_Stencil8;
#else
		pixel_format = MTLPixelFormatDepth24Unorm_Stencil8;
#endif
		break;
	case GL_DEPTH_COMPONENT32F:
		pixel_format = MTLPixelFormatDepth32Float;
		break;
	case GL_DEPTH24_STENCIL8:
#if TARGET_OS_IPHONE
		pixel_format = MTLPixelFormatDepth32Float_Stencil8;
#else
		pixel_format = MTLPixelFormatDepth24Unorm_Stencil8;
#endif
		break;
	case GL_DEPTH32F_STENCIL8:
		pixel_format = MTLPixelFormatDepth32Float_Stencil8;
		break;
	case GL_STENCIL_INDEX8:
		pixel_format = MTLPixelFormatStencil8;
		break;
	// compressed texture formats
#if TARGET_OS_IPHONE
	case GL_COMPRESSED_R11_EAC:
		pixel_format = MTLPixelFormatEAC_R11Unorm;
		break;
	case GL_COMPRESSED_SIGNED_R11_EAC:
		pixel_format = MTLPixelFormatEAC_R11Snorm;
		break;
	case GL_COMPRESSED_RG11_EAC:
		pixel_format = MTLPixelFormatEAC_RG11Unorm;
		break;
	case GL_COMPRESSED_SIGNED_RG11_EAC:
		pixel_format = MTLPixelFormatEAC_RG11Snorm;
		break;
	case GL_COMPRESSED_RGB8_ETC2:
		pixel_format = MTLPixelFormatETC2_RGB8;
		break;
	case GL_COMPRESSED_SRGB8_ETC2:
		pixel_format = MTLPixelFormatETC2_RGB8_sRGB;
		break;
	case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
		pixel_format = MTLPixelFormatETC2_RGB8A1;
		break;
	case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
		pixel_format = MTLPixelFormatETC2_RGB8A1_sRGB;
		break;
	case GL_COMPRESSED_RGBA8_ETC2_EAC:
		pixel_format = MTLPixelFormatEAC_RGBA8;
		break;
	case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
		pixel_format = MTLPixelFormatEAC_RGBA8_sRGB;
		break;
#endif // TARGET_OS_IPHONE
	case 0:
		// 内部実装で使用している無効フォーマット値
		break;
	default:
		AXGL_DBGOUT("Unknown internalformat:0x%04X\n", internalformat);
		break;
	}

	return pixel_format;
}

MTLColorWriteMask convert_color_write_mask(bool red, bool green, bool blue, bool alpha)
{
	MTLColorWriteMask mask = MTLColorWriteMaskNone;
	if (red) {
		mask |= MTLColorWriteMaskRed;
	}
	if (green) {
		mask |= MTLColorWriteMaskGreen;
	}
	if (blue) {
		mask |= MTLColorWriteMaskBlue;
	}
	if (alpha) {
		mask |= MTLColorWriteMaskAlpha;
	}
	return mask;
}

MTLBlendOperation convert_blend_operation(int32_t eq)
{
	MTLBlendOperation operation = MTLBlendOperationAdd;
	switch (eq) {
	case GL_FUNC_ADD:
		break;
	case GL_FUNC_SUBTRACT:
		operation = MTLBlendOperationSubtract;
		break;
	case GL_FUNC_REVERSE_SUBTRACT:
		operation = MTLBlendOperationReverseSubtract;
		break;
	case GL_MIN:
		operation = MTLBlendOperationMin;
		break;
	case GL_MAX:
		operation = MTLBlendOperationMax;
		break;
	default:
		AXGL_DBGOUT("Unknown GL_BLEND_EQUATION 0x%04X¥n", eq);
		break;
	}
	return operation;
}

MTLBlendFactor convert_blend_factor(int32_t func)
{
	MTLBlendFactor factor = MTLBlendFactorOne;
	switch (func) {
	case GL_ZERO:
		factor = MTLBlendFactorZero;
		break;
	case GL_ONE:
		break;
	case GL_SRC_COLOR:
		factor = MTLBlendFactorSourceColor;
		break;
	case GL_ONE_MINUS_SRC_COLOR:
		factor = MTLBlendFactorOneMinusSourceColor;
		break;
	case GL_DST_COLOR:
		factor = MTLBlendFactorDestinationColor;
		break;
	case GL_ONE_MINUS_DST_COLOR:
		factor = MTLBlendFactorOneMinusDestinationColor;
		break;
	case GL_SRC_ALPHA:
		factor = MTLBlendFactorSourceAlpha;
		break;
	case GL_ONE_MINUS_SRC_ALPHA:
		factor = MTLBlendFactorOneMinusSourceAlpha;
		break;
	case GL_DST_ALPHA:
		factor = MTLBlendFactorDestinationAlpha;
		break;
	case GL_ONE_MINUS_DST_ALPHA:
		factor = MTLBlendFactorOneMinusDestinationAlpha;
		break;
	case GL_CONSTANT_COLOR:
		factor = MTLBlendFactorBlendColor;
		break;
	case GL_ONE_MINUS_CONSTANT_COLOR:
		factor = MTLBlendFactorOneMinusBlendColor;
		break;
	case GL_CONSTANT_ALPHA:
		factor = MTLBlendFactorBlendAlpha;
		break;
	case GL_ONE_MINUS_CONSTANT_ALPHA:
		factor = MTLBlendFactorOneMinusBlendAlpha;
		break;
	case GL_SRC_ALPHA_SATURATE:
		factor = MTLBlendFactorSourceAlphaSaturated;
		break;
	default:
		AXGL_DBGOUT("Unknown GL_BLEND_SRCorDST 0x%04X¥n", func);
		break;
	}
	return factor;
}

MTLIndexType convert_index_type(int32_t type)
{
	MTLIndexType mtl_type = MTLIndexTypeUInt16;
	switch (type) {
	case GL_UNSIGNED_BYTE:
		// Metalは非サポートだが、MTLIndexTypeUInt16に変換して描画する
		break;
	case GL_UNSIGNED_SHORT:
		break;
	case GL_UNSIGNED_INT:
		mtl_type = MTLIndexTypeUInt32;
		break;
	default:
		AXGL_ASSERT(0);
		break;
	}
	return mtl_type;
}

MTLWinding convert_winding(int32_t mode, bool flipY)
{
	MTLWinding mtl_winding = MTLWindingClockwise;
	if ((!flipY && (mode == GL_CCW)) || (flipY && (mode == GL_CW))) {
		mtl_winding = MTLWindingCounterClockwise;
	}
	return mtl_winding;
}

MTLCullMode convert_cull_mode(int32_t enable, int32_t mode)
{
	MTLCullMode cull_mode = MTLCullModeNone;
	if (enable == GL_TRUE) {
		switch (mode) {
		case GL_FRONT:
			cull_mode = MTLCullModeFront;
			break;
		case GL_BACK:
			cull_mode = MTLCullModeBack;
			break;
		case GL_FRONT_AND_BACK:
			AXGL_DBGOUT("GL_CULL_FACE_MODE is GL_FRONT_AND_BACK\n");
			cull_mode = MTLCullModeBack; // Backを返す
			break;
		default:
			break;
		}
	}
	return cull_mode;
}

MTLPixelFormat get_pixel_format(int32_t format, int32_t type)
{
	MTLPixelFormat pf = MTLPixelFormatRGBA8Unorm;

	if (format == GL_RED) {
		// MTLPixelFormatR
		switch (type) {
		case GL_UNSIGNED_BYTE:
			pf = MTLPixelFormatR8Unorm;
			break;
		case GL_BYTE:
			pf = MTLPixelFormatR8Snorm;
			break;
		case GL_UNSIGNED_SHORT:
			pf = MTLPixelFormatR16Unorm;
			break;
		case GL_SHORT:
			pf = MTLPixelFormatR16Snorm;
			break;
		case GL_UNSIGNED_INT:
			break;
		case GL_INT:
			break;
		case GL_HALF_FLOAT:
			pf = MTLPixelFormatR16Float;
			break;
		case GL_FLOAT:
			pf = MTLPixelFormatR32Float;
			break;
		case GL_UNSIGNED_SHORT_5_6_5:
			break;
		case GL_UNSIGNED_SHORT_4_4_4_4:
			break;
		case GL_UNSIGNED_SHORT_5_5_5_1:
			break;
		case GL_UNSIGNED_INT_2_10_10_10_REV:
			break;
		case GL_UNSIGNED_INT_10F_11F_11F_REV:
			break;
		case GL_UNSIGNED_INT_5_9_9_9_REV:
			break;
		case GL_UNSIGNED_INT_24_8:
			break;
		case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
			break;
		default:
			break;
		}
	} else if(format == GL_RED_INTEGER) {
		// MTLPixelFormatR
		switch (type) {
		case GL_UNSIGNED_BYTE:
			pf = MTLPixelFormatR8Uint;
			break;
		case GL_BYTE:
			pf = MTLPixelFormatR8Sint;
			break;
		case GL_UNSIGNED_SHORT:
			pf = MTLPixelFormatR16Uint;
			break;
		case GL_SHORT:
			pf = MTLPixelFormatR16Sint;
			break;
		case GL_UNSIGNED_INT:
			pf = MTLPixelFormatR32Uint;
			break;
		case GL_INT:
			pf = MTLPixelFormatR32Sint;
			break;
		case GL_HALF_FLOAT:
			break;
		case GL_FLOAT:
			break;
		case GL_UNSIGNED_SHORT_5_6_5:
			break;
		case GL_UNSIGNED_SHORT_4_4_4_4:
			break;
		case GL_UNSIGNED_SHORT_5_5_5_1:
			break;
		case GL_UNSIGNED_INT_2_10_10_10_REV:
			break;
		case GL_UNSIGNED_INT_10F_11F_11F_REV:
			break;
		case GL_UNSIGNED_INT_5_9_9_9_REV:
			break;
		case GL_UNSIGNED_INT_24_8:
			break;
		case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
			break;
		default:
			break;
		}
	} else if(format == GL_RG) {
		// MTLPixelFormatRG
		switch (type) {
		case GL_UNSIGNED_BYTE:
			pf = MTLPixelFormatRG8Unorm;
			break;
		case GL_BYTE:
			pf = MTLPixelFormatRG8Snorm;
			break;
		case GL_UNSIGNED_SHORT:
			pf = MTLPixelFormatRG16Unorm;
			break;
		case GL_SHORT:
			pf = MTLPixelFormatRG16Snorm;
			break;
		case GL_UNSIGNED_INT:
			break;
		case GL_INT:
			break;
		case GL_HALF_FLOAT:
			pf = MTLPixelFormatRG16Float;
			break;
		case GL_FLOAT:
			pf = MTLPixelFormatRG32Float;
			break;
		case GL_UNSIGNED_SHORT_5_6_5:
			break;
		case GL_UNSIGNED_SHORT_4_4_4_4:
			break;
		case GL_UNSIGNED_SHORT_5_5_5_1:
			break;
		case GL_UNSIGNED_INT_2_10_10_10_REV:
			break;
		case GL_UNSIGNED_INT_10F_11F_11F_REV:
			break;
		case GL_UNSIGNED_INT_5_9_9_9_REV:
			break;
		case GL_UNSIGNED_INT_24_8:
			break;
		case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
			break;
		default:
			break;
		}
	} else if(format == GL_RG_INTEGER) {
		// MTLPixelFormatRG
		switch (type) {
		case GL_UNSIGNED_BYTE:
			pf = MTLPixelFormatRG8Uint;
			break;
		case GL_BYTE:
			pf = MTLPixelFormatRG8Sint;
			break;
		case GL_UNSIGNED_SHORT:
			pf = MTLPixelFormatRG16Uint;
			break;
		case GL_SHORT:
			pf = MTLPixelFormatRG16Sint;
			break;
		case GL_UNSIGNED_INT:
			pf = MTLPixelFormatRG32Uint;
			break;
		case GL_INT:
			pf = MTLPixelFormatRG32Sint;
			break;
		case GL_HALF_FLOAT:
			break;
		case GL_FLOAT:
			break;
		case GL_UNSIGNED_SHORT_5_6_5:
			break;
		case GL_UNSIGNED_SHORT_4_4_4_4:
			break;
		case GL_UNSIGNED_SHORT_5_5_5_1:
			break;
		case GL_UNSIGNED_INT_2_10_10_10_REV:
			break;
		case GL_UNSIGNED_INT_10F_11F_11F_REV:
			break;
		case GL_UNSIGNED_INT_5_9_9_9_REV:
			break;
		case GL_UNSIGNED_INT_24_8:
			break;
		case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
			break;
		default:
			break;
		}
	} else if((format == GL_RGB) || (format == GL_RGBA) || (format == GL_LUMINANCE) || (format == GL_LUMINANCE_ALPHA)) {
		// MTLPixelFormatRGBA
		switch (type) {
		case GL_UNSIGNED_BYTE:
			pf = MTLPixelFormatRGBA8Unorm;
			break;
		case GL_BYTE:
			pf = MTLPixelFormatRGBA8Snorm;
			break;
		case GL_UNSIGNED_SHORT:
			pf = MTLPixelFormatRGBA16Unorm;
			break;
		case GL_SHORT:
			pf = MTLPixelFormatRGBA16Snorm;
			break;
		case GL_UNSIGNED_INT:
			break;
		case GL_INT:
			break;
		case GL_HALF_FLOAT:
			pf = MTLPixelFormatRGBA16Float;
			break;
		case GL_FLOAT:
			pf = MTLPixelFormatRGBA32Float;
			break;
		case GL_UNSIGNED_SHORT_5_6_5:
			break;
		case GL_UNSIGNED_SHORT_4_4_4_4:
			break;
		case GL_UNSIGNED_SHORT_5_5_5_1:
			break;
		case GL_UNSIGNED_INT_2_10_10_10_REV:
			break;
		case GL_UNSIGNED_INT_10F_11F_11F_REV:
			break;
		case GL_UNSIGNED_INT_5_9_9_9_REV:
			break;
		case GL_UNSIGNED_INT_24_8:
			break;
		case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
			break;
		default:
			break;
		}
	} else if((format == GL_RGB_INTEGER) || (format == GL_RGBA_INTEGER)) {
		// MTLPixelFormatRG
		switch (type) {
		case GL_UNSIGNED_BYTE:
			pf = MTLPixelFormatRGBA8Uint;
			break;
		case GL_BYTE:
			pf = MTLPixelFormatRGBA8Sint;
			break;
		case GL_UNSIGNED_SHORT:
			pf = MTLPixelFormatRGBA16Uint;
			break;
		case GL_SHORT:
			pf = MTLPixelFormatRGBA16Sint;
			break;
		case GL_UNSIGNED_INT:
			pf = MTLPixelFormatRGBA32Uint;
			break;
		case GL_INT:
			pf = MTLPixelFormatRGBA32Sint;
			break;
		case GL_HALF_FLOAT:
			break;
		case GL_FLOAT:
			break;
		case GL_UNSIGNED_SHORT_5_6_5:
			break;
		case GL_UNSIGNED_SHORT_4_4_4_4:
			break;
		case GL_UNSIGNED_SHORT_5_5_5_1:
			break;
		case GL_UNSIGNED_INT_2_10_10_10_REV:
			break;
		case GL_UNSIGNED_INT_10F_11F_11F_REV:
			break;
		case GL_UNSIGNED_INT_5_9_9_9_REV:
			break;
		case GL_UNSIGNED_INT_24_8:
			break;
		case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
			break;
		default:
			break;
		}
	} else if(format == GL_DEPTH_COMPONENT) {
		// MTLPixelFormatDepth
		switch (type) {
		case GL_UNSIGNED_BYTE:
			break;
		case GL_BYTE:
			break;
		case GL_UNSIGNED_SHORT:
#if defined(__IPHONE_13_0) && (__IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_13_0)
			pf = MTLPixelFormatDepth16Unorm;
#else
			pf = MTLPixelFormatDepth32Float;
#endif
			break;
		case GL_SHORT:
			break;
		case GL_UNSIGNED_INT:
			break;
		case GL_INT:
			break;
		case GL_HALF_FLOAT:
			break;
		case GL_FLOAT:
			pf = MTLPixelFormatDepth32Float;
			break;
		case GL_UNSIGNED_SHORT_5_6_5:
			break;
		case GL_UNSIGNED_SHORT_4_4_4_4:
			break;
		case GL_UNSIGNED_SHORT_5_5_5_1:
			break;
		case GL_UNSIGNED_INT_2_10_10_10_REV:
			break;
		case GL_UNSIGNED_INT_10F_11F_11F_REV:
			break;
		case GL_UNSIGNED_INT_5_9_9_9_REV:
			break;
		case GL_UNSIGNED_INT_24_8:
			break;
		case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
			break;
		default:
			break;
		}
	} else if(format == GL_DEPTH_STENCIL) {
		// MTLPixelFormatDepth_Stencil
		switch (type) {
		case GL_UNSIGNED_BYTE:
			break;
		case GL_BYTE:
			break;
		case GL_UNSIGNED_SHORT:
			break;
		case GL_SHORT:
			break;
		case GL_UNSIGNED_INT:
			break;
		case GL_INT:
			break;
		case GL_HALF_FLOAT:
			break;
		case GL_FLOAT:
			break;
		case GL_UNSIGNED_SHORT_5_6_5:
			break;
		case GL_UNSIGNED_SHORT_4_4_4_4:
			break;
		case GL_UNSIGNED_SHORT_5_5_5_1:
			break;
		case GL_UNSIGNED_INT_2_10_10_10_REV:
			break;
		case GL_UNSIGNED_INT_10F_11F_11F_REV:
			break;
		case GL_UNSIGNED_INT_5_9_9_9_REV:
			break;
		case GL_UNSIGNED_INT_24_8:
#ifdef TARGET_OS_IPHONE
			pf = MTLPixelFormatDepth32Float_Stencil8;
#else
			pf = MTLPixelFormatDepth24Unorm_Stencil8;
#endif
			break;
		case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
			pf = MTLPixelFormatDepth32Float_Stencil8;
			break;
		default:
			break;
		}
	} else if(format == GL_ALPHA) {
		// MTLPixelFormatA
		switch (type) {
		case GL_UNSIGNED_BYTE:
			pf = MTLPixelFormatA8Unorm;
			break;
		case GL_BYTE:
			break;
		case GL_UNSIGNED_SHORT:
			break;
		case GL_SHORT:
			break;
		case GL_UNSIGNED_INT:
			break;
		case GL_INT:
			break;
		case GL_HALF_FLOAT:
			break;
		case GL_FLOAT:
			break;
		case GL_UNSIGNED_SHORT_5_6_5:
			break;
		case GL_UNSIGNED_SHORT_4_4_4_4:
			break;
		case GL_UNSIGNED_SHORT_5_5_5_1:
			break;
		case GL_UNSIGNED_INT_2_10_10_10_REV:
			break;
		case GL_UNSIGNED_INT_10F_11F_11F_REV:
			break;
		case GL_UNSIGNED_INT_5_9_9_9_REV:
			break;
		case GL_UNSIGNED_INT_24_8:
			break;
		case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
			break;
		default:
			break;
		}
	}

	return pf;
}

NSUInteger get_bytes_per_pixel(int32_t format, int32_t type)
{
	NSUInteger bytes_per_pixel = 0;

	if (format == GL_RED) {
		switch (type) {
		case GL_UNSIGNED_BYTE:
		case GL_BYTE:
			bytes_per_pixel = 1;
			break;
		case GL_UNSIGNED_SHORT:
		case GL_SHORT:
		case GL_HALF_FLOAT:
			bytes_per_pixel = 2;
			break;
		case GL_FLOAT:
			bytes_per_pixel = 4;
			break;
		case GL_UNSIGNED_INT:
		case GL_INT:
		case GL_UNSIGNED_SHORT_5_6_5:
		case GL_UNSIGNED_SHORT_4_4_4_4:
		case GL_UNSIGNED_SHORT_5_5_5_1:
		case GL_UNSIGNED_INT_2_10_10_10_REV:
		case GL_UNSIGNED_INT_10F_11F_11F_REV:
		case GL_UNSIGNED_INT_5_9_9_9_REV:
		case GL_UNSIGNED_INT_24_8:
		case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
			AXGL_ASSERT(0);
			break;
		default:
			break;
		}
	} else if(format == GL_RED_INTEGER) {
		switch (type) {
		case GL_UNSIGNED_BYTE:
		case GL_BYTE:
			bytes_per_pixel = 1;
			break;
		case GL_UNSIGNED_SHORT:
		case GL_SHORT:
			bytes_per_pixel = 2;
			break;
		case GL_UNSIGNED_INT:
		case GL_INT:
			bytes_per_pixel = 4;
			break;
		case GL_HALF_FLOAT:
		case GL_FLOAT:
		case GL_UNSIGNED_SHORT_5_6_5:
		case GL_UNSIGNED_SHORT_4_4_4_4:
		case GL_UNSIGNED_SHORT_5_5_5_1:
		case GL_UNSIGNED_INT_2_10_10_10_REV:
		case GL_UNSIGNED_INT_10F_11F_11F_REV:
		case GL_UNSIGNED_INT_5_9_9_9_REV:
		case GL_UNSIGNED_INT_24_8:
		case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
			AXGL_ASSERT(0);
			break;
		default:
			break;
		}
	} else if(format == GL_RG) {
		switch (type) {
		case GL_UNSIGNED_BYTE:
		case GL_BYTE:
			bytes_per_pixel = 2;
			break;
		case GL_UNSIGNED_SHORT:
		case GL_SHORT:
			bytes_per_pixel = 4;
			break;
		case GL_HALF_FLOAT:
		case GL_FLOAT:
			bytes_per_pixel = 8;
			break;
		case GL_UNSIGNED_INT:
		case GL_INT:
		case GL_UNSIGNED_SHORT_5_6_5:
		case GL_UNSIGNED_SHORT_4_4_4_4:
		case GL_UNSIGNED_SHORT_5_5_5_1:
		case GL_UNSIGNED_INT_2_10_10_10_REV:
		case GL_UNSIGNED_INT_10F_11F_11F_REV:
		case GL_UNSIGNED_INT_5_9_9_9_REV:
		case GL_UNSIGNED_INT_24_8:
		case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
			AXGL_ASSERT(0);
			break;
		default:
			break;
		}
	} else if(format == GL_RG_INTEGER) {
		switch (type) {
		case GL_UNSIGNED_BYTE:
		case GL_BYTE:
			bytes_per_pixel = 2;
			break;
		case GL_UNSIGNED_SHORT:
		case GL_SHORT:
			bytes_per_pixel = 4;
			break;
		case GL_UNSIGNED_INT:
		case GL_INT:
			bytes_per_pixel = 8;
			break;
		case GL_HALF_FLOAT:
		case GL_FLOAT:
		case GL_UNSIGNED_SHORT_5_6_5:
		case GL_UNSIGNED_SHORT_4_4_4_4:
		case GL_UNSIGNED_SHORT_5_5_5_1:
		case GL_UNSIGNED_INT_2_10_10_10_REV:
		case GL_UNSIGNED_INT_10F_11F_11F_REV:
		case GL_UNSIGNED_INT_5_9_9_9_REV:
		case GL_UNSIGNED_INT_24_8:
		case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
			AXGL_ASSERT(0);
			break;
		default:
			break;
		}
	} else if(format == GL_RGB){
		switch (type) {
		case GL_UNSIGNED_BYTE:
			bytes_per_pixel = 4; // 3バイトは無理
			break;
		case GL_BYTE:
		case GL_UNSIGNED_SHORT:
		case GL_SHORT:
		case GL_UNSIGNED_INT:
		case GL_INT:
		case GL_HALF_FLOAT:
		case GL_FLOAT:
		case GL_UNSIGNED_SHORT_5_6_5:
		case GL_UNSIGNED_SHORT_4_4_4_4:
		case GL_UNSIGNED_SHORT_5_5_5_1:
		case GL_UNSIGNED_INT_2_10_10_10_REV:
		case GL_UNSIGNED_INT_10F_11F_11F_REV:
		case GL_UNSIGNED_INT_5_9_9_9_REV:
		case GL_UNSIGNED_INT_24_8:
		case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
			AXGL_ASSERT(0);
			break;
		default:
			break;
		}
	} else if(format == GL_RGBA) {
		switch (type) {
		case GL_UNSIGNED_BYTE:
		case GL_BYTE:
			bytes_per_pixel = 4;
			break;
		case GL_UNSIGNED_SHORT:
		case GL_SHORT:
		case GL_HALF_FLOAT:
			bytes_per_pixel = 8;
			break;
		case GL_FLOAT:
			bytes_per_pixel = 16;
			break;
		case GL_UNSIGNED_INT:
		case GL_INT:
		case GL_UNSIGNED_SHORT_5_6_5:
		case GL_UNSIGNED_SHORT_4_4_4_4:
		case GL_UNSIGNED_SHORT_5_5_5_1:
		case GL_UNSIGNED_INT_2_10_10_10_REV:
		case GL_UNSIGNED_INT_10F_11F_11F_REV:
		case GL_UNSIGNED_INT_5_9_9_9_REV:
		case GL_UNSIGNED_INT_24_8:
		case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
			AXGL_ASSERT(0);
			break;
		default:
			break;
		}
	} else if(format == GL_RGBA_INTEGER) {
		switch (type) {
		case GL_UNSIGNED_BYTE:
		case GL_BYTE:
			bytes_per_pixel = 4;
			break;
		case GL_UNSIGNED_SHORT:
		case GL_SHORT:
			bytes_per_pixel = 8;
			break;
		case GL_UNSIGNED_INT:
		case GL_INT:
			bytes_per_pixel = 16;
			break;
		case GL_HALF_FLOAT:
		case GL_FLOAT:
		case GL_UNSIGNED_SHORT_5_6_5:
		case GL_UNSIGNED_SHORT_4_4_4_4:
		case GL_UNSIGNED_SHORT_5_5_5_1:
		case GL_UNSIGNED_INT_2_10_10_10_REV:
		case GL_UNSIGNED_INT_10F_11F_11F_REV:
		case GL_UNSIGNED_INT_5_9_9_9_REV:
		case GL_UNSIGNED_INT_24_8:
		case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
			AXGL_ASSERT(0);
			break;
		default:
			break;
		}
	} else if(format == GL_DEPTH_COMPONENT) {
		switch (type) {
		case GL_UNSIGNED_SHORT:
			bytes_per_pixel = 2;
			break;
		case GL_FLOAT:
			bytes_per_pixel = 4;
			break;
		case GL_UNSIGNED_BYTE:
		case GL_BYTE:
		case GL_SHORT:
		case GL_UNSIGNED_INT:
		case GL_INT:
		case GL_HALF_FLOAT:
		case GL_UNSIGNED_SHORT_5_6_5:
		case GL_UNSIGNED_SHORT_4_4_4_4:
		case GL_UNSIGNED_SHORT_5_5_5_1:
		case GL_UNSIGNED_INT_2_10_10_10_REV:
		case GL_UNSIGNED_INT_10F_11F_11F_REV:
		case GL_UNSIGNED_INT_5_9_9_9_REV:
		case GL_UNSIGNED_INT_24_8:
		case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
			AXGL_ASSERT(0);
			break;
		default:
			break;
		}
	} else if(format == GL_DEPTH_STENCIL) {
		// MTLPixelFormatDepth_Stencil
		switch (type) {
		case GL_UNSIGNED_INT_24_8:
			bytes_per_pixel = 4;
			break;
		case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
			bytes_per_pixel = 8;
			break;
		case GL_UNSIGNED_BYTE:
		case GL_BYTE:
		case GL_UNSIGNED_SHORT:
		case GL_SHORT:
		case GL_UNSIGNED_INT:
		case GL_INT:
		case GL_HALF_FLOAT:
		case GL_FLOAT:
		case GL_UNSIGNED_SHORT_5_6_5:
		case GL_UNSIGNED_SHORT_4_4_4_4:
		case GL_UNSIGNED_SHORT_5_5_5_1:
		case GL_UNSIGNED_INT_2_10_10_10_REV:
		case GL_UNSIGNED_INT_10F_11F_11F_REV:
		case GL_UNSIGNED_INT_5_9_9_9_REV:
			AXGL_ASSERT(0);
			break;
		default:
			break;
		}
	} else if(format == GL_ALPHA) {
		// MTLPixelFormatA
		switch (type) {
		case GL_UNSIGNED_BYTE:
			bytes_per_pixel = 1;
			break;
		case GL_BYTE:
		case GL_UNSIGNED_SHORT:
		case GL_SHORT:
		case GL_UNSIGNED_INT:
		case GL_INT:
		case GL_HALF_FLOAT:
		case GL_FLOAT:
		case GL_UNSIGNED_SHORT_5_6_5:
		case GL_UNSIGNED_SHORT_4_4_4_4:
		case GL_UNSIGNED_SHORT_5_5_5_1:
		case GL_UNSIGNED_INT_2_10_10_10_REV:
		case GL_UNSIGNED_INT_10F_11F_11F_REV:
		case GL_UNSIGNED_INT_5_9_9_9_REV:
		case GL_UNSIGNED_INT_24_8:
		case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
			AXGL_ASSERT(0);
			break;
		default:
			break;
		}
	} else {
		// GL_RGB
		// GL_LUMINANCE
		// GL_LUMINANCE_ALPHA
		// GL_RGB_INTEGER
		AXGL_ASSERT(0);
	}

	if (bytes_per_pixel == 0) {
		AXGL_DBGOUT("format:0x%04x type:0x%04x\n", format, type);
	}
	return bytes_per_pixel;
}

static uint32_t calc_etc2_block_count(int32_t width)
{
	uint32_t cnt = (width / 4);
	if ((width & 0x3) != 0) {
		cnt++;
	}
	return cnt;
}

NSUInteger get_bytes_per_row_blocks(int32_t format, int32_t width)
{
	// NOTE: compressed format only
	NSUInteger bytes_per_row_blocks = 0;

	switch (format) {
#if TARGET_OS_IPHONE
	case GL_COMPRESSED_R11_EAC:
	case GL_COMPRESSED_SIGNED_R11_EAC:
	case GL_COMPRESSED_RGB8_ETC2:
	case GL_COMPRESSED_SRGB8_ETC2:
	case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
	case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
		// 4x4 block : 64 bit
		bytes_per_row_blocks = calc_etc2_block_count(width) * 8;
		break;
	case GL_COMPRESSED_RG11_EAC:
	case GL_COMPRESSED_SIGNED_RG11_EAC:
	case GL_COMPRESSED_RGBA8_ETC2_EAC:
	case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
		// 4x4 block : 128 bit
		bytes_per_row_blocks = calc_etc2_block_count(width) * 16;
		break;
#else
	case 0x83f1: //GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
		// DXT1 : same size as RGB8_ETC2
		bytes_per_row_blocks = calc_etc2_block_count(width) * 8;
		break;
#endif // TARGET_OS_IPHONE
	default:
		AXGL_DBGOUT("get_bytes_per_row_blocks> Unknown format:0x%04X\n", format);
		break;
	}

	return bytes_per_row_blocks;
}

int32_t get_shader_data_type(MTLDataType type)
{
	int32_t gl_type = 0;

	switch (type) {
	case MTLDataTypeNone:
		break;
	case MTLDataTypeStruct:
		gl_type = -1; // struct
		break;
	case MTLDataTypeArray:
		gl_type = -2; // array
		break;
	case MTLDataTypeFloat: // highp
	case MTLDataTypeHalf: // mediump
		gl_type = GL_FLOAT;
		break;
	case MTLDataTypeFloat2: // highp
	case MTLDataTypeHalf2: // mediump
		gl_type = GL_FLOAT_VEC2;
		break;
	case MTLDataTypeFloat3: // highp
	case MTLDataTypeHalf3: // mediump
		gl_type = GL_FLOAT_VEC3;
		break;
	case MTLDataTypeFloat4: // highp
	case MTLDataTypeHalf4: // mediump
		gl_type = GL_FLOAT_VEC4;
		break;
	case MTLDataTypeFloat2x2: // highp
	case MTLDataTypeHalf2x2: // mediump
		gl_type = GL_FLOAT_MAT2;
		break;
	case MTLDataTypeFloat2x3: // highp
	case MTLDataTypeHalf2x3: // mediump
		gl_type = GL_FLOAT_MAT2x3;
		break;
	case MTLDataTypeFloat2x4: // highp
	case MTLDataTypeHalf2x4: // mediump
		gl_type = GL_FLOAT_MAT2x4;
		break;
	case MTLDataTypeFloat3x2: // highp
	case MTLDataTypeHalf3x2: // mediump
		gl_type = GL_FLOAT_MAT3x2;
		break;
	case MTLDataTypeFloat3x3: // highp
	case MTLDataTypeHalf3x3: // mediump
		gl_type = GL_FLOAT_MAT3;
		break;
	case MTLDataTypeFloat3x4: // highp
	case MTLDataTypeHalf3x4: // mediump
		gl_type = GL_FLOAT_MAT3x4;
		break;
	case MTLDataTypeFloat4x2: // highp
	case MTLDataTypeHalf4x2: // mediump
		gl_type = GL_FLOAT_MAT4x2;
		break;
	case MTLDataTypeFloat4x3: // highp
	case MTLDataTypeHalf4x3: // mediump
		gl_type = GL_FLOAT_MAT4x3;
		break;
	case MTLDataTypeFloat4x4: // highp
	case MTLDataTypeHalf4x4: // mediump
		gl_type = GL_FLOAT_MAT4;
		break;
	case MTLDataTypeInt:
	case MTLDataTypeShort:
	case MTLDataTypeChar:
		gl_type = GL_INT;
		break;
	case MTLDataTypeInt2:
	case MTLDataTypeShort2:
	case MTLDataTypeChar2:
		gl_type = GL_INT_VEC2;
		break;
	case MTLDataTypeInt3:
	case MTLDataTypeShort3:
	case MTLDataTypeChar3:
		gl_type = GL_INT_VEC3;
		break;
	case MTLDataTypeInt4:
	case MTLDataTypeShort4:
	case MTLDataTypeChar4:
		gl_type = GL_INT_VEC4;
		break;
	case MTLDataTypeUInt:
	case MTLDataTypeUShort:
	case MTLDataTypeUChar:
		gl_type = GL_UNSIGNED_INT;
		break;
	case MTLDataTypeUInt2:
	case MTLDataTypeUShort2:
	case MTLDataTypeUChar2:
		gl_type = GL_UNSIGNED_INT_VEC2;
		break;
	case MTLDataTypeUInt3:
	case MTLDataTypeUShort3:
	case MTLDataTypeUChar3:
		gl_type = GL_UNSIGNED_INT_VEC3;
		break;
	case MTLDataTypeUInt4:
	case MTLDataTypeUShort4:
	case MTLDataTypeUChar4:
		gl_type = GL_UNSIGNED_INT_VEC4;
		break;
	case MTLDataTypeBool:
		gl_type = GL_BOOL;
		break;
	case MTLDataTypeBool2:
		gl_type = GL_BOOL_VEC2;
		break;
	case MTLDataTypeBool3:
		gl_type = GL_BOOL_VEC3;
		break;
	case MTLDataTypeBool4:
		gl_type = GL_BOOL_VEC4;
		break;
	case MTLDataTypeTexture:
	case MTLDataTypeSampler:
		gl_type = GL_SAMPLER_2D; // XXX 3D, CUBE, etc...
		break;
	default:
		AXGL_DBGOUT("Unexpected type:%d\n", type);
		break;
	}

	return gl_type;
}

uint32_t get_size_from_gltype(int32_t gltype)
{
	uint32_t size = 0;

	switch (gltype) {
	case GL_FLOAT:
		size = sizeof(float);
		break;
	case GL_FLOAT_VEC2:
		size = sizeof(float) * 2;
		break;
	case GL_FLOAT_VEC3:
		size = sizeof(float) * 3;
		break;
	case GL_FLOAT_VEC4:
		size = sizeof(float) * 4;
		break;
	case GL_INT:
		size = sizeof(int32_t);
		break;
	case GL_INT_VEC2:
		size = sizeof(int32_t) * 2;
		break;
	case GL_INT_VEC3:
		size = sizeof(int32_t) * 3;
		break;
	case GL_INT_VEC4:
		size = sizeof(int32_t) * 4;
		break;
	case GL_UNSIGNED_INT:
		size = sizeof(uint32_t);
		break;
	case GL_UNSIGNED_INT_VEC2:
		size = sizeof(uint32_t) * 2;
		break;
	case GL_UNSIGNED_INT_VEC3:
		size = sizeof(uint32_t) * 3;
		break;
	case GL_UNSIGNED_INT_VEC4:
		size = sizeof(uint32_t) * 4;
		break;
	case GL_BOOL:
		size = sizeof(int32_t);
		break;
	case GL_BOOL_VEC2:
		size = sizeof(int32_t) * 2;
		break;
	case GL_BOOL_VEC3:
		size = sizeof(int32_t) * 3;
		break;
	case GL_BOOL_VEC4:
		size = sizeof(int32_t) * 4;
		break;
	case GL_FLOAT_MAT2:
		size = sizeof(float) * 2 * 2;
		break;
	case GL_FLOAT_MAT3:
		size = sizeof(float) * 3 * 3;
		break;
	case GL_FLOAT_MAT4:
		size = sizeof(float) * 4 * 4;
		break;
	case GL_FLOAT_MAT2x3:
		size = sizeof(float) * 2 * 3;
		break;
	case GL_FLOAT_MAT2x4:
		size = sizeof(float) * 2 * 4;
		break;
	case GL_FLOAT_MAT3x2:
		size = sizeof(float) * 3 * 2;
		break;
	case GL_FLOAT_MAT3x4:
		size = sizeof(float) * 3 * 4;
		break;
	case GL_FLOAT_MAT4x2:
		size = sizeof(float) * 4 * 2;
		break;
	case GL_FLOAT_MAT4x3:
		size = sizeof(float) * 4 * 3;
		break;
	default:
		AXGL_ASSERT(0);
		break;
	}

	return size;
}

int32_t get_internalformat(MTLPixelFormat pixelFormat)
{
	int32_t gl_format = 0;

	switch (pixelFormat) {
	// color
	case MTLPixelFormatR8Unorm:
		gl_format = GL_R8;
		break;
	case MTLPixelFormatR8Uint:
		gl_format = GL_R8UI;
		break;
	case MTLPixelFormatR8Sint:
		gl_format = GL_R8I;
		break;
	case MTLPixelFormatR16Uint:
		gl_format = GL_R16UI;
		break;
	case MTLPixelFormatR16Sint:
		gl_format = GL_R16I;
		break;
	case MTLPixelFormatR16Float:
		gl_format = GL_R16F;
		break;
	case MTLPixelFormatR32Uint:
		gl_format = GL_R32UI;
		break;
	case MTLPixelFormatR32Sint:
		gl_format = GL_R32I;
		break;
	case MTLPixelFormatR32Float:
		gl_format = GL_R32F;
		break;
	case MTLPixelFormatRG8Unorm:
		gl_format = GL_RG8;
		break;
	case MTLPixelFormatRG8Uint:
		gl_format = GL_RG8UI;
		break;
	case MTLPixelFormatRG8Sint:
		gl_format = GL_RG8I;
		break;
	case MTLPixelFormatRG16Uint:
		gl_format = GL_RG16UI;
		break;
	case MTLPixelFormatRG16Sint:
		gl_format = GL_RG16I;
		break;
	case MTLPixelFormatRG16Float:
		gl_format = GL_RG16F;
		break;
	case MTLPixelFormatRG32Uint:
		gl_format = GL_RG32UI;
		break;
	case MTLPixelFormatRG32Sint:
		gl_format = GL_RG32I;
		break;
	case MTLPixelFormatRG32Float:
		gl_format = GL_RG32F;
		break;
#if TARGET_OS_IPHONE
	case MTLPixelFormatB5G6R5Unorm:
		gl_format = GL_RGB565;
		break;
#endif
	case MTLPixelFormatRGBA8Unorm:
		gl_format = GL_RGBA8;
		break;
	case MTLPixelFormatBGRA8Unorm:
		gl_format = GL_BGRA8_EXT;
		break;
	case MTLPixelFormatRGBA8Unorm_sRGB:
	case MTLPixelFormatBGRA8Unorm_sRGB:
		gl_format = GL_SRGB8_ALPHA8;
		break;
#if TARGET_OS_IPHONE
	case MTLPixelFormatBGR5A1Unorm:
	case MTLPixelFormatA1BGR5Unorm:
		gl_format = GL_RGB5_A1;
		break;
	case MTLPixelFormatABGR4Unorm:
		gl_format = GL_RGBA4;
		break;
#endif
	case MTLPixelFormatBGR10A2Unorm:
	case MTLPixelFormatRGB10A2Unorm:
		gl_format = GL_RGB10_A2;
		break;
	case MTLPixelFormatRGBA8Uint:
		gl_format = GL_RGBA8UI;
		break;
	case MTLPixelFormatRGBA8Sint:
		gl_format = GL_RGBA8I;
		break;
	case MTLPixelFormatRGBA16Uint:
		gl_format = GL_RGBA16UI;
		break;
	case MTLPixelFormatRGBA16Sint:
		gl_format = GL_RGBA16I;
		break;
	case MTLPixelFormatRGBA16Float:
		gl_format = GL_RGBA16F;
		break;
	case MTLPixelFormatRGBA32Uint:
		gl_format = GL_RGBA32UI;
		break;
	case MTLPixelFormatRGBA32Sint:
		gl_format = GL_RGBA32I;
		break;
	case MTLPixelFormatRGBA32Float:
		gl_format = GL_RGBA32F;
		break;
	// depth, stencil
#if defined(__IPHONE_13_0) && (__IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_13_0)
	case MTLPixelFormatDepth16Unorm:
		gl_format = GL_DEPTH_COMPONENT16;
		break;
#endif
	case MTLPixelFormatDepth32Float:
		gl_format = GL_DEPTH_COMPONENT32F;
		break;
	case MTLPixelFormatDepth32Float_Stencil8:
		gl_format = GL_DEPTH32F_STENCIL8;
		break;
#if (TARGET_OS_IPHONE == 0)
	case MTLPixelFormatDepth24Unorm_Stencil8:
		gl_format = GL_DEPTH24_STENCIL8;
		break;
#endif
	case MTLPixelFormatStencil8:
		gl_format = GL_STENCIL_INDEX8;
		break;
	default:
		AXGL_DBGOUT("unsupported MTLFormat:%d\n", (int)pixelFormat);
		break;
	}

	return gl_format;
}

void get_shader_constant_copy_params(int32_t gltype, uint32_t* size, uint32_t* stride, uint32_t* count)
{
	// buffer row : float4
	AXGL_ASSERT((size != nullptr) && (stride != nullptr) && (count != nullptr));

	uint32_t sz = 0;
	uint32_t st = 0;
	uint32_t ct = 0;
	switch (gltype) {
	case GL_FLOAT:
		sz = sizeof(float);
		st = sizeof(float);
		ct = 1;
		break;
	case GL_FLOAT_VEC2:
		sz = sizeof(float) * 2;
		st = sizeof(float) * 2;
		ct = 1;
		break;
	case GL_FLOAT_VEC3:
		sz = sizeof(float) * 3;
		st = sizeof(float) * 4;
		ct = 1;
		break;
	case GL_FLOAT_VEC4:
		sz = sizeof(float) * 4;
		st = sizeof(float) * 4;
		ct = 1;
		break;
	case GL_INT:
		sz = sizeof(int32_t);
		st = sizeof(int32_t);
		ct = 1;
		break;
	case GL_INT_VEC2:
		sz = sizeof(int32_t) * 2;
		st = sizeof(int32_t) * 2;
		ct = 1;
		break;
	case GL_INT_VEC3:
		sz = sizeof(int32_t) * 3;
		st = sizeof(int32_t) * 4;
		ct = 1;
		break;
	case GL_INT_VEC4:
		sz = sizeof(int32_t) * 4;
		st = sizeof(int32_t) * 4;
		ct = 1;
		break;
	case GL_UNSIGNED_INT:
		sz = sizeof(uint32_t);
		st = sizeof(uint32_t);
		ct = 1;
		break;
	case GL_UNSIGNED_INT_VEC2:
		sz = sizeof(uint32_t) * 2;
		st = sizeof(uint32_t) * 2;
		ct = 1;
		break;
	case GL_UNSIGNED_INT_VEC3:
		sz = sizeof(uint32_t) * 3;
		st = sizeof(uint32_t) * 4;
		ct = 1;
		break;
	case GL_UNSIGNED_INT_VEC4:
		sz = sizeof(uint32_t) * 4;
		st = sizeof(uint32_t) * 4;
		ct = 1;
		break;
	case GL_BOOL:
		AXGL_DBGOUT("bool is used in uniform\n");
		sz = sizeof(uint8_t);
		st = sizeof(uint8_t);
		ct = 1;
		break;
	case GL_BOOL_VEC2:
		AXGL_DBGOUT("bvec2/bool2 is used in uniform\n");
		sz = sizeof(uint8_t) * 2;
		st = sizeof(uint8_t) * 2;
		ct = 1;
		break;
	case GL_BOOL_VEC3:
		AXGL_DBGOUT("bvec3/bool3 is used in uniform\n");
		sz = sizeof(uint8_t) * 3;
		st = sizeof(uint8_t) * 4;
		ct = 1;
		break;
	case GL_BOOL_VEC4:
		AXGL_DBGOUT("bvec4/bool4 is used in uniform\n");
		sz = sizeof(uint8_t) * 4;
		st = sizeof(uint8_t) * 4;
		ct = 1;
		break;
	case GL_FLOAT_MAT2:
		sz = sizeof(float) * 2;
		st = sizeof(float) * 2;
		ct = 2;
		break;
	case GL_FLOAT_MAT3:
		sz = sizeof(float) * 3;
		st = sizeof(float) * 4;
		ct = 3;
		break;
	case GL_FLOAT_MAT4:
		sz = sizeof(float) * 4;
		st = sizeof(float) * 4;
		ct = 4;
		break;
	case GL_FLOAT_MAT2x3: // C:2,R:3
		sz = sizeof(float) * 3;
		st = sizeof(float) * 4;
		ct = 2;
		break;
	case GL_FLOAT_MAT2x4:
		sz = sizeof(float) * 4;
		st = sizeof(float) * 4;
		ct = 2;
		break;
	case GL_FLOAT_MAT3x2:
		sz = sizeof(float) * 2;
		st = sizeof(float) * 2;
		ct = 3;
		break;
	case GL_FLOAT_MAT3x4:
		sz = sizeof(float) * 4;
		st = sizeof(float) * 4;
		ct = 3;
		break;
	case GL_FLOAT_MAT4x2:
		sz = sizeof(float) * 2;
		st = sizeof(float) * 2;
		ct = 4;
		break;
	case GL_FLOAT_MAT4x3:
		sz = sizeof(float) * 3;
		st = sizeof(float) * 4;
		ct = 4;
		break;
	default:
		AXGL_ASSERT(0);
		break;
	}
	*size = sz;
	*stride = st;
	*count = ct;
	return;
}

} // namespace axgl
