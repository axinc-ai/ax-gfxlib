// ContextMetal.h
#ifndef __ContextMetal_h_
#define __ContextMetal_h_
#include "BackendMetal.h"
#include "../BackendContext.h"
#include "../BackendBuffer.h"
#include "../spirv_msl/SpirvMsl.h"
#include "../../AXGLAllocatorImpl.h"
#include <unordered_map>
#include <utility>

namespace axgl {

class FramebufferMetal;
class RenderbufferMetal;
class ProgramMetal;
class VertexArrayMetal;
class BufferMetal;

class ContextMetal : public BackendContext
{
public:
	ContextMetal();
	virtual ~ContextMetal();
	virtual bool initialize() override;
	virtual void terminate() override;
	virtual uint32_t getNumRenderbufferFormat() override;
	virtual const BackendRenderbufferFormat* getRenderbufferFormat() override;
	virtual const PlatformParams& getPlatformParams() override;
	virtual void fenceSync(BackendSync* sync) override;
	virtual void waitSync(BackendSync* sync, GLbitfield flags, GLuint64 timeout) override;
	virtual GLenum clientWaitSync(BackendSync* sync, GLbitfield flags, GLuint64 timeout) override;
	virtual bool clear(GLbitfield clearBits, const DrawParameters* drawParams, const ClearParameters* clearParams) override;
	virtual bool clearBufferiv(GLenum buffer, GLint drawbuffer, const GLint* value, const DrawParameters* drawParams) override;
	virtual bool clearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint* value, const DrawParameters* drawParams) override;
	virtual bool clearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat* value, const DrawParameters* drawParams) override;
	virtual bool clearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil, const DrawParameters* drawParams) override;
	virtual bool drawArrays(GLenum mode, GLint first, GLsizei count, const DrawParameters* drawParams, const ClearParameters* clearParams) override;
	virtual bool drawElements(GLenum mode, GLsizei count, GLenum type, const void* indices, const DrawParameters* drawParams, const ClearParameters* clearParams) override;
	virtual bool drawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount,
		const DrawParameters* drawParams, const ClearParameters* clearParams) override;
	virtual bool drawElementsInstanced(GLenum mode, GLsizei count, GLsizei type, const void* indices, GLsizei instancecount,
		const DrawParameters* drawParams, const ClearParameters* clearParams) override;
	virtual bool flush() override;
	virtual bool finish() override;
	virtual bool readPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type,
		BackendFramebuffer* readFramebuffer, GLenum readBuffer, void* pixels) override;
	virtual void invalidateCache(GLbitfield flags) override;
	virtual void discardCachesAssociatedWithProgram(BackendProgram* program) override;
	virtual void discardCachesAssociatedWithVertexArray(BackendVertexArray* vertexArray) override;

public:
	id<MTLDevice> getDevice() const;
	id<MTLCommandQueue> getCommandQueue() const;
	id<MTLCommandBuffer> getDrawCommandBuffer() const;
	id<MTLBlitCommandEncoder> getBlitCommandEncoder() const;
	MTLCompileOptions* getCompileOptions() const;
	bool presentRenderbuffer(RenderbufferMetal* renderbuffer);
	SpirvMsl* getBackendSpirvMsl();

