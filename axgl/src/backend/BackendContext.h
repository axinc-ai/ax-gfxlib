// BackendContext.h
#ifndef __BackendContext_h_
#define __BackendContext_h_

#include "../common/axglCommon.h"
#include "../common/DrawParameters.h"
#include "../common/ClearParameters.h"

namespace axgl {

struct DrawParameters;
struct ClearParameters;
struct BackendRenderbufferFormat;
class BackendSync;
class BackendFramebuffer;
class BackendProgram;
class BackendVertexArray;

class BackendContext
{
public:
	struct PlatformParams
	{
		float aliasedLineWidthRange[2];
		float aliasedPointSizeRange[2];
		const GLenum* compressedTextureFormats;
		GLenum implementationColorReadFormat;
		GLenum implementationColorReadType;
		GLint max3dTextureSize;
		GLint maxArrayTextureLayers;
		GLint maxColorAttachments;
		GLint maxCombinedFragmentUniformComponents;
		GLint maxCombinedTextureImageUnits;
		GLint maxCombinedUniformBlocks;
		GLint maxCombinedVertexUniformComponents;
		GLint maxCubeMapTextureSize;
		GLint maxDrawBuffers;
		GLint maxElementIndex;
		GLint maxElementsIndices;
		GLint maxElementsVertices;
		GLint maxFragmentInputComponents;
		GLint maxFragmentUniformBlocks;
		GLint maxFragmentUniformComponents;
		GLint maxFragmentUniformVectors;
		GLint maxProgramTexelOffset;
		GLint maxRenderbufferSize;
		GLint maxSamples;
		GLuint64 maxServerWaitTimeout;
		GLint maxTextureImageUnits;
		float maxTextureLodBias;
		GLint maxTextureSize;
		GLint maxTransformFeedbackInterleavedComponents;
		GLint maxTransformFeedbackSeparateAttribs;
		GLint maxTransformFeedbackSeparateComponents;
		GLint maxUniformBlockSize;
		GLint maxUniformBufferBindings;
		GLint maxVaryingComponents;
		GLint maxVaryingVectors;
		GLint maxVertexAttribs;
		GLint maxVertexTextureImageUnits;
		GLint maxVertexOutputComponents;
		GLint maxVertexUniformBlocks;
		GLint maxVertexUniformComponents;
		GLint maxVertexUniformVectors;
		GLint maxViewportDims[2];
		GLint minProgramTexelOffset;
		GLint numCompressedTextureFormats;
		GLint numExtensions;
		GLint numProgramBinaryFormats;
		GLint numShaderBinaryFormats;
		const GLenum* programBinaryFormats;
		const GLenum* shaderBinaryFormats;
		GLint subpixelBits;
		GLint uniformBufferOffsetAlignment;
	};

public:
	virtual ~BackendContext() {}
	virtual bool initialize() = 0;
	virtual void terminate() = 0;
	virtual uint32_t getNumRenderbufferFormat() = 0;
	virtual const BackendRenderbufferFormat* getRenderbufferFormat() = 0;
	virtual const PlatformParams& getPlatformParams() = 0;
	virtual void fenceSync(BackendSync* sync) = 0;
	virtual void waitSync(BackendSync* sync, GLbitfield flags, GLuint64 timeout) = 0;
	virtual GLenum clientWaitSync(BackendSync* sync, GLbitfield flags, GLuint64 timeout) = 0;
	virtual bool clear(GLbitfield clearBits, const DrawParameters* drawParams, const ClearParameters* clearParams) = 0;
	virtual bool clearBufferiv(GLenum buffer, GLint drawbuffer, const GLint* value, const DrawParameters* drawParams) = 0;
	virtual bool clearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint* value, const DrawParameters* drawParams) = 0;
	virtual bool clearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat* value, const DrawParameters* drawParams) = 0;
	virtual bool clearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil, const DrawParameters* drawParams) = 0;
	virtual bool drawArrays(GLenum mode, GLint first, GLsizei count, const DrawParameters* drawParams, const ClearParameters* clearParams) = 0;
	virtual bool drawElements(GLenum mode, GLsizei count, GLenum type, const void* indices, const DrawParameters* drawParams, const ClearParameters* clearParams) = 0;
	virtual bool drawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount,
		const DrawParameters* drawParams, const ClearParameters* clearParams) = 0;
	virtual bool drawElementsInstanced(GLenum mode, GLsizei count, GLsizei type, const void* indices, GLsizei instancecount,
		const DrawParameters* drawParams, const ClearParameters* clearParams) = 0;
	virtual bool flush() = 0;
	virtual bool finish() = 0;
	virtual bool readPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type,
		BackendFramebuffer* readFramebuffer, GLenum readBuffer, void* pixels) = 0;
	virtual void invalidateCache(GLbitfield flags) = 0;
	virtual void discardCachesAssociatedWithProgram(BackendProgram* program) = 0;
	virtual void discardCachesAssociatedWithVertexArray(BackendVertexArray* vertexArray) = 0;

public:
	static BackendContext* create();
	static void destroy(BackendContext* context);
};

} // namespace axgl

#endif // __BackendContext_h_
