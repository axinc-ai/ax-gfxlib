// CoreUtility.cpp
#include "CoreUtility.h"

namespace axgl {

int getAlphaBitsFromFormat(GLenum format)
{
	int alpha_bits = 0;
	switch (format) {
	case GL_R8:
	case GL_R8UI:
	case GL_R8I:
	case GL_R16UI:
	case GL_R16I:
	case GL_R32UI:
	case GL_R32I:
	case GL_RG8:
	case GL_RG8UI:
	case GL_RG8I:
	case GL_RG16UI:
	case GL_RG16I:
	case GL_RG32UI:
	case GL_RG32I:
	case GL_RGB8:
	case GL_RGB565:
		// none
		break;
	case GL_RGB5_A1:
		// 1bit
		alpha_bits = 1;
		break;
	case GL_RGB10_A2:
	case GL_RGB10_A2UI:
		// 2bits
		alpha_bits = 2;
		break;
	case GL_RGBA4:
		// 4bits
		alpha_bits = 4;
		break;
	case GL_RGBA8:
	case GL_SRGB8_ALPHA8:
	case GL_RGBA8UI:
	case GL_RGBA8I:
		// 8bits
		alpha_bits = 8;
		break;
	case GL_RGBA16UI:
	case GL_RGBA16I:
		// 16bits
		alpha_bits = 16;
		break;
	case GL_RGBA32UI:
	case GL_RGBA32I:
		// 32bits
		alpha_bits = 32;
		break;
	default:
		break;
	}
	return alpha_bits;
}

int getRedBitsFromFormat(GLenum format)
{
	int red_bits = 0;
	switch (format) {
	case GL_RGBA4:
		// 4bits
		red_bits = 4;
		break;
	case GL_RGB565:
	case GL_RGB5_A1:
		// 5bits
		red_bits = 5;
		break;
	case GL_R8:
	case GL_R8UI:
	case GL_R8I:
	case GL_RG8:
	case GL_RG8UI:
	case GL_RG8I:
	case GL_RGB8:
	case GL_RGBA8:
	case GL_SRGB8_ALPHA8:
	case GL_RGBA8UI:
	case GL_RGBA8I:
		// 8bits
		red_bits = 8;
		break;
	case GL_RGB10_A2:
	case GL_RGB10_A2UI:
		// 10bits
		red_bits = 10;
		break;
	case GL_R16UI:
	case GL_R16I:
	case GL_RG16UI:
	case GL_RG16I:
	case GL_RGBA16UI:
	case GL_RGBA16I:
		// 16bits
		red_bits = 16;
		break;
	case GL_R32UI:
	case GL_R32I:
	case GL_RG32UI:
	case GL_RG32I:
	case GL_RGBA32UI:
	case GL_RGBA32I:
		// 32bits
		red_bits = 32;
		break;
	default:
		break;
	}
	return red_bits;
}

int getGreenBitsFromFormat(GLenum format)
{
	int green_bits = 0;
	switch (format) {
	case GL_R8:
	case GL_R8UI:
	case GL_R8I:
	case GL_R16UI:
	case GL_R16I:
	case GL_R32UI:
	case GL_R32I:
		// 0bit
		green_bits = 0;
		break;
	case GL_RGBA4:
		// 4bits
		green_bits = 4;
		break;
	case GL_RGB5_A1:
		// 5bits
		green_bits = 5;
		break;
	case GL_RGB565:
		// 6bits
		green_bits = 6;
		break;
	case GL_RG8:
	case GL_RG8UI:
	case GL_RG8I:
	case GL_RGB8:
	case GL_RGBA8:
	case GL_SRGB8_ALPHA8:
	case GL_RGBA8UI:
	case GL_RGBA8I:
		// 8bits
		green_bits = 8;
		break;
	case GL_RGB10_A2:
	case GL_RGB10_A2UI:
		// 10bits
		green_bits = 10;
		break;
	case GL_RG16UI:
	case GL_RG16I:
	case GL_RGBA16UI:
	case GL_RGBA16I:
		// 16bits
		green_bits = 16;
		break;
	case GL_RG32UI:
	case GL_RG32I:
	case GL_RGBA32UI:
	case GL_RGBA32I:
		// 32bits
		green_bits = 32;
		break;
	default:
		break;
	}
	return green_bits;
}

int getBlueBitsFromFormat(GLenum format)
{
	int blue_bits = 0;
	switch (format) {
	case GL_R8:
	case GL_R8UI:
	case GL_R8I:
	case GL_R16UI:
	case GL_R16I:
	case GL_R32UI:
	case GL_R32I:
	case GL_RG8:
	case GL_RG8UI:
	case GL_RG8I:
	case GL_RG16UI:
	case GL_RG16I:
	case GL_RG32UI:
	case GL_RG32I:
		// none
		break;
	case GL_RGBA4:
		// 4bits
		blue_bits = 4;
		break;
	case GL_RGB565:
	case GL_RGB5_A1:
		// 5bits
		blue_bits = 5;
		break;
	case GL_RGB8:
	case GL_RGBA8:
	case GL_SRGB8_ALPHA8:
	case GL_RGBA8UI:
	case GL_RGBA8I:
		// 8bits
		blue_bits = 8;
		break;
	case GL_RGB10_A2:
	case GL_RGB10_A2UI:
		// 10bits
		blue_bits = 10;
		break;
	case GL_RGBA16UI:
	case GL_RGBA16I:
		// 16bits
		blue_bits = 16;
		break;
	case GL_RGBA32UI:
	case GL_RGBA32I:
		// 32bits
		blue_bits = 32;
		break;
	default:
		break;
	}
	return blue_bits;
}

int getDepthBitsFromFormat(GLenum format)
{
	int depth_bits = 0;
	switch (format) {
	case GL_DEPTH_COMPONENT16:
		depth_bits = 16;
		break;
	case GL_DEPTH_COMPONENT24:
	case GL_DEPTH24_STENCIL8:
		depth_bits = 24;
		break;
	case GL_DEPTH_COMPONENT32F:
	case GL_DEPTH32F_STENCIL8:
		depth_bits = 32;
		break;
	default:
		break;
	}
	return depth_bits;
}

int getStencilBitsFromFormat(GLenum format)
{
	int stencil_bits = 0;
	switch (format) {
	case GL_DEPTH24_STENCIL8:
	case GL_DEPTH32F_STENCIL8:
	case GL_STENCIL_INDEX8:
		stencil_bits = 8;
		break;
	default:
		break;
	}
	return stencil_bits;
}

GLenum getComponentTypeFromFormat(GLenum format, bool isStencil)
{
	GLenum component_type = GL_UNSIGNED_NORMALIZED;
	switch (format) {
	case GL_R8:
	case GL_RG8:
	case GL_RGBA4:
	case GL_RGB565:
	case GL_RGB5_A1:
	case GL_RGB8:
	case GL_RGBA8:
	case GL_SRGB8_ALPHA8:
	case GL_RGB10_A2:
	case GL_DEPTH_COMPONENT16:
	case GL_DEPTH_COMPONENT24:
		break;
		break;
	case GL_R8UI:
	case GL_R16UI:
	case GL_R32UI:
	case GL_RG8UI:
	case GL_RG16UI:
	case GL_RG32UI:
	case GL_RGBA8UI:
	case GL_RGB10_A2UI:
	case GL_RGBA16UI:
	case GL_RGBA32UI:
	case GL_STENCIL_INDEX8:
		component_type = GL_UNSIGNED_INT;
		break;
	case GL_R8I:
	case GL_R16I:
	case GL_R32I:
	case GL_RG8I:
	case GL_RG16I:
	case GL_RG32I:
	case GL_RGBA8I:
	case GL_RGBA16I:
	case GL_RGBA32I:
		component_type = GL_INT;
		break;
	case GL_DEPTH24_STENCIL8:
		component_type = isStencil ? GL_UNSIGNED_INT : GL_UNSIGNED_NORMALIZED;
		break;
	case GL_DEPTH32F_STENCIL8:
		component_type = isStencil ? GL_UNSIGNED_INT : GL_FLOAT;
		break;
	case GL_DEPTH_COMPONENT32F:
		component_type = GL_FLOAT;
		break;
	default:
		break;
	}
	return component_type;
}

GLenum getColorEncodingFromFormat(GLenum format)
{
	GLenum color_encoding = GL_LINEAR;
	switch (format) {
	case GL_SRGB8_ALPHA8:
		color_encoding = GL_SRGB;
		break;
	default:
		break;
	}
	return color_encoding;
}

GLboolean isIntegerFromType(GLenum type)
{
	GLboolean rval = GL_FALSE;
	switch (type) {
	case GL_BYTE:
	case GL_UNSIGNED_BYTE:
	case GL_SHORT:
	case GL_UNSIGNED_SHORT:
	case GL_INT_2_10_10_10_REV:
	case GL_UNSIGNED_INT:
	case GL_FIXED:
		rval = GL_TRUE;
		break;
	case GL_HALF_FLOAT:
	case GL_FLOAT:
		break;
	default:
		break;
	}
	return rval;
}

} // namespace axgl
