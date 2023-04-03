// BackendTexture.h
#ifndef __BackendTexture_h_
#define __BackendTexture_h_

#include "../common/axglCommon.h"

namespace axgl {

class BackendContext;

class BackendTexture
{
public:
	// interface struct
	struct TextureParameters {
		GLint baseLevel = 0;
		GLint maxLevel = 1000;
		GLenum swizzleR = GL_RED;
		GLenum swizzleG = GL_GREEN;
		GLenum swizzleB = GL_BLUE;
		GLenum swizzleA = GL_ALPHA;
	};

public:
	virtual ~BackendTexture() {}
	virtual bool initialize(BackendContext* context) = 0;
	virtual void terminate(BackendContext* context) = 0;
	virtual bool setImage2D(BackendContext* context, GLint level, GLenum internalformat,
		GLsizei width, GLsizei height, GLenum fomrat, GLenum type, const void* pixels, const TextureParameters* params) = 0;
	virtual bool setSubImage2D(BackendContext* context, GLint level, GLint xoffset, GLint yoffset,
		GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels) = 0;
	virtual bool setCompressedImage2D(BackendContext* context, GLint level, GLenum internalformat,
		GLsizei width, GLsizei height, GLsizei imageSize, const void* data, const TextureParameters* params) = 0;
	virtual bool setImageCube(BackendContext* context, GLenum target, GLint level, GLenum internalformat,
		GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels, const TextureParameters* params) = 0;
	virtual bool setSubImageCube(BackendContext* context, GLenum target, GLint level, GLenum xoffset, GLenum yoffset,
		GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels) = 0;
	virtual bool setCompressedImageCube(BackendContext* context, GLenum target, GLint level, GLenum internalformat,
		GLsizei width, GLsizei height, GLsizei imageSize, const void* data, const TextureParameters* params) = 0;
	virtual bool setImage3D(BackendContext* context, GLint level, GLenum internalformat,
		GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels, const TextureParameters* params) = 0;
	virtual bool setSubImage3D(BackendContext* context, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
		GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels) = 0;
	virtual bool setCompressedImage3D(BackendContext* context, GLint level, GLenum internalformat,
		GLsizei width, GLsizei height, GLsizei depth, GLsizei imageSize, const void* data, const TextureParameters* params) = 0;
	virtual bool setImage2DArray(BackendContext* context, GLint level, GLenum internalformat,
		GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels, const TextureParameters* params) = 0;
	virtual bool setSubImage2DArray(BackendContext* context, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
		GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels) = 0;
	virtual bool setCompressedImage2DArray(BackendContext* context, GLint level, GLenum internalformat,
		GLsizei width, GLsizei height, GLsizei depth, GLsizei imageSize, const void* data, const TextureParameters* params) = 0;
	virtual bool createStorage2D(BackendContext* context, GLsizei levels, GLenum internalformat,
		GLsizei width, GLsizei height, const TextureParameters* params) = 0;
	virtual bool createStorageCube(BackendContext* context, GLsizei levels, GLenum internalformat,
		GLsizei width, GLsizei height, const TextureParameters* params) = 0;
	virtual bool createStorage3D(BackendContext* context, GLsizei levels, GLenum internalformat,
		GLsizei width, GLsizei height, GLsizei depth, const TextureParameters* params) = 0;
	virtual bool createStorage2DArray(BackendContext* context, GLsizei levels, GLenum internalformat,
		GLsizei width, GLsizei height, GLsizei depth, const TextureParameters* params) = 0;
	virtual bool generateMipmap(BackendContext* context, const TextureParameters* params) = 0;

public:
	static BackendTexture* create();
	static void destroy(BackendTexture* texture);
};

} // namespace axgl

#endif // __BackendTexture_h_