private:
	// wait mode
	enum WaitMode {
		WaitModeNone = 0,
		WaitModeScheduled = 1,
		WaitModeCompleted = 2
	};
	// VBO update information
	struct VboUpdateInfo {
		BufferMetal* buffer[AXGL_MAX_VERTEX_ATTRIBS];
		uint32_t stride[AXGL_MAX_VERTEX_ATTRIBS];
		uint32_t alignedStride[AXGL_MAX_VERTEX_ATTRIBS];
		BackendBuffer::ConversionMode conversion;
		GLint first;
		GLsizei count;
		bool useBlit;
	};
	// UBO update information
	struct UboUpdateInfo {
		BufferMetal* buffer[AXGL_MAX_UNIFORM_BUFFER_BINDINGS];
	};
	// IBO update information
	struct IboUpdateInfo {
		BufferMetal* buffer;
		BackendBuffer::ConversionMode conversion;
		intptr_t iboOffset;
		intptr_t iboSize;
		bool isUbyte;
		bool useBlit;
	};
	// VBO dynamic update information
	struct VboDynamicUpdateInfo {
		// NOTE: 配列はBackendProgramMetal::getAttribLocations()のロケーションで参照したバッファを格納
		BufferMetal* buffer[AXGL_MAX_VERTEX_ATTRIBS];
		id<MTLBuffer> dynamicBuffer[AXGL_MAX_VERTEX_ATTRIBS];
		size_t offset[AXGL_MAX_VERTEX_ATTRIBS];
		bool useDynamicBuffer;
	};
	// UBO dynamic update information
	struct UboDynamicUpdateInfo {
		// NOTE: 配列はGLのuniform bufferバインド順でバッファを格納
		BufferMetal* buffer[AXGL_MAX_UNIFORM_BUFFER_BINDINGS];
		id<MTLBuffer> dynamicBuffer[AXGL_MAX_UNIFORM_BUFFER_BINDINGS];
		size_t offset[AXGL_MAX_UNIFORM_BUFFER_BINDINGS];
		bool useDynamicBuffer;
	};
	// IBO dynamic update information
	struct IboDynamicUpdateInfo {
		BufferMetal* buffer;
		id<MTLBuffer> dynamicBuffer;
		size_t offset;
		bool useDynamicBuffer;
	};
	
private:
	bool checkVBOUpdate(VboUpdateInfo* updateInfo, VboDynamicUpdateInfo* dynamicUpdateInfo, const DrawParameters* drawParams,
		BackendBuffer::ConversionMode conversion, GLint first, GLsizei count);
	bool checkUBOUpdate(UboUpdateInfo* updateInfo, UboDynamicUpdateInfo* dynamicUpdateInfo, const DrawParameters* drawParams);
	bool checkIBOUpdate(IboUpdateInfo* updateInfo, IboDynamicUpdateInfo* dynamicUpdateInfo, const DrawParameters* drawParams,
		BackendBuffer::ConversionMode conversion, intptr_t iboOffset, intptr_t iboSize, bool isUbyte);
	void updateVBO(const VboUpdateInfo* updateInfo);
	void updateUBO(const UboUpdateInfo* updateInfo);
	void updateIBO(const IboUpdateInfo* updateInfo);
	void updateDynamicBuffers(VboDynamicUpdateInfo* vboInfo, UboDynamicUpdateInfo* uboInfo, IboDynamicUpdateInfo* iboInfo);
	void setupDrawCommandBuffer();
	void commitDrawCommandBuffer(WaitMode waitMode);
	bool setupRenderCommandEncoder(MTLRenderPassDescriptor* renderPassDesc);
	void endRenderCommandEncoder();
	void setupBlitCommandEncoder();
	void setBlitSignalEvent();
	void endBlitCommandEncoder();
	void endCommandEncoder();
	void setupDefaultUniformBuffer(size_t size);
	void setupDynamicBuffer(size_t size);
	void setBufferForDefaultUniform(id<MTLRenderCommandEncoder> encoder,
		int32_t vsIndex, int32_t fsIndex, const void* data, size_t size);
	void setDefaultUniformBuffer(id<MTLRenderCommandEncoder> encoder, const ProgramMetal* program);
	static void setDepthStencilDescriptor(MTLDepthStencilDescriptor* dsDesc,
		const DepthStencilState& dsState);
	static bool viewportNeedYFlip(const DrawParameters* drawParams);
	static void getFramebufferSize(const DrawParameters* drawParams, RectSize* rectSize);
	id<MTLRenderPipelineState> setupRenderPipelineState(const DrawParameters* drawParams,
		const uint32_t* adjustedStride, bool* sameAsLastUsed);
	id<MTLDepthStencilState> setupDepthStencilState(const DrawParameters* drawParams,
		bool* sameAsLastUsed);
	MTLRenderPassDescriptor* setupRenderPassDescriptor(const DrawParameters* drawParams,
		const ClearParameters* clearParams, bool* changed);
	void setViewport(id<MTLRenderCommandEncoder> encoder, const ViewportParams* viewport, bool flipY);
	void setVertexBufferForVBO(id<MTLRenderCommandEncoder> encoder, const CoreBuffer* const* vertexBuffers,
		const ProgramMetal* program, const VertexAttrib* vertexAttribs, const VboDynamicUpdateInfo* dynamicUpdateInfo);
	void setVertexBufferForVBOFromVAO(id<MTLRenderCommandEncoder> encoder, VertexArrayMetal* vertexArray,
		const ProgramMetal* program, bool setAll, const VboDynamicUpdateInfo* dynamicUpdateInfo);
	void setBufferForUBO(id<MTLRenderCommandEncoder> encoder, const ProgramMetal* program,
		const IndexedBuffer* uniformBuffers, bool setAll, const UboDynamicUpdateInfo* dynamicUpdateInfo);
	void setTextureAndSampler(id<MTLRenderCommandEncoder> encoder, const ProgramMetal* program,
		const DrawParameters* drawParams, bool setAll);
	void setVisibilityResultMode(id<MTLRenderCommandEncoder> encoder, const DrawParameters* drawParams,
		bool setAll);
	void setCullMode(id<MTLRenderCommandEncoder> encoder, const CullFaceParams* cullFaceParams, bool flipY);
	void setScissor(id<MTLRenderCommandEncoder> encoder, const ScissorParams* scissorParams,
		const RectSize* rectSize, bool flipY);
	void setDepthBias(id<MTLRenderCommandEncoder> encoder, const PolygonOffsetParams* polygonOffsetParams);
	void setStencilReference(id<MTLRenderCommandEncoder> encoder, const StencilReference* stencilReference);
	void setVertexDescriptorFromCurrentState(MTLRenderPipelineDescriptor* pipelineDesc, const VertexAttrib* vertexAttribs,
		const int32_t* locations, bool isInstanced, const ProgramMetal* program, const uint32_t* adjustedStride);
	 void setVertexDescriptorFromVAO(MTLRenderPipelineDescriptor* pipelineDesc, const VertexArrayMetal* vertexArray,
		const int32_t* locations, bool isInstanced, const ProgramMetal* program, const uint32_t* adjustedStride);
	static BackendBuffer::ConversionMode getIboConversionMode(GLenum mode, GLenum type);
	bool updatePipelineStateUsedOrder(const PipelineState* pipelineState);
	bool updateDepthStencilStateUsedOrder(const DepthStencilState* depthStencilState);

