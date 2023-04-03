// TextureMetal.h
#ifndef __TextureMetal_h_
#define __TextureMetal_h_
#include "BackendMetal.h"
#include "../BackendTexture.h"

namespace axgl {

class BackendContext;

class TextureMetal : public BackendTexture
{
public:
	TextureMetal();
	virtual ~TextureMetal();
	virtual bool initialize(BackendContext* context) override;
	virtual void terminate(BackendContext* context) override;
	virtual bool setImage2D(BackendContext* context, GLint level, GLenum internalformat,
		GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels, const TextureParameters* params) override;
	virtual bool setSubImage2D(BackendContext* context, GLint level, GLint xoffset, GLint yoffset,
		GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels) override;
	virtual bool setCompressedImage2D(BackendContext* context, GLint level, GLenum internalformat,
		GLsizei width, GLsizei height, GLsizei imageSize, const void* data, const TextureParameters* params) override;
	virtual bool setImageCube(BackendContext* context, GLenum target, GLint level, GLenum internalformat,
		GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels, const TextureParameters* params) override;
	virtual bool setSubImageCube(BackendContext* context, GLenum target, GLint level, GLenum xoffset, GLenum yoffset,
		GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels) override;
	virtual bool setCompressedImageCube(BackendContext* context, GLenum target, GLint level, GLenum internalformat,
		GLsizei width, GLsizei height, GLsizei imageSize, const void* data, const TextureParameters* params) override;
	virtual bool setImage3D(BackendContext* context, GLint level, GLenum internalformat,
		GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels, const TextureParameters* params) override;
	virtual bool setSubImage3D(BackendContext* context, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
		GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels) override;
	virtual bool setCompressedImage3D(BackendContext* context, GLint level, GLenum internalformat,
		GLsizei width, GLsizei height, GLsizei depth, GLsizei imageSize, const void* data, const TextureParameters* params) override;
	virtual bool setImage2DArray(BackendContext* context, GLint level, GLenum internalformat,
		GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels, const TextureParameters* params) override;
	virtual bool setSubImage2DArray(BackendContext* context, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
		GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels) override;
	virtual bool setCompressedImage2DArray(BackendContext* context, GLint level, GLenum internalformat,
		GLsizei width, GLsizei height, GLsizei depth, GLsizei imageSize, const void* data, const TextureParameters* params) override;
	virtual bool createStorage2D(BackendContext* context, GLsizei levels, GLenum internalformat,
		GLsizei width, GLsizei height, const TextureParameters* params) override;
	virtual bool createStorageCube(BackendContext* context, GLsizei levels, GLenum internalformat,
		GLsizei width, GLsizei height, const TextureParameters* params) override;
	virtual bool createStorage3D(BackendContext* context, GLsizei levels, GLenum internalformat,
		GLsizei width, GLsizei height, GLsizei depth, const TextureParameters* params) override;
	virtual bool createStorage2DArray(BackendContext* context, GLsizei levels, GLenum internalformat,
		GLsizei width, GLsizei height, GLsizei depth, const TextureParameters* params) override;
	virtual bool generateMipmap(BackendContext* context, const TextureParameters* params) override;

public:
	id<MTLTexture> getMtlTexture() const;
	MTLPixelFormat getPixelFormat() const;
	bool isDirty() const;
	void clearDirty();

private:
	bool isStorageChanged(MTLTextureType mtltype, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth);
	bool isSubImageAcceptable(MTLTextureType mtltype, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
		GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type);
	static uint8_t* convertPixels(int w, int h, int d, int convertType, const void* pixels);

private:
	MTLTextureDescriptor* m_mtlTextureDesc = nil;
	id<MTLTexture> m_mtlTexture = nil;
	bool m_dirty = false;
};

} // namespace axgl

#endif // __TextureMetal_h_

