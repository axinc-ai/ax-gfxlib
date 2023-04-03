// CoreState.h
// Stateクラスの宣言
#ifndef __CoreState_h_
#define __CoreState_h_

#include "../common/axglCommon.h"
#include "../common/DrawParameters.h"
#include "../common/ClearParameters.h"
#include <memory>

namespace axgl {

class CoreContext;
class CoreBuffer;
class CoreTexture;
class CoreFramebuffer;
class CoreRenderbuffer;
class CoreVertexArray;
class CoreSampler;
class CoreTransformFeedback;
class CoreProgram;
class CoreQuery;

// Stateクラス
class CoreState
{
public:
	// pixel pack parameters
	struct PackParams {
		// GL_PACK_ROW_LENGTH
		GLint packRowLength = 0;
		// GL_PACK_SKIP_ROWS
		GLint packSkipRows = 0;
		// GL_PACK_SKIP_PIXELS
		GLint packSkipPixels = 0;
		// GL_PACK_ALIGNMENT
		GLint packAlignment = 4;
	};
	// pixel unpack parameters
	struct UnpackParams {
		// GL_UNPACK_ROW_LENGTH
		GLint unpackRowLength = 0;
		// GL_UNPACK_IMAGE_HEIGHT
		GLint unpackImageHeight = 0;
		// GL_UNPACK_SKIP_ROWS
		GLint unpackSkipRows = 0;
		// GL_UNPACK_SKIP_PIXELS
		GLint unpackSkipPixels = 0;
		// GL_UNPACK_SKIP_IMAGES
		GLint unpackSkipImages = 0;
		// GL_UNPACK_ALIGNMENT
		GLint unpackAlignment = 4;
	};
	// transform feedback parameters
	struct TransformFeedbackParams {
		// Transform feedback primitive
		GLenum transformFeedbackPrimitive = GL_POINTS;
		// GL_TRANSFORM_FEEDBACK_ACTIVE
		GLboolean transformFeedbackActive = GL_FALSE;
		// GL_TRANSFORM_FEEDBACK_PAUSED
		GLboolean transformFeedbackPaused = GL_FALSE;
	};

public:
	CoreState();
	~CoreState();
	void releaseResources(CoreContext* context);
	void setBuffer(CoreContext* context, GLenum target, CoreBuffer* buffer);
	CoreBuffer* getBuffer(GLenum target) const;
	void setTexture(CoreContext* context, GLenum target, CoreTexture* texture);
	CoreTexture* getTexture(GLenum target) const;
	void setFramebuffer(CoreContext* context, GLenum target, CoreFramebuffer* framebuffer);
	CoreFramebuffer* getFramebuffer(GLenum target) const;
	void setRenderbuffer(CoreContext* context, GLenum target, CoreRenderbuffer* renderbuffer);
	CoreRenderbuffer* getRenderbuffer(GLenum target) const;
	void setVertexArray(CoreContext* context, CoreVertexArray* vertexArray);
	CoreVertexArray* getVertexArray() const;
	void setSampler(CoreContext* context, GLuint unit, CoreSampler* sampler);
	CoreSampler* getSampler(GLuint unit) const;
	void setTransformFeedback(CoreContext* context, GLenum target, CoreTransformFeedback* transformFeedback);
	CoreTransformFeedback* getTransformFeedback(GLenum target) const;
	void setProgram(CoreContext* context, CoreProgram* program);
	CoreProgram* getProgram() const;
	void setCurrentQuery(CoreContext* context, GLenum target, CoreQuery* query);
	CoreQuery* getCurrentQuery(GLenum target) const;
	void setEnableVertexAttrib(GLuint index, GLboolean enable);
	void setVertexAttrib1f(GLuint index, GLfloat x);
	void setVertexAttrib1fv(GLuint index, const GLfloat *v);
	void setVertexAttrib2f(GLuint index, GLfloat x, GLfloat y);
	void setVertexAttrib2fv(GLuint index, const GLfloat *v);
	void setVertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z);
	void setVertexAttrib3fv(GLuint index, const GLfloat *v);
	void setVertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
	void setVertexAttrib4fv(GLuint index, const GLfloat *v);
	void setVertexAttribPointer(CoreContext* context, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer, CoreBuffer* buffer);
	void setVertexAttribIPointer(CoreContext* context, GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer, CoreBuffer* buffer);
	void setVertexAttribI4i(GLuint index, GLint x, GLint y, GLint z, GLint w);
	void setVertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
	void setVertexAttribDivisor(GLuint index, GLuint divisor);
	void getVertexAttribfv(GLuint index, GLenum pname, GLfloat* params);
	void getVertexAttribiv(GLuint index, GLenum pname, GLint* params);
	void getVertexAttribIiv(GLuint index, GLenum pname, GLint* params);
	void getVertexAttribIuiv(GLuint index, GLenum pname, GLuint* params);
	void getVertexAttribPointerv(GLuint index, GLenum pname, void** pointer);
	void setEnable(GLenum cap, GLboolean enable);
	GLboolean getEnable(GLenum cap);
	void setActiveTexture(GLenum texture);
	GLenum getActiveTexture() const;
	void setBlendColor(float red, float green, float blue, float alpha);
	void setBlendEquation(GLenum modeRgb, GLenum modeAlpha);
	void setBlendFunc(GLenum srcRgb, GLenum dstRgb, GLenum srcAlpha, GLenum dstAlpha);
	void setColorClearValue(float red, float green, float blue, float alpha);
	void setDepthClearValue(float d);
	void setStencilClearValue(GLint s);
	void setColorWriteMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
	void setCullFaceMode(GLenum mode);
	void setDepthFunc(GLenum func);
	void setDepthWritemask(GLboolean flag);
	void setDepthRange(float n, float f);
	void setFrontFace(GLenum mode);
	void setLineWidth(float width);
	float getLineWidth() const;
	void setPixelStore(GLenum pname, GLint param);
	void setPolygonOffset(float factor, float units);
	void setSampleCoverage(float value, GLboolean invert);
	void setScissorBox(GLint x, GLint y, GLint width, GLint height);
	void setStencilFunc(GLenum func, GLint ref, GLuint mask);
	void setStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask);
	void setStencilWritemask(GLuint mask);
	void setStencilWritemaskSeparate(GLenum face, GLuint mask);
	void setStencilOp(GLenum fail, GLenum zfail, GLenum zpass);
	void setStencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
	void setViewport(GLint x, GLint y, GLsizei width, GLsizei height);
	void setReadBuffer(GLenum src);
	GLenum getReadBuffer() const;
	void setDrawBuffers(GLsizei n, const GLenum* bufs);
	GLenum getDrawBuffer(GLenum pname);
	void setTransformFeedbackActive(GLboolean active);
	void setTransformFeedbackPrimitive(GLenum primitive);
	void setBufferRange(CoreContext* context, GLenum target, GLuint index, CoreBuffer* buffer, GLintptr offset, GLsizeiptr size);
	void setBufferBase(CoreContext* context, GLenum target, GLuint index, CoreBuffer* buffer);
	void setHint(GLenum target, GLenum mode);
	GLenum getHint(GLenum target) const;
	void clearDrawParameterDirtyFlags();
	void setupTextureSampler(CoreContext* context);
	void setInstancedRendering(bool isInstanced);
	const BlendParams& getBlendParams() const;
	const DepthTestParams& getDepthTestParams() const;
	const StencilTestParams& getStencilTestParams() const;
	const StencilReference& getStencilReference() const;
	const ViewportParams& getViewportParams() const;
	const ClearParameters& getClearParams() const;
	const ScissorParams& getScissorParams() const;
	const CullFaceParams& getCullFaceParams() const;
	const ColorWritemaskParams& getColorWritemaskParams() const;
	const DepthStencilWritemaskParams& getDepthStencilWritemaskParams() const;
	const PackParams& getPackParams() const;
	const UnpackParams& getUnpackParams() const;
	const SampleCoverageParams& getSampleCoverageParams() const;
	const PolygonOffsetParams& getPolygonOffsetParams() const;
	const TransformFeedbackParams& getTransformFeedbackParams() const;
	const IndexedBuffer* getIndexedBuffer(GLenum target, GLuint index) const;
	const DrawParameters& getDrawParameters() const;
	bool elementArrayBufferAvailable() const;

