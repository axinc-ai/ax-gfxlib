// TextureMetal.mm
#include "TextureMetal.h"
#include "ContextMetal.h"
#include "../../AXGLAllocatorImpl.h"

namespace axgl {

// BackendTextureクラスの実装 --------
BackendTexture* BackendTexture::create()
{
	TextureMetal* sampler = AXGL_NEW(TextureMetal);
	return sampler;
}
	
void BackendTexture::destroy(BackendTexture* texture)
{
	if (texture == nullptr) {
		return;
	}
	AXGL_DELETE(texture);
	return;
}

// TextureMetalクラスの実装 --------
enum ConvertType {
	ConvertTypeNone = 0,
	ConvertTypeRGB2RGBA = 1,
	ConvertTypeRGB2RGBAInt = 2
};

static inline MTLTextureSwizzle convert_swizzle(GLenum swizzle_gl)
{
	MTLTextureSwizzle swizzle_mtl = MTLTextureSwizzleRed;

	switch (swizzle_gl) {
	case GL_RED:
		break;
	case GL_GREEN:
		swizzle_mtl = MTLTextureSwizzleGreen;
		break;
	case GL_BLUE:
		swizzle_mtl = MTLTextureSwizzleBlue;
		break;
	case GL_ALPHA:
		swizzle_mtl = MTLTextureSwizzleAlpha;
		break;
	case GL_ZERO:
		swizzle_mtl = MTLTextureSwizzleZero;
		break;
	case GL_ONE:
		swizzle_mtl = MTLTextureSwizzleOne;
		break;
	default:
		AXGL_ASSERT(0);
		break;
	}

	return swizzle_mtl;
}

static void calc_lv0_size_2d(int w, int h, int lv, int* l0w, int* l0h)
{
	AXGL_ASSERT((l0w != nullptr) && (l0h != nullptr));
	int w0 = w;
	int h0 = h;
	for (; lv > 0; lv--) {
		w0 *= 2;
		h0 *= 2;
	}
	*l0w = w0;
	*l0h = h0;

	return;
}

static void calc_lv0_size_3d(int w, int h, int d, int lv, int* l0w, int* l0h, int* l0d)
{
	AXGL_ASSERT((l0w != nullptr) && (l0h != nullptr) && (l0d != nullptr));
	int w0 = w;
	int h0 = h;
	int d0 = d;
	for (; lv > 0; lv--) {
		w0 *= 2;
		h0 *= 2;
		d0 *= 2;
	}
	*l0w = w0;
	*l0h = h0;
	*l0d = d0;
	return;
}

static int calc_num_level_2d(int w, int h)
{
	int v = (w > h) ? w : h;
	int lv = 1;
	while (v > 1) {
		v /= 2;
		lv++;
	}
	return lv;
}

static int calc_num_level_3d(int w, int h, int d)
{
	int v = (w > h) ? ((d > w) ? d : w) : h;
	int lv = 1;
	while (v > 1) {
		v /= 2;
		lv++;
	}
	return lv;
}

static ConvertType get_convert_type(GLenum format, GLenum type, int* converted_format, int* converted_type)
{
	AXGL_ASSERT((converted_format != nullptr) && (converted_type != nullptr));

	ConvertType convert_type = ConvertTypeNone;

	switch (format) {
	// 24bit formats
	case GL_RGB:
		if (type == GL_UNSIGNED_BYTE) {
			convert_type = ConvertTypeRGB2RGBA;
			*converted_format = GL_RGBA;
			*converted_type = GL_UNSIGNED_BYTE;
		} else if (type == GL_BYTE) {
			convert_type = ConvertTypeRGB2RGBA;
			*converted_format = GL_RGBA;
			*converted_type = GL_BYTE;
		}
		break;
	case GL_RGB_INTEGER:
		if (type == GL_UNSIGNED_BYTE) {
			convert_type = ConvertTypeRGB2RGBAInt;
			*converted_format = GL_RGBA_INTEGER;
			*converted_type = GL_UNSIGNED_BYTE;
		} else if (type == GL_BYTE) {
			convert_type = ConvertTypeRGB2RGBAInt;
			*converted_format = GL_RGBA_INTEGER;
			*converted_type = GL_BYTE;
		}
		break;
	default:
		break;
	}

	return convert_type;
}

static bool convert_rgb_to_rgba(int w, int h, int d, const uint8_t* src, uint8_t** dst, uint8_t alpha)
{
	AXGL_ASSERT((src != nullptr) && (dst != nullptr));
	int num_pix = w * h * d;
	// バッファを確保
	*dst = static_cast<uint8_t*>(AXGL_ALLOC(num_pix * 4));
	if (*dst == nullptr) {
		return false;
	}
	// ピクセルデータを変換
	const uint8_t* sp = src;
	uint8_t* dp = *dst;
	for (int i = 0; i < num_pix; i++) {
		memcpy(dp, sp, sizeof(uint8_t) * 3);
		dp[3] = alpha;
		sp += 3;
		dp += 4;
	}
	return true;
}

static void release_convert_work(void* work)
{
	// バッファを解放
	if (work != nullptr) {
		AXGL_FREE(work);
	}
	return;
}

static NSUInteger get_slice_value(GLenum target)
{
	NSUInteger slice = 0;
	switch (target) {
	case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
		break;
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
		slice = 1;
		break;
	case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
		slice = 2;
		break;
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
		slice = 3;
		break;
	case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
		slice = 4;
		break;
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
		slice = 5;
		break;
	default:
		AXGL_ASSERT(0);
		break;
	}
	return slice;
}

static int calc_image_size(int lv, int size)
{
	int sz = size;
	for (int i = 0; i < lv; i++) {
		sz /= 2;
	}
	if (sz < 1) {
		sz = 1;
	}
	return sz;
}

static int32_t num_block_vertical(int32_t internalformat, int32_t height)
{
	int32_t num_block = 0;
	switch (internalformat) {
	case GL_COMPRESSED_R11_EAC:
	case GL_COMPRESSED_SIGNED_R11_EAC:
	case GL_COMPRESSED_RG11_EAC:
	case GL_COMPRESSED_SIGNED_RG11_EAC:
	case GL_COMPRESSED_RGB8_ETC2:
	case GL_COMPRESSED_SRGB8_ETC2:
	case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
	case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
	case GL_COMPRESSED_RGBA8_ETC2_EAC:
	case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
		// ETC2 : 4x4 block
		num_block = height / 4;
		if ((height & 0x3) != 0) {
			num_block++;
		}
		break;
	default:
		AXGL_ASSERT(0);
		break;
	}
	return num_block;
}

TextureMetal::TextureMetal()
{
}

TextureMetal::~TextureMetal()
{
}

bool TextureMetal::initialize(BackendContext* context)
{
	AXGL_ASSERT((m_mtlTextureDesc == nil) && (m_mtlTexture == nil));
	// MTLTextureDescriptorを作成
	m_mtlTextureDesc = [[MTLTextureDescriptor alloc] init];
	if (m_mtlTextureDesc == nil) {
		return false;
	}
	m_dirty = false;
	return true;
}

void TextureMetal::terminate(BackendContext* context)
{
	m_mtlTexture = nil;
	m_mtlTextureDesc = nil;
	// MTLTextureを解放したのでdirtyにしておく
	m_dirty = true;
	return;
}

bool TextureMetal::setImage2D(BackendContext* context, GLint level, GLenum internalformat,
	GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels, const TextureParameters* params)
{
	AXGL_ASSERT(params != nullptr);
	// ストレージが変化したかをチェック
	bool texture_changed = isStorageChanged(MTLTextureType2D, level, internalformat, width, height, 1);
	if ((m_mtlTexture == nil) || texture_changed) {
		int lv0_width  = width;
		int lv0_height = height;
		if (level != 0) {
			// レベル0のサイズを算出
			calc_lv0_size_2d(width, height, level, &lv0_width, &lv0_height);
		}
		// ミップマップレベル数
		int num_level = calc_num_level_2d(lv0_width, lv0_height);
		// 2Dストレージを作成
		createStorage2D(context, num_level, internalformat, lv0_width, lv0_height, params);
	}
	// フォーマット変換が必要な場合は実行
	int converted_format = 0;
	int converted_data_type = 0;
	uint8_t* convert_work_buf = nullptr;
	ConvertType convert_type = get_convert_type(format, type, &converted_format, &converted_data_type);
	if ((convert_type != ConvertTypeNone) && (pixels != nullptr)) {
		convert_work_buf = convertPixels(width, height, 1, convert_type, pixels);
		pixels = convert_work_buf;
		format = converted_format;
		type = converted_data_type;
	}
	// テクスチャイメージを設定
	if ((m_mtlTexture != nil) && (pixels != nullptr)) {
		MTLRegion region = {
			{0, 0, 0},
			{(NSUInteger)width, (NSUInteger)height, 1}
		};
		NSUInteger bytes_per_row = get_bytes_per_pixel(format, type) * width;
		[m_mtlTexture replaceRegion:region mipmapLevel:level withBytes:pixels bytesPerRow:bytes_per_row];
	}
	// ワークを作成している場合は解放
	release_convert_work(convert_work_buf);
	return true;
}

bool TextureMetal::setSubImage2D(BackendContext* context, GLint level, GLint xoffset, GLint yoffset,
	GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels)
{
	// サブイメージ設定可能か
	bool texture_changed = isSubImageAcceptable(MTLTextureType2D, level, xoffset, yoffset, 0, width, height, 1, format, type);
	if ((m_mtlTexture == nil) || !texture_changed) {
		return false;
	}
	// フォーマット変換が必要な場合は実行
	int converted_format = 0;
	int converted_data_type = 0;
	uint8_t* convert_work_buf = nullptr;
	ConvertType convert_type = get_convert_type(format, type, &converted_format, &converted_data_type);
	if ((convert_type != ConvertTypeNone) && (pixels != nullptr)) {
		convert_work_buf = convertPixels(width, height, 1, convert_type, pixels);
		pixels = convert_work_buf;
		format = converted_format;
		type = converted_data_type;
	}
	// テクスチャイメージを設定
	if ((m_mtlTexture != nil) && (pixels != nullptr)) {
		MTLRegion region = {
			{(NSUInteger)xoffset, (NSUInteger)yoffset, 0},
			{(NSUInteger)width, (NSUInteger)height, 1}
		};
		NSUInteger bytes_per_row = get_bytes_per_pixel(format, type) * width;
		[m_mtlTexture replaceRegion:region mipmapLevel:level withBytes:pixels bytesPerRow:bytes_per_row];
	}
	// ワークを作成している場合は解放
	release_convert_work(convert_work_buf);
	return true;
}

bool TextureMetal::setCompressedImage2D(BackendContext* context, GLint level, GLenum internalformat,
	GLsizei width, GLsizei height, GLsizei imageSize, const void* data, const TextureParameters* params)
{
	AXGL_ASSERT(params != nullptr);
	// ストレージが変化したかをチェック
	bool texture_changed = isStorageChanged(MTLTextureType2D, level, internalformat, width, height, 1);
	if ((m_mtlTexture == nil) || texture_changed) {
		int lv0_width  = width;
		int lv0_height = height;
		if (level != 0) {
			// レベル0のサイズを算出
			calc_lv0_size_2d(width, height, level, &lv0_width, &lv0_height);
		}
		// ミップマップレベル数
		int num_level = calc_num_level_2d(lv0_width, lv0_height);
		// 2Dストレージを作成
		createStorage2D(context, num_level, internalformat, lv0_width, lv0_height, params);
	}
	// テクスチャイメージを設定
	if ((m_mtlTexture != nil) && (data != nullptr)) {
		MTLRegion region = {
			{0, 0, 0},
			{(NSUInteger)width, (NSUInteger)height, 1}
		};
		NSUInteger bytes_per_row_blocks = get_bytes_per_row_blocks(internalformat, width);
		[m_mtlTexture replaceRegion:region mipmapLevel:level withBytes:data bytesPerRow:bytes_per_row_blocks];
	}

	return true;
}

bool TextureMetal::setImageCube(BackendContext* context, GLenum target, GLint level, GLenum internalformat,
	GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels, const TextureParameters* params)
{
	AXGL_ASSERT(params !=  nullptr);
	// ストレージが変化したかをチェック
	bool texture_changed = isStorageChanged(MTLTextureTypeCube, level, internalformat, width, height, 1);
	if ((m_mtlTexture == nil) || texture_changed) {
		int lv0_width  = width;
		int lv0_height = height;
		if (level != 0) {
			// レベル0のサイズを算出
			calc_lv0_size_2d(width, height, level, &lv0_width, &lv0_height);
		}
		// ミップマップレベル数
		int num_level = calc_num_level_2d(lv0_width, lv0_height);
		// Cubeストレージを作成
		createStorageCube(context, num_level, internalformat, lv0_width, lv0_height, params);
	}
	// フォーマット変換が必要な場合は実行
	int converted_format = 0;
	int converted_data_type = 0;
	uint8_t* convert_work_buf = nullptr;
	ConvertType convert_type = get_convert_type(format, type, &converted_format, &converted_data_type);
	if ((convert_type != ConvertTypeNone) && (pixels != nullptr)) {
		convert_work_buf = convertPixels(width, height, 1, convert_type, pixels);
		pixels = convert_work_buf;
		format = converted_format;
		type = converted_data_type;
	}
	// テクスチャイメージを設定
	if ((m_mtlTexture != nil) && (pixels != nullptr)) {
		MTLRegion region = {
			{0, 0, 0},
			{(NSUInteger)width, (NSUInteger)height, 1}
		};
		NSUInteger slice_value = get_slice_value(target);
		NSUInteger bytes_per_row = get_bytes_per_pixel(format, type) * width;
		NSUInteger bytes_per_image = bytes_per_row * height;
		[m_mtlTexture replaceRegion:region mipmapLevel:level slice:slice_value withBytes:pixels bytesPerRow:bytes_per_row bytesPerImage:bytes_per_image];
	}
	// ワークを作成している場合は解放
	release_convert_work(convert_work_buf);
	return true;
}

bool TextureMetal::setSubImageCube(BackendContext* context, GLenum target, GLint level, GLenum xoffset, GLenum yoffset,
	GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels)
{
	// サブイメージ設定可能か
	bool texture_changed = isSubImageAcceptable(MTLTextureTypeCube, level, xoffset, yoffset, 0, width, height, 1, format, type);
	if ((m_mtlTexture == nil) || !texture_changed) {
		return false;
	}
	// フォーマット変換が必要な場合は実行
	int converted_format = 0;
	int converted_data_type = 0;
	uint8_t* convert_work_buf = nullptr;
	ConvertType convert_type = get_convert_type(format, type, &converted_format, &converted_data_type);
	if ((convert_type != ConvertTypeNone) && (pixels != nullptr)) {
		convert_work_buf = convertPixels(width, height, 1, convert_type, pixels);
		pixels = convert_work_buf;
		format = converted_format;
		type = converted_data_type;
	}
	// テクスチャイメージを設定
	if ((m_mtlTexture != nil) && (pixels != nullptr)) {
		MTLRegion region = {
			{(NSUInteger)xoffset, (NSUInteger)yoffset, 0},
			{(NSUInteger)width, (NSUInteger)height, 1}
		};
		NSUInteger slice_value = get_slice_value(target);
		NSUInteger bytes_per_row = get_bytes_per_pixel(format, type) * width;
		NSUInteger bytes_per_image = bytes_per_row * height;
		[m_mtlTexture replaceRegion:region mipmapLevel:level slice:slice_value withBytes:pixels bytesPerRow:bytes_per_row bytesPerImage:bytes_per_image];
	}
	// ワークを作成している場合は解放
	release_convert_work(convert_work_buf);
	return true;
}

bool TextureMetal::setCompressedImageCube(BackendContext* context, GLenum target, GLint level, GLenum internalformat,
	GLsizei width, GLsizei height, GLsizei imageSize, const void* data, const TextureParameters* params)
{
	AXGL_ASSERT(params != nullptr);
	// ストレージが変化したかをチェック
	bool texture_changed = isStorageChanged(MTLTextureTypeCube, level, internalformat, width, height, 1);
	if ((m_mtlTexture == nil) || texture_changed) {
		int lv0_width  = width;
		int lv0_height = height;
		if (level != 0) {
			// レベル0のサイズを算出
			calc_lv0_size_2d(width, height, level, &lv0_width, &lv0_height);
		}
		// ミップマップレベル数
		int num_level = calc_num_level_2d(lv0_width, lv0_height);
		// Cubeストレージを作成
		createStorageCube(context, num_level, internalformat, lv0_width, lv0_height, params);
	}
	// テクスチャイメージを設定
	if ((m_mtlTexture != nil) && (data != nullptr)) {
		MTLRegion region = {
			{0, 0, 0},
			{(NSUInteger)width, (NSUInteger)height, 1}
		};
		NSUInteger slice_value = get_slice_value(target);
		NSUInteger bytes_per_row_blocks = get_bytes_per_row_blocks(internalformat, width);
		NSUInteger bytes_per_image = bytes_per_row_blocks * num_block_vertical(internalformat, height);
		[m_mtlTexture replaceRegion:region mipmapLevel:level slice:slice_value withBytes:data bytesPerRow:bytes_per_row_blocks bytesPerImage:bytes_per_image];
	}

	return true;
}

bool TextureMetal::setImage3D(BackendContext* context, GLint level, GLenum internalformat,
	GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels, const TextureParameters* params)
{
	AXGL_ASSERT(params != nullptr);
	// ストレージが変化したかをチェック
	bool texture_changed = isStorageChanged(MTLTextureType3D, level, internalformat, width, height, depth);
	if ((m_mtlTexture == nil) || texture_changed) {
		int lv0_width  = width;
		int lv0_height = height;
		int lv0_depth  = depth;
		if (level != 0) {
			// レベル0のサイズを算出
			calc_lv0_size_3d(width, height, depth, level, &lv0_width, &lv0_height, &lv0_depth);
		}
		// ミップマップレベル数
		int num_level = calc_num_level_3d(lv0_width, lv0_height, lv0_depth);
		// 3Dストレージを作成
		createStorage3D(context, num_level, internalformat, lv0_width, lv0_height, lv0_depth, params);
	}
	// フォーマット変換が必要な場合は実行
	int converted_format = 0;
	int converted_data_type = 0;
	uint8_t* convert_work_buf = nullptr;
	ConvertType convert_type = get_convert_type(format, type, &converted_format, &converted_data_type);
	if ((convert_type != ConvertTypeNone) && (pixels != nullptr)) {
		convert_work_buf = convertPixels(width, height, depth, convert_type, pixels);
		pixels = convert_work_buf;
		format = converted_format;
		type = converted_data_type;
	}
	// テクスチャイメージを設定
	if ((m_mtlTexture != nil) && (pixels != nullptr)) {
		MTLRegion region = {
			{0, 0, 0},
			{(NSUInteger)width, (NSUInteger)height, (NSUInteger)depth}
		};
		NSUInteger bytes_per_row = get_bytes_per_pixel(format, type) * width;
		[m_mtlTexture replaceRegion:region mipmapLevel:level withBytes:pixels bytesPerRow:bytes_per_row];
	}
	// ワークを作成している場合は解放
	release_convert_work(convert_work_buf);
	return true;
}

bool TextureMetal::setSubImage3D(BackendContext* context, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
	GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels)
{
	// サブイメージ設定可能か
	bool texture_changed = isSubImageAcceptable(MTLTextureType3D, level, xoffset, yoffset, zoffset, width, height, depth, format, type);
	if ((m_mtlTexture == nil) || !texture_changed) {
		return false;
	}
	// フォーマット変換が必要な場合は実行
	int converted_format = 0;
	int converted_data_type = 0;
	uint8_t* convert_work_buf = nullptr;
	ConvertType convert_type = get_convert_type(format, type, &converted_format, &converted_data_type);
	if ((convert_type != ConvertTypeNone) && (pixels != nullptr)) {
		convert_work_buf = convertPixels(width, height, depth, convert_type, pixels);
		pixels = convert_work_buf;
		format = converted_format;
		type = converted_data_type;
	}
	// テクスチャイメージを設定
	if ((m_mtlTexture != nil) && (pixels != nullptr)) {
		MTLRegion region = {
			{(NSUInteger)xoffset, (NSUInteger)yoffset, (NSUInteger)zoffset},
			{(NSUInteger)width, (NSUInteger)height, (NSUInteger)depth}
		};
		NSUInteger bytes_per_row = get_bytes_per_pixel(format, type) * width;
		[m_mtlTexture replaceRegion:region mipmapLevel:level withBytes:pixels bytesPerRow:bytes_per_row];
	}
	// ワークを作成している場合は解放
	release_convert_work(convert_work_buf);
	return true;
}

bool TextureMetal::setCompressedImage3D(BackendContext* context, GLint level, GLenum internalformat,
	GLsizei width, GLsizei height, GLsizei depth, GLsizei imageSize, const void* data, const TextureParameters* params)
{
	// TODO:
	AXGL_ASSERT(0);
	return true;
}

bool TextureMetal::setImage2DArray(BackendContext* context, GLint level, GLenum internalformat,
	GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels, const TextureParameters* params)
{
	AXGL_ASSERT(params != nullptr);
	// ストレージが変化したかをチェック
	bool texture_changed = isStorageChanged(MTLTextureType2DArray, level, internalformat, width, height, depth);
	if ((m_mtlTexture == nil) || texture_changed) {
		int lv0_width  = width;
		int lv0_height = height;
		int lv0_depth  = depth;
		if (level != 0) {
			// レベル0のサイズを算出
			calc_lv0_size_3d(width, height, depth, level, &lv0_width, &lv0_height, &lv0_depth);
		}
		// ミップマップレベル数
		int num_level = calc_num_level_3d(lv0_width, lv0_height, lv0_depth);
		// 2DArrayストレージを作成
		createStorage2DArray(context, num_level, internalformat, lv0_width, lv0_height, lv0_depth, params);
	}
	// TODO:
	AXGL_ASSERT(0);
	return true;
}

bool TextureMetal::setSubImage2DArray(BackendContext* context, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
	GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels)
{
	// サブイメージ設定可能か
	bool texture_changed = isSubImageAcceptable(MTLTextureType2DArray, level, xoffset, yoffset, zoffset, width, height, depth, format, type);
	if ((m_mtlTexture == nil) || !texture_changed) {
		return false;
	}
	// TODO:
	AXGL_ASSERT(0);
	return true;
}

bool TextureMetal::setCompressedImage2DArray(BackendContext* context, GLint level, GLenum internalformat,
	GLsizei width, GLsizei height, GLsizei depth, GLsizei imageSize, const void* data, const TextureParameters* params)
{
	// TODO
	AXGL_ASSERT(0);
	return true;
}

bool TextureMetal::createStorage2D(BackendContext* context, GLsizei levels, GLenum internalformat,
	GLsizei width, GLsizei height, const TextureParameters* params)
{
	// NOTE: glTexImage*,glTexStorage* を呼び出す前にテクスチャパラメータ（サンプラパラメータは含まない）を設定する前提の処理
	MTLPixelFormat pixel_format = convert_internalformat(internalformat);
	// MTLTextureUsageRenderTarget: フレームバッファにアタッチすることを想定
	MTLTextureUsage texture_usage = MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget;
#if TARGET_OS_IPHONE
	MTLStorageMode storage_mode = MTLStorageModeShared;
#else
	MTLStorageMode storage_mode = MTLStorageModeManaged;
#endif
	[m_mtlTextureDesc setTextureType:MTLTextureType2D];
	[m_mtlTextureDesc setPixelFormat:pixel_format];
	[m_mtlTextureDesc setWidth:width];
	[m_mtlTextureDesc setHeight:height];
	[m_mtlTextureDesc setUsage:texture_usage];
	[m_mtlTextureDesc setStorageMode:storage_mode];
	[m_mtlTextureDesc setSampleCount:1];
	// NOTE: support level zero only
	AXGL_ASSERT(params->baseLevel == 0);
	uint32_t mipmap_level_count = (levels < (params->maxLevel + 1)) ? levels : (params->maxLevel + 1);
	[m_mtlTextureDesc setMipmapLevelCount:mipmap_level_count];
	// swizzle
	MTLTextureSwizzleChannels mtl_swizzle = {
		convert_swizzle(params->swizzleR), // red
		convert_swizzle(params->swizzleG), // green
		convert_swizzle(params->swizzleB), // blue
		convert_swizzle(params->swizzleA)  // alpha
	};
	[m_mtlTextureDesc setSwizzle:mtl_swizzle];
	// MTLTextureを作成
	ContextMetal* mtl_context = reinterpret_cast<ContextMetal*>(context);
	id<MTLDevice> mtl_device = mtl_context->getDevice();
	m_mtlTexture = [mtl_device newTextureWithDescriptor:m_mtlTextureDesc];
	// MTLTextureを再生成したことからdirty
	m_dirty = true;
	if (m_mtlTexture == nil) {
		return false;
	}
	return true;
}

bool TextureMetal::createStorageCube(BackendContext* context, GLsizei levels, GLenum internalformat,
	GLsizei width, GLsizei height, const TextureParameters* params)
{
	// NOTE: glTexImage*,glTexStorage* を呼び出す前にテクスチャパラメータ（サンプラパラメータは含まない）を設定する前提の処理
	MTLPixelFormat pixel_format = convert_internalformat(internalformat);
	MTLTextureUsage texture_usage = MTLTextureUsageShaderRead;
#if TARGET_OS_IPHONE
	MTLStorageMode storage_mode = MTLStorageModeShared;
#else
	MTLStorageMode storage_mode = MTLStorageModeManaged;
#endif
	[m_mtlTextureDesc setTextureType:MTLTextureTypeCube];
	[m_mtlTextureDesc setPixelFormat:pixel_format];
	[m_mtlTextureDesc setWidth:width];
	[m_mtlTextureDesc setHeight:height];
	[m_mtlTextureDesc setUsage:texture_usage];
	[m_mtlTextureDesc setStorageMode:storage_mode];
	[m_mtlTextureDesc setSampleCount:1];
	// NOTE: support level zero only
	AXGL_ASSERT(params->baseLevel == 0);
	uint32_t mipmap_level_count = (levels < (params->maxLevel + 1)) ? levels : (params->maxLevel + 1);
	[m_mtlTextureDesc setMipmapLevelCount:mipmap_level_count];
	// swizzle
	MTLTextureSwizzleChannels mtl_swizzle = {
		convert_swizzle(params->swizzleR), // red
		convert_swizzle(params->swizzleG), // green
		convert_swizzle(params->swizzleB), // blue
		convert_swizzle(params->swizzleA)  // alpha
	};
	[m_mtlTextureDesc setSwizzle:mtl_swizzle];
	// MTLTextureを作成
	ContextMetal* mtl_context = reinterpret_cast<ContextMetal*>(context);
	id<MTLDevice> mtl_device = mtl_context->getDevice();
	m_mtlTexture = [mtl_device newTextureWithDescriptor:m_mtlTextureDesc];
	// MTLTextureを再生成したことからdirty
	m_dirty = true;
	if (m_mtlTexture == nil) {
		return false;
	}
	return true;
}

bool TextureMetal::createStorage3D(BackendContext* context, GLsizei levels, GLenum internalformat,
	GLsizei width, GLsizei height, GLsizei depth, const TextureParameters* params)
{
	// NOTE: glTexImage*,glTexStorage* を呼び出す前にテクスチャパラメータ（サンプラパラメータは含まない）を設定する前提の処理
	MTLPixelFormat pixel_format = convert_internalformat(internalformat);
	MTLTextureUsage texture_usage = MTLTextureUsageShaderRead;
#if TARGET_OS_IPHONE
	MTLStorageMode storage_mode = MTLStorageModeShared;
#else
	MTLStorageMode storage_mode = MTLStorageModeManaged;
#endif
	[m_mtlTextureDesc setTextureType:MTLTextureType3D];
	[m_mtlTextureDesc setPixelFormat:pixel_format];
	[m_mtlTextureDesc setWidth:width];
	[m_mtlTextureDesc setHeight:height];
	[m_mtlTextureDesc setDepth:depth];
	[m_mtlTextureDesc setUsage:texture_usage];
	[m_mtlTextureDesc setStorageMode:storage_mode];
	[m_mtlTextureDesc setSampleCount:1];
	// NOTE: support level zero only
	AXGL_ASSERT(params->baseLevel == 0);
	uint32_t mipmap_level_count = (levels < (params->maxLevel + 1)) ? levels : (params->maxLevel + 1);
	[m_mtlTextureDesc setMipmapLevelCount:mipmap_level_count];
	// swizzle
	MTLTextureSwizzleChannels mtl_swizzle = {
		convert_swizzle(params->swizzleR), // red
		convert_swizzle(params->swizzleG), // green
		convert_swizzle(params->swizzleB), // blue
		convert_swizzle(params->swizzleA)  // alpha
	};
	[m_mtlTextureDesc setSwizzle:mtl_swizzle];
	// MTLTextureを作成
	ContextMetal* mtl_context = reinterpret_cast<ContextMetal*>(context);
	id<MTLDevice> mtl_device = mtl_context->getDevice();
	m_mtlTexture = [mtl_device newTextureWithDescriptor:m_mtlTextureDesc];
	// MTLTextureを再生成したことからdirty
	m_dirty = true;
	if (m_mtlTexture == nil) {
		return false;
	}
	return true;
}

bool TextureMetal::createStorage2DArray(BackendContext* context, GLsizei levels, GLenum internalformat,
	GLsizei width, GLsizei height, GLsizei depth, const TextureParameters* params)
{
	// TODO:
	AXGL_ASSERT(0);
	return true;
}

bool TextureMetal::generateMipmap(BackendContext* context, const TextureParameters* params)
{
	if (m_mtlTexture == nil) {
		// テクスチャイメージのストレージを確保するAPIの呼び出しなしに実行された状態
		// 無視して成功で終了
		return true;
	}
	AXGL_ASSERT(params != nullptr);
	// NOTE: support level zero only
	AXGL_ASSERT(params->baseLevel == 0);
	// generateMipmapsForTextureを実行
	ContextMetal* context_metal = static_cast<ContextMetal*>(context);
	AXGL_ASSERT(context_metal != nullptr);
	id<MTLCommandQueue> command_queue = context_metal->getCommandQueue();
	AXGL_ASSERT(command_queue != nil);
	id<MTLCommandBuffer> command_buffer = [command_queue commandBuffer];
	AXGL_ASSERT(command_buffer != nil);
	id<MTLBlitCommandEncoder> blit_encoder = [command_buffer blitCommandEncoder];
	AXGL_ASSERT(blit_encoder != nil);
	[blit_encoder generateMipmapsForTexture:m_mtlTexture];
	[blit_encoder endEncoding];
	[command_buffer commit];
	[command_buffer waitUntilCompleted];
	return true;
}

id<MTLTexture> TextureMetal::getMtlTexture() const
{
	return m_mtlTexture;
}

MTLPixelFormat TextureMetal::getPixelFormat() const
{
	MTLPixelFormat pixel_format = MTLPixelFormatInvalid;
	if (m_mtlTextureDesc != nil) {
		pixel_format = [m_mtlTextureDesc pixelFormat];
	}
	return pixel_format;
}

bool TextureMetal::isDirty() const
{
	return m_dirty;
}

void TextureMetal::clearDirty()
{
	m_dirty = false;
}

bool TextureMetal::isStorageChanged(MTLTextureType mtltype, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
	// TextureType
	if (mtltype != [m_mtlTextureDesc textureType]) {
		return true;
	}
	// PixelFormat
	MTLPixelFormat img_format = convert_internalformat(internalformat);
	if (img_format != [m_mtlTextureDesc pixelFormat]) {
		return true;
	}
	// Width, Height, Depth
	NSUInteger lv0_width  = [m_mtlTextureDesc width];
	NSUInteger lv0_height = [m_mtlTextureDesc height];
	NSUInteger lv0_depth  = [m_mtlTextureDesc depth];
	int tex_width  = calc_image_size(level, (int)lv0_width);
	int tex_height = calc_image_size(level, (int)lv0_height);
	int tex_depth  = calc_image_size(level, (int)lv0_depth);
	if ((tex_width != width) || (tex_height != height) || (tex_depth != depth)) {
		return true;
	}
	return false;
}

bool TextureMetal::isSubImageAcceptable(MTLTextureType mtltype, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
	GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type)
{
	// TextureType
	if (mtltype != [m_mtlTextureDesc textureType]) {
		return false;
	}
	// PixelFormat
	MTLPixelFormat img_format = get_pixel_format(format, type);
	if (img_format != [m_mtlTextureDesc pixelFormat]) {
		return false;
	}
	// Width, Height, Depth
	NSUInteger lv0_width  = [m_mtlTextureDesc width];
	NSUInteger lv0_height = [m_mtlTextureDesc height];
	NSUInteger lv0_depth  = [m_mtlTextureDesc depth];
	int tex_width  = calc_image_size(level, (int)lv0_width);
	int tex_height = calc_image_size(level, (int)lv0_height);
	int tex_depth  = calc_image_size(level, (int)lv0_depth);
	if ((xoffset < 0) || (yoffset < 0) || (zoffset < 0)
		|| ((xoffset + width) > tex_width) || ((yoffset + height) > tex_height) || ((zoffset + depth) > tex_depth)) {
		return false;
	}
	return true;
}

uint8_t* TextureMetal::convertPixels(int w, int h, int d, int convertType, const void* pixels)
{
	uint8_t* work_buf = nullptr;
	AXGL_ASSERT(pixels != nullptr);
	switch (convertType) {
	case ConvertTypeRGB2RGBA:
		if (!convert_rgb_to_rgba(w, h, d, static_cast<const uint8_t*>(pixels), &work_buf, 0xff)) {
			AXGL_DBGOUT("RGB2RGBA> convert_rgb_to_rgba() failed\n");
		}
		break;
	case ConvertTypeRGB2RGBAInt:
		if (!convert_rgb_to_rgba(w, h, d, static_cast<const uint8_t*>(pixels), &work_buf, 1)) {
			AXGL_DBGOUT("RGB2RGBAInt> convert_rgb_to_rgba() failed\n");
		}
		break;
	default:
		AXGL_ASSERT(0);
		break;
	}
	return work_buf;
}

} // namespace axgl
