// CoreTexture.h
// Textureクラスの宣言
#ifndef __CoreTexture_h_
#define __CoreTexture_h_

#include "CoreObject.h"
#include "../backend/BackendTexture.h"
#include "../backend/BackendSampler.h"

namespace axgl { 

class CoreContext;
class BackendTexture;
class BackendSampler;

// Textureクラス
class CoreTexture : public CoreObject
{
public:
	CoreTexture();
	~CoreTexture();
	void terminate(CoreContext* context) override;
	void setTarget(GLenum target);
	GLenum getTarget() const;
	void compressedTexImage2d(CoreContext* context, GLenum target, GLint level, GLenum internalformat,
		GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
	void compressedTexSubImage2d(CoreContext* context, GLenum target, GLint level, GLint xoffset, GLint yoffset,
		GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
	void copyTexImage2d(GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
	void copyTexSubImage2d(GLint level, GLint xoffset, GLint yofffset, GLint x, GLint y, GLsizei width, GLsizei height);
	void texImage2d(CoreContext* context, GLenum target, GLint level, GLint internalformat,
		GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
	void texSubImage2d(CoreContext* context, GLenum target, GLint level, GLint xoffset, GLint yoffset,
		GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
	void texImage3d(CoreContext* context, GLenum target, GLint level, GLint internalformat,
		GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
	void texSubImage3d(CoreContext* context, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
		GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
	void copyTexSubImage3d(GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
	void compressedTexImage3d(CoreContext* context, GLenum target, GLint level, GLenum internalformat,
		GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data);
	void compressedTexSubImage3d(CoreContext* context, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
		GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
	void texParameterf(GLenum pname, GLfloat param);
	void texParameterfv(GLenum pname, const GLfloat* params);
	void texParameteri(GLenum pname, GLint param);
	void texParameteriv(GLenum pname, const GLint* params);
	void texStorage2d(CoreContext* context, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
	void texStorage3d(CoreContext* context, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
	void generateMipmap(CoreContext* context);
	GLenum getInternalformat() const
	{
		return m_internalformat;
	}
	void getParameterfv(GLenum pname, GLfloat* params);
	void getParameteriv(GLenum pname, GLint* params);
	bool setupSampler(CoreContext* context);
	BackendTexture* getBackendTexture() const
	{
		return m_pBackendTexture;
	}
	BackendSampler* getBackendSampler() const
	{
		return m_pBackendSampler;
	}

private:
	enum {
		TARGET_UNKNOWN = 0
	};

private:
	bool setupBackendTexture(CoreContext* context);

private:
	GLenum m_target = TARGET_UNKNOWN;
	GLenum m_internalformat = GL_RGBA8; // ### dummy initial value
	BackendTexture::TextureParameters m_textureParameters;
	GLboolean m_immutableFormat = GL_FALSE;
	BackendTexture* m_pBackendTexture = nullptr;
	BackendSampler::SamplerParameters m_samplerParameters;
	BackendSampler* m_pBackendSampler = nullptr;
	bool m_samplerDirty = true;
};

} // namespace axgl

#endif // __CoreTexture_h_