private:
	// hash関数を指定したunordered_map
	using PipelineStateMap = std::unordered_map<PipelineState, id<MTLRenderPipelineState>, PipelineState::Hash, std::equal_to<PipelineState>, AXGLStlAllocator<std::pair<const PipelineState, id<MTLRenderPipelineState>>>>;
	using DepthStencilStateMap = std::unordered_map<DepthStencilState, id<MTLDepthStencilState>, DepthStencilState::Hash, std::equal_to<DepthStencilState>, AXGLStlAllocator<std::pair<const DepthStencilState, id<MTLDepthStencilState>>>>;
	id<MTLDevice> m_mtlDevice = nil;
	id<MTLBuffer> m_disableBuffer = nil;
	id<MTLCommandQueue> m_commandQueue = nil;
	id<MTLCommandBuffer> m_drawCommandBuffer = nil;
	id<MTLRenderCommandEncoder> m_renderCommandEncoder = nil; // 描画用
	id<MTLBlitCommandEncoder> m_blitCommandEncoder = nil; // Blit用
	id<MTLBuffer> m_defaultUniformBuffer = nil;
	size_t m_defaultUniformBufferOffset = 0;
	id<MTLBuffer> m_dynamicBuffer = nil;
	size_t m_dynamicBufferOffset = 0;
	MTLCompileOptions* m_compileOptions = nil;
	PipelineStateMap m_pipelineStateCache;
	AXGLList<const PipelineState*> m_pipelineStateUsedOrder;
	DepthStencilStateMap m_depthStencilStateCache;
	AXGLList<const DepthStencilState*> m_depthStencilStateUsedOrder;
	FramebufferMetal* m_renderFramebuffer = nullptr;
	bool m_setDrawParameterToEncoder = false;
	SpirvMsl m_spirvMsl;
};

} // namespace axgl

#endif // __ContextMetal_h_