private:
	void setCoreBuffer(CoreContext* context, CoreBuffer** dst, CoreBuffer* buf);
	void setCoreTexture(CoreContext* context, CoreTexture** dst, CoreTexture* tex);
	void setCoreFramebuffer(CoreContext* context, CoreFramebuffer** dst, CoreFramebuffer* framebuffer);
	void setCoreSampler(CoreContext* context, CoreSampler** dst, CoreSampler* sampler);
	void setCoreQuery(CoreContext* context, CoreQuery** dst, CoreQuery* query);
	void setIndexedBuffer(CoreContext* context, IndexedBuffer* dst, CoreBuffer* buf, GLintptr offset, GLsizeiptr size);
	void updatePipelineStateAttachments();
	static bool isIntegerType(GLenum type);

private:
	// Buffer
	CoreBuffer* m_pArrayBuffer = nullptr;
	CoreBuffer* m_pCopyReadBuffer = nullptr;
	CoreBuffer* m_pCopyWriteBuffer = nullptr;
	CoreBuffer* m_pPixelPackBuffer = nullptr;
	CoreBuffer* m_pPixelUnpackBuffer = nullptr;
	CoreBuffer* m_pTransformFeedbackBuffer = nullptr;
	CoreBuffer* m_pUniformBuffer = nullptr;
	IndexedBuffer m_indexedTransformFeedbackBuffer[AXGL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS];
	// Framebuffer
	CoreFramebuffer* m_pReadFramebuffer = nullptr;
	CoreFramebuffer* m_pDrawFramebuffer = nullptr;
	// Renderbuffer
	CoreRenderbuffer* m_pRenderbuffer = nullptr;
	// TransformFeedback
	CoreTransformFeedback* m_pTransformFeedback = nullptr;
	// Query
	CoreQuery* m_pTransformFeedbackPrimitivesWritten = nullptr;
	// enable/disable
	GLboolean m_dither = GL_TRUE;
	GLboolean m_primitiveRestartFixedIndex = GL_FALSE;
	GLboolean m_rasterizerDiscard = GL_FALSE;
	// GL_ACTIVE_TEXTURE
	GLenum m_activeTexture = GL_TEXTURE0;
	// GL_LINE_WIDTH
	float m_lineWidth = 1.0f;
	// GL_READ_BUFFER
	GLenum m_readBuffer = GL_BACK;
	// GL_DRAW_BUFFERi
	GLenum m_drawBuffer[AXGL_MAX_DRAW_BUFFERS];
	ClearParameters m_clearParams;
	PackParams m_packParams;
	UnpackParams m_unpackParams;
	TransformFeedbackParams m_transformFeedbackParams;
	// Hints
	GLenum m_fragmentShaderDerivativeHint = GL_DONT_CARE;
	GLenum m_generateMipmapHint = GL_DONT_CARE;
	// draw parameters
	DrawParameters m_drawParameters;
};

} // namespace axgl

#endif // __CoreState_h_
