// BackendContextMetal.mm
#if defined(__APPLE_CC__)
#include "ContextMetal.h"
#include "BufferMetal.h"
#include "FramebufferMetal.h"
#include "RenderbufferMetal.h"
#include "ProgramMetal.h"
#include "QueryMetal.h"
#include "SamplerMetal.h"
#include "ShaderMetal.h"
#include "SyncMetal.h"
#include "TextureMetal.h"
#include "VertexArrayMetal.h"
#include "../../AXGLAllocatorImpl.h"
#include "../../core/CoreBuffer.h"
#include "../../core/CoreFramebuffer.h"
#include "../../core/CoreQuery.h"
#include "../../core/CoreSampler.h"
#include "../../core/CoreTexture.h"
#include "../../core/CoreVertexArray.h"

#include <algorithm>

namespace axgl {

#if TARGET_IPHONE_SIMULATOR
// アライメントはMacOS Metalの256そのままになっている
static inline size_t get_aligned_buffer_offset(size_t offset)
{
	return ((offset + 255) / 256) * 256;
}
#else
static inline size_t get_aligned_buffer_offset(size_t offset)
{
	return ((offset + 15) / 16) * 16;
}
#endif // TARGET_IPHONE_SIMULATOR

// インデックスのサイズ(バイト数)を取得
static inline intptr_t get_indices_size(GLsizei count, GLenum type)
{
	intptr_t size = 0;
	switch (type) {
	case GL_UNSIGNED_BYTE:
		size = count;
		break;
	case GL_UNSIGNED_SHORT:
		size = count * 2;
		break;
	case GL_UNSIGNED_INT:
		size = count * 4;
		break;
	default:
		AXGL_ASSERT(0);
		break;
	}
	return size;
}

// ミップマップのサイズを算出
static uint32_t calc_mipmap_size(uint32_t size0, int level)
{
	uint32_t size = size0;
	for (int i = 0; i < level; i++) {
		size /= 2;
	}
	if (size < 1) {
		size = 1;
	}
	return size;
}

// バッファのアライメントされたストライドを取得
static inline uint32_t get_aligned_stride(uint32_t val)
{
	return ((val + 3) / 4) * 4; // 4バイトアライメント
}

// サンプラのタイプから使用するテクスチャを取得する
static const CoreTexture* get_core_texture(const DrawParameters* drawParams, int32_t type, int index)
{
	const CoreTexture* texture = nullptr;
	AXGL_ASSERT(drawParams != nullptr);
	AXGL_ASSERT(index < AXGL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
	switch (type) {
	case GL_SAMPLER_2D:
	case GL_SAMPLER_2D_SHADOW:
		texture = drawParams->texture2d[index];
		break;
	case GL_SAMPLER_CUBE:
	case GL_SAMPLER_CUBE_SHADOW:
		texture = drawParams->textureCube[index];
		break;
	case GL_SAMPLER_3D:
		texture = drawParams->texture3d[index];
		break;
	case GL_SAMPLER_2D_ARRAY:
	case GL_SAMPLER_2D_ARRAY_SHADOW:
		texture = drawParams->texture2dArray[index];
		break;
	default:
		break;
	}
	return texture;
}

// BackendContextクラスの実装 --------
BackendContext* BackendContext::create()
{
	ContextMetal* context = AXGL_NEW(ContextMetal);
	return context;
}
	
void BackendContext::destroy(BackendContext* context)
{
	if (context == nullptr) {
		return;
	}
	AXGL_DELETE(context);
	return;
}

// BackendContextMetalクラスの実装 --------
static constexpr GLint c_rgba8_samples[] = {1,2,4,8};
static constexpr BackendRenderbufferFormat c_rbformat[] = {
	{GL_RGBA8, sizeof(c_rgba8_samples)/sizeof(GLint), c_rgba8_samples}
};
static constexpr size_t c_default_uniform_buffer_size = (8 * 1024 * 1024); // 8MB
static constexpr size_t c_dynamic_buffer_size = (8 * 1024 * 1024); // 8MB
static constexpr size_t c_pipeline_state_cache_max = 512;
static constexpr size_t c_depth_stencil_state_cache_max = 64;
static constexpr uint32_t c_vbo_index_offset = AXGL_MAX_UNIFORM_BUFFER_BINDINGS;

static constexpr BackendContext::PlatformParams c_platformParams = {
	{1.0f,1.0f}, // aliasedLineWidthRange
	{1.0f,511.0f}, // aliasedPointSizeRange
	nullptr, // TODO: compressedTextureFormats
	0, // TODO: implementationColorReadFormat
	0, // TODO: implementationColorReadType
	2048, // max3dTextureSize
	2048, // maxArrayTextureLayers
	4,    // maxColorAttachments
	0, // TODO: maxCombinedFragmentUniformComponents
	0, // TODO: maxCombinedTextureImageUnits
	0, // TODO: maxCombinedUniformBlocks
	0, // TODO: maxCombinedVertexUniformComponents
	8192, // maxCubeMapTextureSize
	0,//TODO : GLint maxDrawBuffers;
	0,//TODO : GLint maxElementIndex;
	0,//TODO : GLint maxElementsIndices;
	0,//TODO : GLint maxElementsVertices;
	0,//TODO : GLint maxFragmentInputComponents;
	0,//TODO : GLint maxFragmentUniformBlocks;
	0,//TODO : GLint maxFragmentUniformComponents;
	0,//TODO : GLint maxFragmentUniformVectors;
	0,//TODO : GLint maxProgramTexelOffset;
	0,//TODO : GLint maxRenderbufferSize;
	0,//TODO : GLint maxSamples;
	0,//TODO : GLuint64 maxServerWaitTimeout;
	0,//TODO : GLint maxTextureImageUnits;
	0.0f,//TODO : float maxTextureLodBias;
	8192,//GLint maxTextureSize;
	0,//TODO : GLint maxTransformFeedbackInterleavedComponents;
	0,//TODO : GLint maxTransformFeedbackSeparateAttribs;
	0,//TODO : GLint maxTransformFeedbackSeparateComponents;
	0,//TODO : GLint maxUniformBlockSize;
	0,//TODO : GLint maxUniformBufferBindings;
	0,//TODO : GLint maxVaryingComponents;
	0,//TODO : GLint maxVaryingVectors;
	0,//TODO : GLint maxVertexAttribs;
	0,//TODO : GLint maxVertexTextureImageUnits;
	0,//TODO : GLint maxVertexOutputComponents;
	0,//TODO : GLint maxVertexUniformBlocks;
	0,//TODO : GLint maxVertexUniformComponents;
	128,// GLint maxVertexUniformVectors;
	{0,0},//TODO : GLint maxViewportDims[2];
	0,//TODO : GLint minProgramTexelOffset;
	0,//TODO : GLint numCompressedTextureFormats;
	0,//TODO : GLint numExtensions;
	0,//TODO : GLint numProgramBinaryFormats;
	0,//TODO : GLint numShaderBinaryFormats;
	nullptr,//TODO : const GLenum* programBinaryFormats;
	nullptr,//TODO : const GLenum* shaderBinaryFormats;
	0,//TODO : GLint subpixelBits;
	0 //TODO : GLint uniformBufferOffsetAlignment;
};

// コンストラクタ
ContextMetal::ContextMetal()
{
}

// デストラクタ
ContextMetal::~ContextMetal()
{
}

// 初期化
bool ContextMetal::initialize()
{
	// デバイスを作成
	m_mtlDevice = MTLCreateSystemDefaultDevice();
	if (m_mtlDevice == nil) {
		return false;
	}
	// コマンドキューを作成
	m_commandQueue = [m_mtlDevice newCommandQueue];
	if (m_commandQueue == nil) {
		return false;
	}
	// MSLのコンパイルオプションを作成
	m_compileOptions = [[MTLCompileOptions alloc] init];
	if (m_compileOptions == nil) {
		return false;
	}
	// コンパイルオプションは仮設定
	m_compileOptions.fastMathEnabled = YES;
	m_compileOptions.languageVersion = MTLLanguageVersion2_0;
	// 描画パラメータ設定をクリアしておく
	m_setDrawParameterToEncoder = false;
	// Disableの頂点属性に設定するための固定値バッファを用意
	{
		// NOTE: float/uint/int のスカラーとベクトルで使用することを想定して 16 バイトの 0 でフィル
		// 4 byte * 4 component : 16 byte
		static constexpr uint8_t c_zero[16] = {0};
		m_disableBuffer = [m_mtlDevice newBufferWithBytes:c_zero length:sizeof(c_zero) options:MTLResourceStorageModeShared];
		if (m_disableBuffer == nil) {
			return false;
		}
	}
	// glslangを初期化
	// NOTE: プロセスで１回初期化すれば良く、static変数を参照して呼び出すようにしたほうが良いかも
	m_spirvMsl.initialize();
	return true;
}

// 終了処理
void ContextMetal::terminate()
{
	m_defaultUniformBuffer = nil;
	m_drawCommandBuffer = nil;
	m_renderCommandEncoder = nil;
	m_blitCommandEncoder = nil;
	m_disableBuffer = nil;
	m_compileOptions = nil;
	m_commandQueue = nil;
	m_mtlDevice = nil;
	// glslangを終了
	// NOTE: プロセスの最後で１回呼ぶ、static変数を参照して呼び出すようにしたほうが良いかも
	m_spirvMsl.terminate();
	return;
}

// Renderbufferフォーマット数を取得
uint32_t ContextMetal::getNumRenderbufferFormat()
{
	return sizeof(c_rbformat)/sizeof(BackendRenderbufferFormat);
}

// Renderbufferフォーマットを取得
const BackendRenderbufferFormat* ContextMetal::getRenderbufferFormat()
{
	return c_rbformat;
}

// プラットフォーム固有のパラメータを取得
const BackendContext::PlatformParams& ContextMetal::getPlatformParams()
{
	return c_platformParams;
}

// Syncオブジェクトの同期を設定
void ContextMetal::fenceSync(BackendSync* sync)
{
	if (sync == nullptr) {
		return;
	}
	if (m_commandQueue == nil) {
		return;
	}
	// 描画コマンドバッファが存在している場合はScheduled待ち
	commitDrawCommandBuffer(WaitModeScheduled);
	// 新たにコマンドバッファを作成しCompleted handlerを設定（描画以外のコマンドを考慮）
	SyncMetal* sync_metal = static_cast<SyncMetal*>(sync);
	sync_metal->setStatus(GL_UNSIGNALED);
	// GL_SYNC_GPU_COMMANDS_COMPLETE用のハンドラ
	void (^completed_func)(id<MTLCommandBuffer>) = ^(id<MTLCommandBuffer> cmdBuffer){
		sync_metal->setStatus(GL_SIGNALED);
	};
	id<MTLCommandBuffer> command_buffer = [m_commandQueue commandBuffer];
	if (command_buffer != nil) {
		[command_buffer addCompletedHandler:completed_func];
		[command_buffer commit];
	}
	return;
}

// Syncオブジェクトの同期を待つ(glWaitSync相当)
void ContextMetal::waitSync(BackendSync* sync, GLbitfield flags, GLuint64 timeout)
{
	if ((sync == nullptr) || (m_commandQueue == nil)) {
		return;
	}
	AXGL_UNUSED(flags);
	AXGL_UNUSED(timeout);
	GLint stat = sync->getStatus();
	while (stat != GL_SIGNALED) {
		stat = sync->getStatus();
	}
	return;
}

// Syncオブジェクトの同期を待つ(glClientWaitSync相当)
GLenum ContextMetal::clientWaitSync(BackendSync* sync, GLbitfield flags, GLuint64 timeout)
{
	if ((sync == nullptr) || (m_commandQueue == nil)) {
		return GL_WAIT_FAILED;
	}
	AXGL_UNUSED(flags);
	GLenum rval = GL_CONDITION_SATISFIED;
	GLint stat = sync->getStatus();
	if (stat == GL_SIGNALED) {
		rval = GL_ALREADY_SIGNALED;
	} else {
		volatile GLint sync_stat = stat;
		volatile double cur_time = [[NSDate date] timeIntervalSince1970];
		// timeoutの単位は[nanoseconds]
		const double end_time = cur_time + ((double)timeout * 0.000000001);
		while ((sync_stat != GL_SIGNALED) && (cur_time < end_time)) {
			[NSThread sleepForTimeInterval:0.0005]; // 500us sleep
			sync_stat = sync->getStatus();
			cur_time = [[NSDate date] timeIntervalSince1970];
		}
		if (sync_stat != GL_SIGNALED) {
			rval = GL_TIMEOUT_EXPIRED;
		}
	}
	return rval;
}

// クリアを実行する(glClear相当)
bool ContextMetal::clear(GLbitfield clearBits, const DrawParameters* drawParams, const ClearParameters* clearParams)
{
	AXGL_ASSERT(drawParams != nullptr);
	// 描画コマンドバッファを用意
	setupDrawCommandBuffer();
	AXGL_ASSERT(m_drawCommandBuffer != nil);
	// RenderPassDescriptorを用意
	bool render_pass_modified = false;
	MTLRenderPassDescriptor* render_pass_desc = nil;
	FramebufferMetal* framebuffer_metal = nullptr;
	if (drawParams->framebufferDraw != nullptr) {
		framebuffer_metal = static_cast<FramebufferMetal*>(drawParams->framebufferDraw->getBackendFramebuffer());
		if (framebuffer_metal != nullptr) {
			QueryMetal* query_metal = nullptr;
			if (drawParams->anySamplesPassedQuery != nullptr) {
				query_metal = static_cast<QueryMetal*>(drawParams->anySamplesPassedQuery->getBackendQuery());
			} else if (drawParams->anySamplesPassedConservativeQuery != nullptr) {
				query_metal = static_cast<QueryMetal*>(drawParams->anySamplesPassedConservativeQuery->getBackendQuery());
			}
			render_pass_modified = framebuffer_metal->setupRenderPassDescriptor(query_metal, clearBits, clearParams);
			render_pass_desc = framebuffer_metal->getMtlRenderPassDescriptor();
			AXGL_ASSERT(render_pass_desc != nil);
		}
	}
	// 使用したフレームバッファを保持
	m_renderFramebuffer = framebuffer_metal;
	// MTLDepthStencilStateを用意
	bool same_as_last = false;
	id<MTLDepthStencilState> depth_stencil_state = setupDepthStencilState(drawParams, &same_as_last);
	AXGL_ASSERT(depth_stencil_state != nil);
	// レンダーパスのクリアで実行するため、古いEncoderを終了して新たにEncoderを作成
	endRenderCommandEncoder();
	setupRenderCommandEncoder(render_pass_desc);
	id<MTLRenderCommandEncoder> command_encoder = m_renderCommandEncoder;
	[command_encoder setDepthStencilState:depth_stencil_state];
	return true;
}

// クリアを実行する(glClearBufferiv相当)
bool ContextMetal::clearBufferiv(GLenum buffer, GLint drawbuffer, const GLint* value, const DrawParameters* drawParams)
{
	AXGL_ASSERT((buffer == GL_COLOR) || (buffer == GL_STENCIL));
	AXGL_ASSERT((value != nullptr) && (drawParams != nullptr));
	// 描画コマンドバッファを用意
	setupDrawCommandBuffer();
	AXGL_ASSERT(m_drawCommandBuffer != nil);
	// RenderPassDescriptorを用意
	bool render_pass_modified = false;
	MTLRenderPassDescriptor* render_pass_desc = nil;
	FramebufferMetal* framebuffer_metal = nullptr;
	if (drawParams->framebufferDraw != nullptr) {
		framebuffer_metal = static_cast<FramebufferMetal*>(drawParams->framebufferDraw->getBackendFramebuffer());
		if (framebuffer_metal != nullptr) {
			render_pass_modified = framebuffer_metal->setupRenderPassDescriptorForClearI(buffer, drawbuffer, value);
			render_pass_desc = framebuffer_metal->getMtlRenderPassDescriptor();
			AXGL_ASSERT(render_pass_desc != nil);
		}
	}
	// 使用したフレームバッファを保持
	m_renderFramebuffer = framebuffer_metal;
	// MTLDepthStencilStateを用意
	bool same_as_last = false;
	id<MTLDepthStencilState> depth_stencil_state = setupDepthStencilState(drawParams, &same_as_last);
	AXGL_ASSERT(depth_stencil_state != nil);
	// レンダーパスのクリアで実行するため、古いEncoderを終了して新たにEncoderを作成
	endRenderCommandEncoder();
	setupRenderCommandEncoder(render_pass_desc);
	id<MTLRenderCommandEncoder> command_encoder = m_renderCommandEncoder;
	[command_encoder setDepthStencilState:depth_stencil_state];
	return true;
}

// クリアを実行する(glClearBufferuiv相当)
bool ContextMetal::clearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint* value, const DrawParameters* drawParams)
{
	AXGL_ASSERT(buffer == GL_COLOR);
	AXGL_ASSERT((value != nullptr) && (drawParams != nullptr));
	// 描画コマンドバッファを用意
	setupDrawCommandBuffer();
	AXGL_ASSERT(m_drawCommandBuffer != nil);
	// RenderPassDescriptorを用意
	bool render_pass_modified = false;
	MTLRenderPassDescriptor* render_pass_desc = nil;
	FramebufferMetal* framebuffer_metal = nullptr;
	if (drawParams->framebufferDraw != nullptr) {
		framebuffer_metal = static_cast<FramebufferMetal*>(drawParams->framebufferDraw->getBackendFramebuffer());
		if (framebuffer_metal != nullptr) {
			render_pass_modified = framebuffer_metal->setupRenderPassDescriptorForClearUI(buffer, drawbuffer, value);
			render_pass_desc = framebuffer_metal->getMtlRenderPassDescriptor();
			AXGL_ASSERT(render_pass_desc != nil);
		}
	}
	// 使用したフレームバッファを保持
	m_renderFramebuffer = framebuffer_metal;
	// MTLDepthStencilStateを用意
	bool same_as_last = false;
	id<MTLDepthStencilState> depth_stencil_state = setupDepthStencilState(drawParams, &same_as_last);
	AXGL_ASSERT(depth_stencil_state != nil);
	// レンダーパスのクリアで実行するため、古いEncoderを終了して新たにEncoderを作成
	endRenderCommandEncoder();
	setupRenderCommandEncoder(render_pass_desc);
	id<MTLRenderCommandEncoder> command_encoder = m_renderCommandEncoder;
	[command_encoder setDepthStencilState:depth_stencil_state];
	return true;
}

// クリアを実行する(glClearBufferfv相当)
bool ContextMetal::clearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat* value, const DrawParameters* drawParams)
{
	AXGL_ASSERT((buffer == GL_COLOR) || (buffer == GL_DEPTH));
	AXGL_ASSERT((value != nullptr) && (drawParams != nullptr));
	// 描画コマンドバッファを用意
	setupDrawCommandBuffer();
	AXGL_ASSERT(m_drawCommandBuffer != nil);
	// RenderPassDescriptorを用意
	bool render_pass_modified = false;
	MTLRenderPassDescriptor* render_pass_desc = nil;
	FramebufferMetal* framebuffer_metal = nullptr;
	if (drawParams->framebufferDraw != nullptr) {
		framebuffer_metal = static_cast<FramebufferMetal*>(drawParams->framebufferDraw->getBackendFramebuffer());
		if (framebuffer_metal != nullptr) {
			render_pass_modified = framebuffer_metal->setupRenderPassDescriptorForClearF(buffer, drawbuffer, value);
			render_pass_desc = framebuffer_metal->getMtlRenderPassDescriptor();
			AXGL_ASSERT(render_pass_desc != nil);
		}
	}
	// 使用したフレームバッファを保持
	m_renderFramebuffer = framebuffer_metal;
	// MTLDepthStencilStateを用意
	bool same_as_last = false;
	id<MTLDepthStencilState> depth_stencil_state = setupDepthStencilState(drawParams, &same_as_last);
	AXGL_ASSERT(depth_stencil_state != nil);
	// レンダーパスのクリアで実行するため、古いEncoderを終了して新たにEncoderを作成
	endRenderCommandEncoder();
	setupRenderCommandEncoder(render_pass_desc);
	id<MTLRenderCommandEncoder> command_encoder = m_renderCommandEncoder;
	[command_encoder setDepthStencilState:depth_stencil_state];
	return true;
}

// クリアを実行する(glClearBufferfi相当)
bool ContextMetal::clearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil, const DrawParameters* drawParams)
{
	AXGL_ASSERT(buffer == GL_DEPTH_STENCIL);
	AXGL_ASSERT(drawParams != nullptr);
	// 描画コマンドバッファを用意
	setupDrawCommandBuffer();
	AXGL_ASSERT(m_drawCommandBuffer != nil);
	// RenderPassDescriptorを用意
	bool render_pass_modified = false;
	MTLRenderPassDescriptor* render_pass_desc = nil;
	FramebufferMetal* framebuffer_metal = nullptr;
	if (drawParams->framebufferDraw != nullptr) {
		framebuffer_metal = static_cast<FramebufferMetal*>(drawParams->framebufferDraw->getBackendFramebuffer());
		if (framebuffer_metal != nullptr) {
			render_pass_modified = framebuffer_metal->setupRenderPassDescriptorForClearFI(buffer, drawbuffer, depth, stencil);
			render_pass_desc = framebuffer_metal->getMtlRenderPassDescriptor();
			AXGL_ASSERT(render_pass_desc != nil);
		}
	}
	// 使用したフレームバッファを保持
	m_renderFramebuffer = framebuffer_metal;
	// MTLDepthStencilStateを用意
	bool same_as_last = false;
	id<MTLDepthStencilState> depth_stencil_state = setupDepthStencilState(drawParams, &same_as_last);
	AXGL_ASSERT(depth_stencil_state != nil);
	// レンダーパスのクリアで実行するため、古いEncoderを終了して新たにEncoderを作成
	endRenderCommandEncoder();
	setupRenderCommandEncoder(render_pass_desc);
	id<MTLRenderCommandEncoder> command_encoder = m_renderCommandEncoder;
	[command_encoder setDepthStencilState:depth_stencil_state];
	return true;
}

// インデックスなしの描画を実行(glDrawArrays相当)
bool ContextMetal::drawArrays(GLenum mode, GLint first, GLsizei count, const DrawParameters* drawParams, const ClearParameters* clearParams)
{
	AXGL_ASSERT((drawParams != nullptr) && (clearParams != nullptr));
	// TriangleFanの場合は変換処理が必要
	BackendBuffer::ConversionMode vbo_conversion = BackendBuffer::ConversionModeNone;
	if (mode == GL_TRIANGLE_FAN) {
		vbo_conversion = BackendBuffer::ConversionModeTriFanVertices;
	}
	// 各バッファの更新をチェックする
	VboUpdateInfo vbo_update_info;
	UboUpdateInfo ubo_update_info;
	VboDynamicUpdateInfo vbo_dynamic_update_info;
	UboDynamicUpdateInfo ubo_dynamic_update_info;
	bool vbo_update = checkVBOUpdate(&vbo_update_info, &vbo_dynamic_update_info, drawParams, vbo_conversion, first, count);
	bool ubo_update = checkUBOUpdate(&ubo_update_info, &ubo_dynamic_update_info, drawParams);
	// 描画コマンドバッファを用意
	setupDrawCommandBuffer();
	AXGL_ASSERT(m_drawCommandBuffer != nil);
	// バッファ更新がBlitコマンドを必要とするか
	bool use_blit_command = (vbo_update && vbo_update_info.useBlit) || ubo_update;
	if (use_blit_command) {
		// Blit command encoder を作成、Render command encoder が使用されている場合は終了される
		setupBlitCommandEncoder();
	}
	// 各バッファの更新処理
	if (vbo_update) {
		updateVBO(&vbo_update_info);
	}
	if (ubo_update) {
		updateUBO(&ubo_update_info);
	}
	if (use_blit_command) {
		// 描画を実行するためBlit command encoderを終了
		endBlitCommandEncoder();
	}
	// 動的バッファ用のMTLBufferを更新
	updateDynamicBuffers(&vbo_dynamic_update_info, &ubo_dynamic_update_info, nullptr);
	// MTLRenderPipelineStateを用意
	bool same_as_last_ps = false;
	id<MTLRenderPipelineState> pipeline_state = setupRenderPipelineState(drawParams, vbo_update_info.alignedStride, &same_as_last_ps);
	AXGL_ASSERT(pipeline_state != nil);
	// MTLDepthStencilStateを用意
	bool same_as_last_dss = false;
	id<MTLDepthStencilState> depth_stencil_state = setupDepthStencilState(drawParams, &same_as_last_dss);
	AXGL_ASSERT(depth_stencil_state != nil);
	// 描画コマンドを作成
	{
		// MTLRenderPassDescriptorを準備
		bool render_pass_changed = false;
		MTLRenderPassDescriptor* render_pass_desc = setupRenderPassDescriptor(drawParams, clearParams, &render_pass_changed);
		AXGL_ASSERT(render_pass_desc != nil);
		if (render_pass_changed) {
			// レンダーパスが変わった場合、古いコマンドエンコーダを終了
			endRenderCommandEncoder();
		}
		// コマンドエンコーダを用意
		setupRenderCommandEncoder(render_pass_desc);
		id<MTLRenderCommandEncoder> command_encoder = m_renderCommandEncoder;
		AXGL_ASSERT(command_encoder != nil);
		// 描画パラメータの設定が必要か
		bool need_draw_parameter = !m_setDrawParameterToEncoder;
		// 描画に使用するプログラム
		ProgramMetal* program_metal = static_cast<ProgramMetal*>(drawParams->renderPipelineState.program);
		if (need_draw_parameter || !same_as_last_ps) {
			// RenderPipelineStateを設定
			[command_encoder setRenderPipelineState:pipeline_state];
		}
		if (need_draw_parameter || !same_as_last_dss) {
			// DepthStencilStateを設定
			[command_encoder setDepthStencilState:depth_stencil_state];
		}
		if (need_draw_parameter || ((drawParams->dirtyFlags & STENCIL_REFERENCE_DIRTY_BIT) != 0)) {
			// ステンシルリファレンス値を設定
			setStencilReference(command_encoder, &drawParams->stencilReference);
		}
		bool need_yflip = viewportNeedYFlip(drawParams);
		if (need_draw_parameter || ((drawParams->dirtyFlags & VIEWPORT_DIRTY_BIT) != 0)) {
			// ビューポートを設定
			setViewport(command_encoder, &drawParams->viewportParams, need_yflip);
		}
		// GLのVBOとして機能する頂点バッファの設定
		if (drawParams->vertexArray == nullptr) {
			// 現在のGLステートから設定
			setVertexBufferForVBO(command_encoder, drawParams->vertexBuffer, program_metal, drawParams->renderPipelineState.vertexAttribs, &vbo_dynamic_update_info);
		} else {
			// VAOから設定
			VertexArrayMetal* vertex_array_metal = static_cast<VertexArrayMetal*>(drawParams->vertexArray->getBackendVertexArray());
			AXGL_ASSERT(vertex_array_metal != nullptr);
			// エンコーダーが新規に作成された場合、プログラムやVAOに変更があった場合は頂点バッファを全設定する
			bool need_to_set_vbo = need_draw_parameter
				|| ((drawParams->dirtyFlags & (CURRENT_PROGRAM_DIRTY_BIT | VERTEX_ARRAY_BINDING_DIRTY_BIT)) != 0)
				|| program_metal->isProgramDirty() || vertex_array_metal->isVertexBufferDirty();
			setVertexBufferForVBOFromVAO(command_encoder, vertex_array_metal, program_metal, need_to_set_vbo, &vbo_dynamic_update_info);
		}
		// GLのUBOとして機能する頂点バッファとフラグメントバッファを設定
		// プログラムやUniform bufferのバインドに変更があった場合はMTLBufferを全設定する
		bool set_all_ubo = need_draw_parameter
			|| ((drawParams->dirtyFlags & (CURRENT_PROGRAM_DIRTY_BIT | UNIFORM_BUFFER_BINDING_DIRTY_BIT)) != 0);
		setBufferForUBO(command_encoder, program_metal, drawParams->uniformBuffer, set_all_ubo, &ubo_dynamic_update_info);
		// デフォルトUniform blockとして機能する頂点バッファとフラグメントバッファを設定
		setDefaultUniformBuffer(command_encoder, program_metal);
		// テクスチャとサンプラを設定
		setTextureAndSampler(command_encoder, program_metal, drawParams, need_draw_parameter);
		// VisibilityResultModeを設定
		// NOTE: GLのQueryに使用するVisibilityResultBufferは描画毎にオフセットを変更するため、毎回設定する
		setVisibilityResultMode(command_encoder, drawParams, need_draw_parameter);
		if (need_draw_parameter || ((drawParams->dirtyFlags & CULL_FACE_DIRTY_BIT) != 0)) {
			// カリングを設定
			setCullMode(command_encoder, &drawParams->cullFaceParams, need_yflip);
		}
		if (need_draw_parameter || ((drawParams->dirtyFlags & SCISSOR_DIRTY_BIT) != 0)) {
			// Scissorを設定
			RectSize framebuffer_size;
			getFramebufferSize(drawParams, &framebuffer_size);
			setScissor(command_encoder, &drawParams->scissorParams, &framebuffer_size, need_yflip);
		}
		if (need_draw_parameter || ((drawParams->dirtyFlags & POLYGON_OFFSET_DIRTY_BIT) != 0)) {
			// 深度バイアスを設定
			setDepthBias(command_encoder, &drawParams->polygonOffsetParams);
		}
		// drawPrimitivesの呼び出し
		if (mode == GL_TRIANGLE_FAN) {
			if (count > 2) {
				// TriangleFan(TriangleFan変換済みのデータは常にバッファ先頭から格納)
				uint32_t tri_fan_count = (count - 2) * 3;
				[command_encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:tri_fan_count];
			}
		} else {
			// TriangleFan以外
			MTLPrimitiveType primitive_type = convert_primitive_type(mode);
			[command_encoder drawPrimitives:primitive_type vertexStart:first vertexCount:count];
		}
		// 描画パラメータを設定済み
		m_setDrawParameterToEncoder = true;
		// 使用したプログラムのdirtyをクリア
		program_metal->clearDirty();
	}
	return true;
}

// インデックスありの描画を実行(glDrawElements相当)
bool ContextMetal::drawElements(GLenum mode, GLsizei count, GLenum type, const void* indices, const DrawParameters* drawParams, const ClearParameters* clearParams)
{
	AXGL_ASSERT((drawParams != nullptr) && (clearParams != nullptr));
	// インデックスバッファの変換を判別
	intptr_t ibo_offset = (intptr_t)indices;
	intptr_t ibo_size = get_indices_size(count, type);
	BackendBuffer::ConversionMode ibo_conversion = getIboConversionMode(mode, type);
	// 各バッファの更新をチェック
	bool is_ubyte = (type == GL_UNSIGNED_BYTE);
	VboUpdateInfo vbo_update_info;
	UboUpdateInfo ubo_update_info;
	IboUpdateInfo ibo_update_info;
	VboDynamicUpdateInfo vbo_dynamic_update_info;
	UboDynamicUpdateInfo ubo_dynamic_update_info;
	IboDynamicUpdateInfo ibo_dynamic_update_info;
	bool vbo_update = checkVBOUpdate(&vbo_update_info, &vbo_dynamic_update_info, drawParams, BackendBuffer::ConversionModeNone, 0, 0);
	bool ubo_update = checkUBOUpdate(&ubo_update_info, &ubo_dynamic_update_info, drawParams);
	bool ibo_update = checkIBOUpdate(&ibo_update_info, &ibo_dynamic_update_info, drawParams, ibo_conversion, ibo_offset, ibo_size, is_ubyte);
	// 描画コマンドバッファを用意
	setupDrawCommandBuffer();
	AXGL_ASSERT(m_drawCommandBuffer != nil);
	// バッファ更新がBlitコマンドを必要とするか
	bool use_blit_command = (vbo_update && vbo_update_info.useBlit) || ubo_update
		|| (ibo_update && ibo_update_info.useBlit);
	if (use_blit_command) {
		// Blit command encoder を作成、Render command encoder が使用されている場合は終了される
		setupBlitCommandEncoder();
	}
	// 各バッファの更新処理
	if (vbo_update) {
		updateVBO(&vbo_update_info);
	}
	if (ubo_update) {
		updateUBO(&ubo_update_info);
	}
	if (ibo_update) {
		updateIBO(&ibo_update_info);
	}
	if (use_blit_command) {
		// 描画を実行するためBlit command encoderを終了
		endBlitCommandEncoder();
	}
	// 動的バッファ用のMTLBufferを更新
	updateDynamicBuffers(&vbo_dynamic_update_info, &ubo_dynamic_update_info, &ibo_dynamic_update_info);
	// MTLRenderPipelineStateを用意
	bool same_as_last_ps = false;
	id<MTLRenderPipelineState> pipeline_state = setupRenderPipelineState(drawParams, vbo_update_info.alignedStride, &same_as_last_ps);
	AXGL_ASSERT(pipeline_state != nil);
	// MTLDepthStencilStateを用意
	bool same_as_last_dss = false;
	id<MTLDepthStencilState> depth_stencil_state = setupDepthStencilState(drawParams, &same_as_last_dss);
	AXGL_ASSERT(depth_stencil_state != nil);
	// 描画コマンドを作成
	{
		// MTLRenderPassDescriptorを準備
		bool render_pass_changed = false;
		MTLRenderPassDescriptor* render_pass_desc = setupRenderPassDescriptor(drawParams, clearParams, &render_pass_changed);
		AXGL_ASSERT(render_pass_desc != nil);
		if (render_pass_changed) {
			// レンダーパスが変わった場合、古いコマンドエンコーダを終了
			endRenderCommandEncoder();
		}
		// コマンドエンコーダを用意
		setupRenderCommandEncoder(render_pass_desc);
		id<MTLRenderCommandEncoder> command_encoder = m_renderCommandEncoder;
		AXGL_ASSERT(command_encoder != nil);
		// 描画パラメータの設定が必要か
		bool need_draw_parameter = !m_setDrawParameterToEncoder;
		// 描画に使用するプログラム
		ProgramMetal* program_metal = static_cast<ProgramMetal*>(drawParams->renderPipelineState.program);
		if (need_draw_parameter || !same_as_last_ps) {
			// RenderPipelineStateを設定
			[command_encoder setRenderPipelineState:pipeline_state];
		}
		if (need_draw_parameter || !same_as_last_dss) {
			// DepthStencilStateを設定
			[command_encoder setDepthStencilState:depth_stencil_state];
		}
		if (need_draw_parameter || ((drawParams->dirtyFlags & STENCIL_REFERENCE_DIRTY_BIT) != 0)) {
			// ステンシルリファレンス値を設定
			setStencilReference(command_encoder, &drawParams->stencilReference);
		}
		bool need_yflip = viewportNeedYFlip(drawParams);
		if (need_draw_parameter || ((drawParams->dirtyFlags & VIEWPORT_DIRTY_BIT) != 0)) {
			// ビューポートを設定
			setViewport(command_encoder, &drawParams->viewportParams, need_yflip);
		}
		// GLのVBOとして機能する頂点バッファの設定
		id<MTLBuffer> index_buffer = nil;
		size_t index_buffer_base_offset = 0;
		if (drawParams->vertexArray == nullptr) {
			// 現在のGLステートから設定
			setVertexBufferForVBO(command_encoder, drawParams->vertexBuffer, program_metal, drawParams->renderPipelineState.vertexAttribs, &vbo_dynamic_update_info);
			// 現在のインデックスバッファを取得
			if (drawParams->indexBuffer != nullptr) {
				BufferMetal* buffer_metal = static_cast<BufferMetal*>(drawParams->indexBuffer->getBackendBuffer());
				if (buffer_metal != nullptr) {
					index_buffer = buffer_metal->getMtlBuffer();
				}
			}
		} else {
			// VAOから設定
			VertexArrayMetal* vertex_array_metal = static_cast<VertexArrayMetal*>(drawParams->vertexArray->getBackendVertexArray());
			AXGL_ASSERT(vertex_array_metal != nullptr);
			// エンコーダーが新規に作成された場合、プログラムやVAOに変更があった場合は頂点バッファを全設定する
			bool need_to_set_vbo = need_draw_parameter
				|| ((drawParams->dirtyFlags & (CURRENT_PROGRAM_DIRTY_BIT | VERTEX_ARRAY_BINDING_DIRTY_BIT)) != 0)
				|| program_metal->isProgramDirty() || vertex_array_metal->isVertexBufferDirty();
			setVertexBufferForVBOFromVAO(command_encoder, vertex_array_metal, program_metal, need_to_set_vbo, &vbo_dynamic_update_info);
			// VAOからインデックスバッファを取得
			BufferMetal* buffer_metal = vertex_array_metal->getIndexBuffer();
			if (buffer_metal != nullptr) {
				index_buffer = buffer_metal->getMtlBuffer();
			}
		}
		// 動的インデックスバッファの場合、動的バッファを使用
		if (ibo_dynamic_update_info.dynamicBuffer != nil) {
			index_buffer = ibo_dynamic_update_info.dynamicBuffer;
			index_buffer_base_offset = ibo_dynamic_update_info.offset;
		}
		// GLのUBOとして機能する頂点バッファとフラグメントバッファを設定
		// プログラムやUniform bufferのバインドに変更があった場合はMTLBufferを全設定する
		bool set_all_ubo = need_draw_parameter
			|| ((drawParams->dirtyFlags & (CURRENT_PROGRAM_DIRTY_BIT | UNIFORM_BUFFER_BINDING_DIRTY_BIT)) != 0);
		setBufferForUBO(command_encoder, program_metal, drawParams->uniformBuffer, set_all_ubo, &ubo_dynamic_update_info);
		// デフォルトUniform blockとして機能する頂点バッファとフラグメントバッファを設定
		setDefaultUniformBuffer(command_encoder, program_metal);
		// テクスチャとサンプラを設定
		setTextureAndSampler(command_encoder, program_metal, drawParams, need_draw_parameter);
		// VisibilityResultModeを設定
		// NOTE: GLのQueryに使用するVisibilityResultBufferは描画毎にオフセットを変更するため、毎回設定する
		setVisibilityResultMode(command_encoder, drawParams, need_draw_parameter);
		if (need_draw_parameter || ((drawParams->dirtyFlags & CULL_FACE_DIRTY_BIT) != 0)) {
			// カリングを設定
			setCullMode(command_encoder, &drawParams->cullFaceParams, need_yflip);
		}
		if (need_draw_parameter || ((drawParams->dirtyFlags & SCISSOR_DIRTY_BIT) != 0)) {
			// Scissorを設定
			RectSize framebuffer_size;
			getFramebufferSize(drawParams, &framebuffer_size);
			setScissor(command_encoder, &drawParams->scissorParams, &framebuffer_size, need_yflip);
		}
		if (need_draw_parameter || ((drawParams->dirtyFlags & POLYGON_OFFSET_DIRTY_BIT) != 0)) {
			// 深度バイアスを設定
			setDepthBias(command_encoder, &drawParams->polygonOffsetParams);
		}
		// drawIndexedPrimitivesの呼び出し
		if (index_buffer != nil) {
			MTLIndexType index_type = convert_index_type(type);
			if (mode == GL_TRIANGLE_FAN) {
				if (count > 2) {
					// TriangleFan(TriangleFan変換済みのデータは常にバッファ先頭から格納)
					uint32_t tri_fan_count = (count - 2) * 3;
					[command_encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:tri_fan_count indexType:index_type indexBuffer:index_buffer indexBufferOffset:index_buffer_base_offset];
				}
			} else {
				// TriangleFan以外
				MTLPrimitiveType mtl_type = convert_primitive_type(mode);
				uint32_t index_buffer_offset = (uint32_t)((intptr_t)indices); // GL仕様からのキャスト
				if (is_ubyte) {
					// uint8インデックスの場合、uint16に変換しているためオフセットを調整
					index_buffer_offset *= 2;
				}
				[command_encoder drawIndexedPrimitives:mtl_type indexCount:count indexType:index_type indexBuffer:index_buffer indexBufferOffset:(index_buffer_base_offset + index_buffer_offset)];
			}
		} else {
			AXGL_ASSERT(0);
		}
		// 描画パラメータを設定済み
		m_setDrawParameterToEncoder = true;
		// 使用したプログラムのdirtyをクリア
		program_metal->clearDirty();
	}
	return true;
}

// インデックスなしインスタンス描画を実行(glDrawArraysInstanced相当)
bool ContextMetal::drawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount,
	const DrawParameters* drawParams, const ClearParameters* clearParams)
{
	AXGL_ASSERT((drawParams != nullptr) && (clearParams != nullptr));
	// TriangleFanの場合は変換処理が必要
	BackendBuffer::ConversionMode vbo_conversion = BackendBuffer::ConversionModeNone;
	if (mode == GL_TRIANGLE_FAN) {
		vbo_conversion = BackendBuffer::ConversionModeTriFanVertices;
	}
	// 各バッファの更新をチェック
	VboUpdateInfo vbo_update_info;
	UboUpdateInfo ubo_update_info;
	VboDynamicUpdateInfo vbo_dynamic_update_info;
	UboDynamicUpdateInfo ubo_dynamic_update_info;
	bool vbo_update = checkVBOUpdate(&vbo_update_info, &vbo_dynamic_update_info, drawParams, vbo_conversion, first, count);
	bool ubo_update = checkUBOUpdate(&ubo_update_info, &ubo_dynamic_update_info, drawParams);
	// 描画コマンドバッファを用意
	setupDrawCommandBuffer();
	AXGL_ASSERT(m_drawCommandBuffer != nil);
	// バッファ更新がBlitコマンドを必要とするか
	bool use_blit_command = (vbo_update && vbo_update_info.useBlit) || ubo_update;
	if (use_blit_command) {
		// Blit command encoder を作成、Render command encoder が使用されている場合は終了される
		setupBlitCommandEncoder();
	}
	// 各バッファの更新処理
	if (vbo_update) {
		updateVBO(&vbo_update_info);
	}
	if (ubo_update) {
		updateUBO(&ubo_update_info);
	}
	if (use_blit_command) {
		// 描画を実行するためBlit command encoderを終了
		endBlitCommandEncoder();
	}
	// 動的バッファ用のMTLBufferを更新
	updateDynamicBuffers(&vbo_dynamic_update_info, &ubo_dynamic_update_info, nullptr);
	// MTLRenderPipelineStateを用意
	bool same_as_last_ps = false;
	id<MTLRenderPipelineState> pipeline_state = setupRenderPipelineState(drawParams, vbo_update_info.alignedStride, &same_as_last_ps);
	AXGL_ASSERT(pipeline_state != nil);
	// MTLDepthStencilStateを用意
	bool same_as_last_dss = false;
	id<MTLDepthStencilState> depth_stencil_state = setupDepthStencilState(drawParams, &same_as_last_dss);
	AXGL_ASSERT(depth_stencil_state != nil);
	// 描画コマンドを作成
	{
		// MTLRenderPassDescriptorを準備
		bool render_pass_changed = false;
		MTLRenderPassDescriptor* render_pass_desc = setupRenderPassDescriptor(drawParams, clearParams, &render_pass_changed);
		AXGL_ASSERT(render_pass_desc != nil);
		if (render_pass_changed) {
			// レンダーパスが変わった場合、古いコマンドエンコーダを終了
			endRenderCommandEncoder();
		}
		// コマンドエンコーダを用意
		setupRenderCommandEncoder(render_pass_desc);
		id<MTLRenderCommandEncoder> command_encoder = m_renderCommandEncoder;
		AXGL_ASSERT(command_encoder != nil);
		// 描画パラメータの設定が必要か
		bool need_draw_parameter = !m_setDrawParameterToEncoder;
		// 描画に使用するプログラム
		ProgramMetal* program_metal = static_cast<ProgramMetal*>(drawParams->renderPipelineState.program);
		if (need_draw_parameter || !same_as_last_ps) {
			// RenderPipelineStateを設定
			[command_encoder setRenderPipelineState:pipeline_state];
		}
		if (need_draw_parameter || !same_as_last_dss) {
			// DepthStencilStateを設定
			[command_encoder setDepthStencilState:depth_stencil_state];
		}
		if (need_draw_parameter || ((drawParams->dirtyFlags & STENCIL_REFERENCE_DIRTY_BIT) != 0)) {
			// ステンシルリファレンス値を設定
			setStencilReference(command_encoder, &drawParams->stencilReference);
		}
		bool need_yflip = viewportNeedYFlip(drawParams);
		if (need_draw_parameter || ((drawParams->dirtyFlags & VIEWPORT_DIRTY_BIT) != 0)) {
			// ビューポートを設定
			setViewport(command_encoder, &drawParams->viewportParams, need_yflip);
		}
		// GLのVBOとして機能する頂点バッファの設定
		if (drawParams->vertexArray == nullptr) {
			// 現在のGLステートから設定
			setVertexBufferForVBO(command_encoder, drawParams->vertexBuffer, program_metal, drawParams->renderPipelineState.vertexAttribs, &vbo_dynamic_update_info);
		} else {
			// VAOから設定
			VertexArrayMetal* vertex_array_metal = static_cast<VertexArrayMetal*>(drawParams->vertexArray->getBackendVertexArray());
			AXGL_ASSERT(vertex_array_metal != nullptr);
			// エンコーダーが新規に作成された場合、プログラムやVAOに変更があった場合は頂点バッファを全設定する
			bool need_to_set_vbo = need_draw_parameter
				|| ((drawParams->dirtyFlags & (CURRENT_PROGRAM_DIRTY_BIT | VERTEX_ARRAY_BINDING_DIRTY_BIT)) != 0)
				|| program_metal->isProgramDirty() || vertex_array_metal->isVertexBufferDirty();
			setVertexBufferForVBOFromVAO(command_encoder, vertex_array_metal, program_metal, need_to_set_vbo, &vbo_dynamic_update_info);
		}
		// GLのUBOとして機能する頂点バッファとフラグメントバッファを設定
		// プログラムやUniform bufferのバインドに変更があった場合はMTLBufferを全設定する
		bool set_all_ubo = need_draw_parameter
			|| ((drawParams->dirtyFlags & (CURRENT_PROGRAM_DIRTY_BIT | UNIFORM_BUFFER_BINDING_DIRTY_BIT)) != 0);
		setBufferForUBO(command_encoder, program_metal, drawParams->uniformBuffer, set_all_ubo, &ubo_dynamic_update_info);
		// デフォルトUniform blockとして機能する頂点バッファとフラグメントバッファを設定
		setDefaultUniformBuffer(command_encoder, program_metal);
		// テクスチャとサンプラを設定
		setTextureAndSampler(command_encoder, program_metal, drawParams, need_draw_parameter);
		// VisibilityResultModeを設定
		// NOTE: GLのQueryに使用するVisibilityResultBufferは描画毎にオフセットを変更するため、毎回設定する
		setVisibilityResultMode(command_encoder, drawParams, need_draw_parameter);
		if (need_draw_parameter || ((drawParams->dirtyFlags & CULL_FACE_DIRTY_BIT) != 0)) {
			// カリングを設定
			setCullMode(command_encoder, &drawParams->cullFaceParams, need_yflip);
		}
		if (need_draw_parameter || ((drawParams->dirtyFlags & SCISSOR_DIRTY_BIT) != 0)) {
			// Scissorを設定
			RectSize framebuffer_size;
			getFramebufferSize(drawParams, &framebuffer_size);
			setScissor(command_encoder, &drawParams->scissorParams, &framebuffer_size, need_yflip);
		}
		if (need_draw_parameter || ((drawParams->dirtyFlags & POLYGON_OFFSET_DIRTY_BIT) != 0)) {
			// 深度バイアスを設定
			setDepthBias(command_encoder, &drawParams->polygonOffsetParams);
		}
		// drawPrimitivesの呼び出し
		if (mode == GL_TRIANGLE_FAN) {
			if (count > 2) {
				// TriangleFan(TriangleFan変換済みのデータは常にバッファ先頭から格納)
				uint32_t tri_fan_count = (count - 2) * 3;
				[command_encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:tri_fan_count instanceCount:instancecount];
			}
		} else {
			// TriangleFan以外
			MTLPrimitiveType primitive_type = convert_primitive_type(mode);
			[command_encoder drawPrimitives:primitive_type vertexStart:first vertexCount:count instanceCount:instancecount];
		}
		// 描画パラメータを設定済み
		m_setDrawParameterToEncoder = true;
		// 使用したプログラムのdirtyをクリア
		program_metal->clearDirty();
	}
	return true;
}

// インデックスありインスタンス描画を実行(glDrawArraysInstanced相当)
bool ContextMetal::drawElementsInstanced(GLenum mode, GLsizei count, GLsizei type, const void* indices, GLsizei instancecount,
	const DrawParameters* drawParams, const ClearParameters* clearParams)
{
	AXGL_ASSERT((drawParams != nullptr) && (clearParams != nullptr));
	// インデックスバッファの変換を判別
	intptr_t ibo_offset = (intptr_t)indices;
	intptr_t ibo_size = get_indices_size(count, type);
	BackendBuffer::ConversionMode ibo_conversion = getIboConversionMode(mode, type);
	// 各バッファの更新をチェック
	bool is_ubyte = (type == GL_UNSIGNED_BYTE);
	VboUpdateInfo vbo_update_info;
	UboUpdateInfo ubo_update_info;
	IboUpdateInfo ibo_update_info;
	VboDynamicUpdateInfo vbo_dynamic_update_info;
	UboDynamicUpdateInfo ubo_dynamic_update_info;
	IboDynamicUpdateInfo ibo_dynamic_update_info;
	bool vbo_update = checkVBOUpdate(&vbo_update_info, &vbo_dynamic_update_info, drawParams, BackendBuffer::ConversionModeNone, 0, 0);
	bool ubo_update = checkUBOUpdate(&ubo_update_info, &ubo_dynamic_update_info, drawParams);
	bool ibo_update = checkIBOUpdate(&ibo_update_info, &ibo_dynamic_update_info, drawParams, ibo_conversion, ibo_offset, ibo_size, is_ubyte);
	// 描画コマンドバッファを用意
	setupDrawCommandBuffer();
	AXGL_ASSERT(m_drawCommandBuffer != nil);
	// バッファ更新がBlitコマンドを必要とするか
	bool use_blit_command = (vbo_update && vbo_update_info.useBlit) || ubo_update
		|| (ibo_update && ibo_update_info.useBlit);
	if (use_blit_command) {
		// Blit command encoder を作成、Render command encoder が使用されている場合は終了される
		setupBlitCommandEncoder();
	}
	// 各バッファの更新処理
	if (vbo_update) {
		updateVBO(&vbo_update_info);
	}
	if (ubo_update) {
		updateUBO(&ubo_update_info);
	}
	if (ibo_update) {
		updateIBO(&ibo_update_info);
	}
	if (use_blit_command) {
		// 描画を実行するためBlit command encoderを終了
		endBlitCommandEncoder();
	}
	// 動的バッファ用のMTLBufferを更新
	updateDynamicBuffers(&vbo_dynamic_update_info, &ubo_dynamic_update_info, &ibo_dynamic_update_info);
	// MTLRenderPipelineStateを用意
	bool same_as_last_ps = false;
	id<MTLRenderPipelineState> pipeline_state = setupRenderPipelineState(drawParams, vbo_update_info.alignedStride, &same_as_last_ps);
	AXGL_ASSERT(pipeline_state != nil);
	// MTLDepthStencilStateを用意
	bool same_as_last_dss = false;
	id<MTLDepthStencilState> depth_stencil_state = setupDepthStencilState(drawParams, &same_as_last_dss);
	AXGL_ASSERT(depth_stencil_state != nil);
	// 描画コマンドを作成
	{
		// MTLRenderPassDescriptorを準備
		bool render_pass_changed = false;
		MTLRenderPassDescriptor* render_pass_desc = setupRenderPassDescriptor(drawParams, clearParams, &render_pass_changed);
		AXGL_ASSERT(render_pass_desc != nil);
		if (render_pass_changed) {
			// レンダーパスが変わった場合、古いコマンドエンコーダを終了
			endRenderCommandEncoder();
		}
		// コマンドエンコーダを用意
		setupRenderCommandEncoder(render_pass_desc);
		id<MTLRenderCommandEncoder> command_encoder = m_renderCommandEncoder;
		AXGL_ASSERT(command_encoder != nil);
		// 描画パラメータの設定が必要か
		bool need_draw_parameter = !m_setDrawParameterToEncoder;
		// 描画に使用するプログラム
		ProgramMetal* program_metal = static_cast<ProgramMetal*>(drawParams->renderPipelineState.program);
		if (need_draw_parameter || !same_as_last_ps) {
			// RenderPipelineStateを設定
			[command_encoder setRenderPipelineState:pipeline_state];
		}
		if (need_draw_parameter || !same_as_last_dss) {
			// DepthStencilStateを設定
			[command_encoder setDepthStencilState:depth_stencil_state];
		}
		if (need_draw_parameter || ((drawParams->dirtyFlags & STENCIL_REFERENCE_DIRTY_BIT) != 0)) {
			// ステンシルリファレンス値を設定
			setStencilReference(command_encoder, &drawParams->stencilReference);
		}
		bool need_yflip = viewportNeedYFlip(drawParams);
		if (need_draw_parameter || ((drawParams->dirtyFlags & VIEWPORT_DIRTY_BIT) != 0)) {
			// ビューポートを設定
			setViewport(command_encoder, &drawParams->viewportParams, need_yflip);
		}
		// GLのVBOとして機能する頂点バッファの設定
		id<MTLBuffer> index_buffer = nil;
		size_t index_buffer_base_offset = 0;
		if (drawParams->vertexArray == nullptr) {
			// 現在のGLステートから設定
			setVertexBufferForVBO(command_encoder, drawParams->vertexBuffer, program_metal, drawParams->renderPipelineState.vertexAttribs, &vbo_dynamic_update_info);
			// 現在のインデックスバッファを取得
			if (drawParams->indexBuffer != nullptr) {
				BufferMetal* buffer_metal = static_cast<BufferMetal*>(drawParams->indexBuffer->getBackendBuffer());
				if (buffer_metal != nullptr) {
					index_buffer = buffer_metal->getMtlBuffer();
				}
			}
		} else {
			// VAOから設定
			VertexArrayMetal* vertex_array_metal = static_cast<VertexArrayMetal*>(drawParams->vertexArray->getBackendVertexArray());
			AXGL_ASSERT(vertex_array_metal != nullptr);
			// エンコーダーが新規に作成された場合、プログラムやVAOに変更があった場合は頂点バッファを全設定する
			bool need_to_set_vbo = need_draw_parameter
				|| ((drawParams->dirtyFlags & (CURRENT_PROGRAM_DIRTY_BIT | VERTEX_ARRAY_BINDING_DIRTY_BIT)) != 0)
				|| program_metal->isProgramDirty() || vertex_array_metal->isVertexBufferDirty();
			setVertexBufferForVBOFromVAO(command_encoder, vertex_array_metal, program_metal, need_to_set_vbo, &vbo_dynamic_update_info);
			// VAOからインデックスバッファを取得
			BufferMetal* buffer_metal = vertex_array_metal->getIndexBuffer();
			if (buffer_metal != nullptr) {
				index_buffer = buffer_metal->getMtlBuffer();
			}
		}
		// 動的インデックスバッファの場合、動的バッファを使用
		if (ibo_dynamic_update_info.dynamicBuffer != nil) {
			index_buffer = ibo_dynamic_update_info.dynamicBuffer;
			index_buffer_base_offset = ibo_dynamic_update_info.offset;
		}
		// GLのUBOとして機能する頂点バッファとフラグメントバッファを設定
		// プログラムやUniform bufferのバインドに変更があった場合はMTLBufferを全設定する
		bool set_all_ubo = need_draw_parameter
			|| ((drawParams->dirtyFlags & (CURRENT_PROGRAM_DIRTY_BIT | UNIFORM_BUFFER_BINDING_DIRTY_BIT)) != 0);
		setBufferForUBO(command_encoder, program_metal, drawParams->uniformBuffer, set_all_ubo, &ubo_dynamic_update_info);
		// デフォルトUniform blockとして機能する頂点バッファとフラグメントバッファを設定
		setDefaultUniformBuffer(command_encoder, program_metal);
		// テクスチャとサンプラを設定
		setTextureAndSampler(command_encoder, program_metal, drawParams, need_draw_parameter);
		// VisibilityResultModeを設定
		// NOTE: GLのQueryに使用するVisibilityResultBufferは描画毎にオフセットを変更するため、毎回設定する
		setVisibilityResultMode(command_encoder, drawParams, need_draw_parameter);
		if (need_draw_parameter || ((drawParams->dirtyFlags & CULL_FACE_DIRTY_BIT) != 0)) {
			// カリングを設定
			setCullMode(command_encoder, &drawParams->cullFaceParams, need_yflip);
		}
		if (need_draw_parameter || ((drawParams->dirtyFlags & SCISSOR_DIRTY_BIT) != 0)) {
			// Scissorを設定
			RectSize framebuffer_size;
			getFramebufferSize(drawParams, &framebuffer_size);
			setScissor(command_encoder, &drawParams->scissorParams, &framebuffer_size, need_yflip);
		}
		if (need_draw_parameter || ((drawParams->dirtyFlags & POLYGON_OFFSET_DIRTY_BIT) != 0)) {
			// 深度バイアスを設定
			setDepthBias(command_encoder, &drawParams->polygonOffsetParams);
		}
		// drawIndexedPrimitivesの呼び出し
		if (index_buffer != nil) {
			MTLIndexType index_type = convert_index_type(type);
			if (mode == GL_TRIANGLE_FAN) {
				if (count > 2) {
					// TriangleFan(TriangleFan変換済みのデータは常にバッファ先頭から格納)
					uint32_t tri_fan_count = (count - 2) * 3;
					[command_encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:tri_fan_count indexType:index_type indexBuffer:index_buffer indexBufferOffset:index_buffer_base_offset instanceCount:instancecount];
				}
			} else {
				MTLPrimitiveType mtl_type = convert_primitive_type(mode);
				uint32_t index_buffer_offset = (uint32_t)((intptr_t)indices); // GL仕様からのキャスト
				if (is_ubyte) {
					// uint8インデックスの場合、uint16に変換しているためオフセットを調整
					index_buffer_offset *= 2;
				}
				[command_encoder drawIndexedPrimitives:mtl_type indexCount:count indexType:index_type indexBuffer:index_buffer indexBufferOffset:(index_buffer_base_offset + index_buffer_offset) instanceCount:instancecount];
			}
		} else {
			AXGL_ASSERT(0);
		}
		// 描画パラメータを設定済み
		m_setDrawParameterToEncoder = true;
		// 使用したプログラムのdirtyをクリア
		program_metal->clearDirty();
	}
	return true;
}

// 描画コマンドを実行(glFlush相当)
bool ContextMetal::flush()
{
	// 描画コマンドバッファが存在する場合は commit する
	commitDrawCommandBuffer(WaitModeNone);
	return true;
}

// 描画コマンドを実行して完了を待つ(glFinish相当)
bool ContextMetal::finish()
{
	// 描画コマンドバッファが存在する場合は commit -> scheduled待ち
	commitDrawCommandBuffer(WaitModeScheduled);
	// 新たにコマンドバッファを作成して完了待ち（描画以外のコマンドを考慮）
	AXGL_ASSERT(m_commandQueue != nil);
	id<MTLCommandBuffer> command_buffer = [m_commandQueue commandBuffer];
	AXGL_ASSERT(command_buffer != nil);
	[command_buffer commit];
	[command_buffer waitUntilCompleted];
	return true;
}

// カラーバッファのピクセルを読み出す(glReadPixels相当)
// NOTE: テストやデバッグ用の実装であり、通常のアプリケーションでglReadPixelsを使用すべきでない
bool ContextMetal::readPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type,
	BackendFramebuffer* readFramebuffer, GLenum readBuffer, void* pixels)
{
	AXGL_ASSERT(pixels != nullptr);
	// 描画コマンドバッファが存在する場合は commit して完了を待つ
	commitDrawCommandBuffer(WaitModeCompleted);
	// ソーステクスチャを取得
	FramebufferMetal* framebuffer_metal = static_cast<FramebufferMetal*>(readFramebuffer);
	id<MTLTexture> src_texture = nil;
	uint32_t src_level = 0;
	if (readBuffer == GL_BACK) {
		// NOTE: デフォルトフレームバッファのリードをiOS GLESはサポートしない
		src_texture = nil;
	} else if((readBuffer >= GL_COLOR_ATTACHMENT0) && (readBuffer < (GL_COLOR_ATTACHMENT0 + AXGL_MAX_COLOR_ATTACHMENTS))) {
		if (readFramebuffer != nullptr) {
			int attach_index = readBuffer - GL_COLOR_ATTACHMENT0;
			src_texture = framebuffer_metal->getColorTexture(attach_index);
			src_level = framebuffer_metal->getColorTextureLevel(attach_index);
		}
	}
	if (src_texture == nil) {
		return true; // リードするターゲットが存在しない
	}
	if (src_texture.framebufferOnly == YES) {
		AXGL_DBGOUT("readPixels> framebufferOnly:YES, MTLTexture can not be blit source\n");
		return false;
	}
	if ((src_texture.pixelFormat != MTLPixelFormatRGBA8Unorm) && (src_texture.pixelFormat != MTLPixelFormatBGRA8Unorm)) {
		AXGL_DBGOUT("readPixels> unsupported MTLPixelFormat\n");
		return false;
	}
	if ((format != GL_RGBA) || (type != GL_UNSIGNED_BYTE)) {
		AXGL_DBGOUT("readPixels> unsuppoted format:0x%04X type:0x%04X\n", format, type);
		return false;
	}
	// Blit転送先のテクスチャを作成
	id<MTLTexture> dst_texture = nil;
	MTLPixelFormat texture_format = src_texture.pixelFormat;
	{
		// NOTE: 2D, Level0 only
		MTLTextureDescriptor* desc = [[MTLTextureDescriptor alloc] init];
		AXGL_ASSERT(desc != nil);
		[desc setTextureType:MTLTextureType2D];
		[desc setPixelFormat:texture_format];
		[desc setWidth:width];
		[desc setHeight:height];
		[desc setUsage:MTLTextureUsageRenderTarget];
		[desc setStorageMode:MTLStorageModeShared];
		[desc setSampleCount:1];
		[desc setMipmapLevelCount:1];
		dst_texture = [m_mtlDevice newTextureWithDescriptor:desc];
		AXGL_ASSERT(dst_texture != nil);
		desc = nil;
	}
	// Blitコマンドを作成
	id<MTLCommandBuffer> command_buffer = [m_commandQueue commandBuffer];
	AXGL_ASSERT(command_buffer != nil);
	id<MTLBlitCommandEncoder> blit_encoder = [command_buffer blitCommandEncoder];
	{
		uint32_t src_img_height = calc_mipmap_size((uint32_t)[src_texture height], src_level);
		MTLSize src_size = {
			(NSUInteger)width, (NSUInteger)height, 1 // width,height,depth
		};
		MTLOrigin src_origin = {
			(NSUInteger)x, (NSUInteger)(src_img_height - y - height), 0, // x,y,z
		};
		MTLOrigin dst_origin = {
			0,0,0 // x,y,z
		};
		[blit_encoder copyFromTexture:src_texture sourceSlice:0 sourceLevel:src_level sourceOrigin:src_origin sourceSize:src_size  toTexture:dst_texture destinationSlice:0 destinationLevel:0 destinationOrigin:dst_origin];
	}
	[blit_encoder endEncoding];
	// コマンドを実行して完了待ち
	[command_buffer commit];
	[command_buffer waitUntilCompleted];
	// Blit転送先からデータを取得
	MTLRegion dst_region = {
		{0,0,0}, // origin
		{(NSUInteger)width,(NSUInteger)height,1} // size
	};
	NSUInteger bpp = get_bytes_per_pixel(format, type);
	NSUInteger stride = width * bpp;
	[dst_texture getBytes:pixels bytesPerRow:stride fromRegion:dst_region mipmapLevel:0];
	// Blit転送先をリリース
	dst_texture = nil;
	// デバイススクリーン座標がGLとMetalで上下反対のため並べ替える
	if (height > 1) {
		void* work_buf = AXGL_ALLOC(stride);
		if (work_buf != nullptr) {
			uint8_t* pixels_buf = static_cast<uint8_t*>(pixels);
			for (int i = 0; i < (height / 2); i++) {
				uint8_t* p0 = pixels_buf + (i * stride);
				uint8_t* p1 = pixels_buf + ((height - 1 - i) * stride);
				memcpy(work_buf, p0, stride);
				memcpy(p0, p1, stride);
				memcpy(p1, work_buf, stride);
			}
			AXGL_FREE(work_buf);
		} else {
			AXGL_DBGOUT("BackendContextMetal::readPixels> work memory allocation failed\n");
		}
	}
	// BGRAはRGBAに並び替える
	if (texture_format == MTLPixelFormatBGRA8Unorm) {
		for (int i = 0; i < (width * height); i++) {
			uint8_t* p = static_cast<uint8_t*>(pixels) + (i * 4);
			uint8_t tmp = p[0];
			p[0] = p[2];
			p[2] = tmp;
		}
	}
	return true;
}

// 全てのキャッシュを無効化(invalidate)する
void ContextMetal::invalidateCache(GLbitfield flags)
{
	if ((flags & GL_CACHE_RENDER_PIPELINE_STATE_BIT_AXGL) != 0) {
		m_pipelineStateUsedOrder.clear();
		m_pipelineStateCache.clear();
	}
	if ((flags & GL_CACHE_DEPTH_STENCIL_STATE_BIT_AXGL) != 0) {
		m_depthStencilStateUsedOrder.clear();
		m_depthStencilStateCache.clear();
	}
	return;
}

// ProgramObjectに関連するキャッシュを破棄する
void ContextMetal::discardCachesAssociatedWithProgram(BackendProgram* program)
{
	AXGL_ASSERT(m_pipelineStateCache.size() == m_pipelineStateUsedOrder.size());

	if (m_pipelineStateUsedOrder.size() != 0) {
		for (auto it = m_pipelineStateUsedOrder.begin(); it != m_pipelineStateUsedOrder.end(); ) {
			if ((*it)->program == program) {
				auto cache_it = m_pipelineStateCache.find(*(*it));
				AXGL_ASSERT(cache_it != m_pipelineStateCache.end());
				cache_it->second = nil;
				m_pipelineStateCache.erase(cache_it);
				it = m_pipelineStateUsedOrder.erase(it);
			} else {
				it++;
			}
		}
	}
	return;
}

// VertexArrayObjectに関連するキャッシュを破棄する
void ContextMetal::discardCachesAssociatedWithVertexArray(BackendVertexArray* vertexArray)
{
	AXGL_ASSERT(m_pipelineStateCache.size() == m_pipelineStateUsedOrder.size());

	if (m_pipelineStateUsedOrder.size() != 0) {
		for (auto it = m_pipelineStateUsedOrder.begin(); it != m_pipelineStateUsedOrder.end(); ) {
			if ((*it)->vertexArray == vertexArray) {
				auto cache_it = m_pipelineStateCache.find(*(*it));
				AXGL_ASSERT(cache_it != m_pipelineStateCache.end());
				cache_it->second = nil;
				m_pipelineStateCache.erase(cache_it);
				it = m_pipelineStateUsedOrder.erase(it);
			} else {
				it++;
			}
		}
	}
	return;
}

// Metal backend interface methods ========
// MTLDeviceを取得
id<MTLDevice> ContextMetal::getDevice() const
{
	return m_mtlDevice;
}

// 描画のMTLCommandQueueを取得
id<MTLCommandQueue> ContextMetal::getCommandQueue() const
{
	return m_commandQueue;
}

// 描画に使用するMTLCommandBufferを取得
id<MTLCommandBuffer> ContextMetal::getDrawCommandBuffer() const
{
	return m_drawCommandBuffer;
}

// BlitのMTLCommandEncoderを取得
id<MTLBlitCommandEncoder> ContextMetal::getBlitCommandEncoder() const
{
	return m_blitCommandEncoder;
}

// MTLCompileOptionsを取得
MTLCompileOptions* ContextMetal::getCompileOptions() const
{
	return m_compileOptions;
}

// レンダーバッファの表示を実行
bool ContextMetal::presentRenderbuffer(RenderbufferMetal* renderbuffer)
{
	if ((m_commandQueue == nil) || (renderbuffer == nullptr)) {
		return false;
	}
	// レンダーバッファから現在のDrawableを取得
	id<CAMetalDrawable> drawable = renderbuffer->getCurrentDrawable();
	// 表示コマンドを作成
	AXGL_ASSERT(drawable != nil);
	setupDrawCommandBuffer();
	AXGL_ASSERT(m_drawCommandBuffer != nil);
	[m_drawCommandBuffer presentDrawable:drawable];
	// コマンドをcommitして実行開始させる
	commitDrawCommandBuffer(WaitModeScheduled);
	// レンダーバッファに次のDrawableを取得させる
	renderbuffer->nextDrawable();
	return true;
}

SpirvMsl* ContextMetal::getBackendSpirvMsl()
{
	return &m_spirvMsl;
}

// private methods --------
// VBOの更新が必要かをチェックする
bool ContextMetal::checkVBOUpdate(VboUpdateInfo* updateInfo, VboDynamicUpdateInfo* dynamicUpdateInfo, const DrawParameters* drawParams,
	BackendBuffer::ConversionMode conversion, GLint first, GLsizei count)
{
	bool update = false;
	AXGL_ASSERT((updateInfo != nullptr) && (drawParams != nullptr));
	// 変換パラメータを保持
	updateInfo->conversion = conversion;
	updateInfo->first = first;
	updateInfo->count = count;
	// Blit マンドを使用しないでクリア
	updateInfo->useBlit = false;
	dynamicUpdateInfo->useDynamicBuffer = false;
	// GL頂点属性のlocationを取得
	const ProgramMetal* program_metal = static_cast<const ProgramMetal*>(drawParams->renderPipelineState.program);
	AXGL_ASSERT(program_metal != nullptr);
	const int32_t* locations = program_metal->getAttribLocations();
	AXGL_ASSERT(locations != nullptr);
	if (drawParams->vertexArray != nullptr) {
		// VAOにバインドされているVBOをチェック
		VertexArrayMetal* vertex_array_metal = static_cast<VertexArrayMetal*>(drawParams->vertexArray->getBackendVertexArray());
		AXGL_ASSERT(vertex_array_metal != nullptr);
		const VertexArrayMetal::AttribParamMetal* attribs = vertex_array_metal->getAttribParams();
		AXGL_ASSERT(attribs != nullptr);
		for (int32_t i = 0; i < AXGL_MAX_VERTEX_ATTRIBS; i++) {
			// デフォルト値でクリア
			updateInfo->buffer[i] = nullptr;
			updateInfo->stride[i] = 16;
			updateInfo->alignedStride[i]  = 16;
			dynamicUpdateInfo->buffer[i] = nullptr;
			dynamicUpdateInfo->offset[i] = 0;
			int32_t loc = locations[i];
			AXGL_ASSERT(loc < AXGL_MAX_VERTEX_ATTRIBS);
			int metalIndex = program_metal->getActiveVertexAttribIndex(i);
			if ((loc >= 0) && (metalIndex >= 0) && attribs[loc].enabled) {
				uint32_t stride = attribs[loc].stride;
				AXGL_ASSERT(stride != 0);
				uint32_t aligned_stride = get_aligned_stride(stride);
				BufferMetal* buffer_metal = attribs[loc].buffer;
				AXGL_ASSERT(buffer_metal != nullptr);
				// NOTE: 頂点バッファ書き換え時は動的バッファを使わない
				if (conversion != BackendBuffer::ConversionModeNone) {
					// フォーマット変換が必要な更新をチェック
					if (buffer_metal->needUpdateWithStrideAndDataConversion(stride, aligned_stride, conversion, first, count)) {
						updateInfo->buffer[i] = buffer_metal;
						update = true;
					}
				} else if (stride != aligned_stride) {
					// ストライド変換が必要な更新をチェック
					if (buffer_metal->needUpdateWithStrideConversion(stride, aligned_stride)) {
						updateInfo->buffer[i] = buffer_metal;
						update = true;
					}
				} else {
					// 変換が不要なケース
					if (buffer_metal->isDynamicBuffer()) {
						// 動的バッファを割り当て
						dynamicUpdateInfo->buffer[i] = buffer_metal;
						dynamicUpdateInfo->useDynamicBuffer = true;
					} else if (buffer_metal->needUpdateWithoutConversion()) {
						// 更新が必要な場合、Blitで更新する
						updateInfo->buffer[i] = buffer_metal;
						update = true;
						updateInfo->useBlit = true;
					}
				}
				// ストライドを格納しておく(変換処理で使用)
				updateInfo->stride[i] = stride;
				updateInfo->alignedStride[i] = aligned_stride;
			}
		}
	} else {
		// 現ステートにバインドされているVBOをチェック
		const VertexAttrib* vertexAttribs = drawParams->renderPipelineState.vertexAttribs;
		AXGL_ASSERT(vertexAttribs != nullptr);
		for (int32_t i = 0; i < AXGL_MAX_VERTEX_ATTRIBS; i++) {
			// デフォルト値でクリア
			updateInfo->buffer[i] = nullptr;
			updateInfo->stride[i] = 16;
			updateInfo->alignedStride[i]  = 16;
			dynamicUpdateInfo->buffer[i] = nullptr;
			dynamicUpdateInfo->dynamicBuffer[i] = nil;
			dynamicUpdateInfo->offset[i] = 0;
			int32_t loc = locations[i];
			AXGL_ASSERT(loc < AXGL_MAX_VERTEX_ATTRIBS);
			const VertexAttrib& va = vertexAttribs[loc];
			int metalIndex = program_metal->getActiveVertexAttribIndex(i);
			if ((loc >= 0) && (metalIndex >= 0) && va.enable) {
				uint32_t stride = va.stride;
				if (stride == 0) {
					stride = get_size_from_gltype(va.type) * va.size;
				}
				uint32_t aligned_stride = get_aligned_stride(stride);
				AXGL_ASSERT(drawParams->vertexBuffer[loc] != nullptr);
				BufferMetal* buffer_metal = static_cast<BufferMetal*>(drawParams->vertexBuffer[loc]->getBackendBuffer());
				AXGL_ASSERT(buffer_metal != nullptr);
				// NOTE: 頂点バッファ書き換え時は動的バッファを使わない
				if (conversion != BackendBuffer::ConversionModeNone) {
					// フォーマット変換が必要な更新をチェック
					if (buffer_metal->needUpdateWithStrideAndDataConversion(stride, aligned_stride, conversion, first, count)) {
						updateInfo->buffer[i] = buffer_metal;
						update = true;
					}
				} else if (stride != aligned_stride) {
					// ストライド変換が必要な更新をチェック
					if (buffer_metal->needUpdateWithStrideConversion(stride, aligned_stride)) {
						updateInfo->buffer[i] = buffer_metal;
						update = true;
					}
				} else {
					// 変換が不要なケース
					if (buffer_metal->isDynamicBuffer()) {
						// 動的バッファを割り当て
						dynamicUpdateInfo->buffer[i] = buffer_metal;
						dynamicUpdateInfo->useDynamicBuffer = true;
					} else if (buffer_metal->needUpdateWithoutConversion()) {
						// 更新が必要な場合、Blitで更新する
						updateInfo->buffer[i] = buffer_metal;
						update = true;
						updateInfo->useBlit = true;
					}
				}
				// ストライドを格納しておく(変換処理で使用)
				updateInfo->stride[i] = stride;
				updateInfo->alignedStride[i] = aligned_stride;
			}
		}
	}
	return update;
}

// UBOの更新が必要かをチェックする
bool ContextMetal::checkUBOUpdate(UboUpdateInfo* updateInfo, UboDynamicUpdateInfo* dynamicUpdateInfo, const DrawParameters* drawParams)
{
	bool update = false;
	AXGL_ASSERT((updateInfo != nullptr) && (drawParams != nullptr));
	dynamicUpdateInfo->useDynamicBuffer = false;
	for (int i = 0; i < AXGL_MAX_UNIFORM_BUFFER_BINDINGS; i++) {
		updateInfo->buffer[i] = nullptr;
		dynamicUpdateInfo->buffer[i] = nullptr;
		dynamicUpdateInfo->dynamicBuffer[i] = nil;
		dynamicUpdateInfo->offset[i] = 0;
		CoreBuffer* core_buffer = drawParams->uniformBuffer[i].buffer;
		if (core_buffer != nullptr) {
			BufferMetal* buffer_metal = static_cast<BufferMetal*>(core_buffer->getBackendBuffer());
			AXGL_ASSERT(buffer_metal != nullptr);
			if (buffer_metal->isDynamicBuffer()) {
				dynamicUpdateInfo->buffer[i] = buffer_metal;
				dynamicUpdateInfo->useDynamicBuffer = true;
			} else if (buffer_metal->needUpdateWithoutConversion()) {
				updateInfo->buffer[i] = buffer_metal;
				update = true;
			}
		}
	}
	return update;
}

// IBOの更新が必要かをチェックする
bool ContextMetal::checkIBOUpdate(IboUpdateInfo* updateInfo, IboDynamicUpdateInfo* dynamicUpdateInfo, const DrawParameters* drawParams,
	BackendBuffer::ConversionMode conversion, intptr_t iboOffset, intptr_t iboSize, bool isUbyte)
{
	AXGL_ASSERT((updateInfo != nullptr) && (drawParams != nullptr));
	BufferMetal* buffer_metal = nullptr;
	if (drawParams->vertexArray == nullptr) {
		// 現在のインデックスバッファ
		if (drawParams->indexBuffer != nullptr) {
			buffer_metal = static_cast<BufferMetal*>(drawParams->indexBuffer->getBackendBuffer());
		}
	} else {
		// VAOからインデックスバッファを取得
		CoreBuffer* index_buffer = drawParams->vertexArray->getIndexBuffer();
		if (index_buffer != nullptr) {
			buffer_metal = static_cast<BufferMetal*>(index_buffer->getBackendBuffer());
		}
	}
	// デフォルト値でクリア
	updateInfo->buffer = nullptr;
	updateInfo->conversion = BackendBuffer::ConversionModeNone;
	updateInfo->iboOffset = iboOffset;
	updateInfo->iboSize = iboSize;
	updateInfo->isUbyte = isUbyte;
	updateInfo->useBlit = false;
	dynamicUpdateInfo->buffer = nullptr;
	dynamicUpdateInfo->dynamicBuffer = nil;
	dynamicUpdateInfo->offset = 0;
	dynamicUpdateInfo->useDynamicBuffer = false;
	bool update = false;
	if (buffer_metal != nullptr) {
		int result = buffer_metal->needUpdateWithIndexConversion(conversion, iboOffset, iboSize, isUbyte);
		if (buffer_metal->isDynamicBuffer() && ((result == BufferMetal::NoUpdateRequired) || (result == BufferMetal::UpdateRequiredWithBlitCommand))) {
			// 動的バッファを割り当て
			dynamicUpdateInfo->buffer = buffer_metal;
			dynamicUpdateInfo->useDynamicBuffer = true;
		} else if (result != BufferMetal::NoUpdateRequired) {
			// バッファを更新
			updateInfo->buffer = buffer_metal;
			updateInfo->conversion = conversion;
			updateInfo->useBlit = (result == BufferMetal::UpdateRequiredWithBlitCommand);
			update = true;
		}
	}

	return update;
}

// VBOの更新を実行する
void ContextMetal::updateVBO(const VboUpdateInfo* updateInfo)
{
	AXGL_ASSERT(updateInfo != nullptr);
	for (int i = 0; i < AXGL_MAX_VERTEX_ATTRIBS; i++) {
		BufferMetal* buffer_metal = updateInfo->buffer[i];
		if (buffer_metal != nullptr) {
			if (updateInfo->conversion != BackendBuffer::ConversionModeNone) {
				// データフォーマット変換を含むセットアップ
				bool result = buffer_metal->setupWithStrideAndDataConversion(this,
					updateInfo->stride[i], updateInfo->alignedStride[i],
					updateInfo->conversion, updateInfo->first, updateInfo->count);
				AXGL_ASSERT(result);
			} else if (updateInfo->stride[i] != updateInfo->alignedStride[i]) {
				// ストライド変換を含むセットアップ
				bool result = buffer_metal->setupBufferWithStrideConversion(this,
					updateInfo->stride[i], updateInfo->alignedStride[i]);
				AXGL_ASSERT(result);
			} else {
				// 変換を必要としないセットアップ
				bool result = buffer_metal->setupBufferInDraw(this);
				AXGL_ASSERT(result);
			}
		}
	}
	return;
}

// UBOの更新を実行する
void ContextMetal::updateUBO(const UboUpdateInfo* updateInfo)
{
	AXGL_ASSERT(updateInfo != nullptr);
	for (int i = 0; i < AXGL_MAX_UNIFORM_BUFFER_BINDINGS; i++) {
		BufferMetal* buffer_metal = updateInfo->buffer[i];;
		if (buffer_metal != nullptr) {
			buffer_metal->setupBufferInDraw(this);
		}
	}
	return;
}

// IBOの更新を実行する
void ContextMetal::updateIBO(const IboUpdateInfo* updateInfo)
{
	AXGL_ASSERT(updateInfo != nullptr);
	BufferMetal* buffer_metal = updateInfo->buffer;
	if (buffer_metal != nullptr) {
		if (updateInfo->isUbyte) {
			buffer_metal->setU8U16ConversionMode();
		}
		bool result = buffer_metal->setupBufferInDraw(this, updateInfo->conversion, updateInfo->iboOffset, updateInfo->iboSize);
		AXGL_ASSERT(result);
	}
	return;
}

// 動的バッファの更新を行う
void ContextMetal::updateDynamicBuffers(VboDynamicUpdateInfo* vboInfo, UboDynamicUpdateInfo* uboInfo, IboDynamicUpdateInfo* iboInfo)
{
	AXGL_ASSERT((vboInfo != nullptr) && (uboInfo != nullptr));
	// VBO
	if (vboInfo->useDynamicBuffer) {
		for (int32_t i = 0; i < AXGL_MAX_VERTEX_ATTRIBS; i++) {
			if (vboInfo->buffer[i] != nullptr) {
				BufferMetal* buffer = vboInfo->buffer[i];
				size_t buffer_data_size = buffer->getBufferDataSize();
				// 動的バッファを用意してコピー
				setupDynamicBuffer(buffer_data_size);
				buffer->copyToDynamicBuffer(m_dynamicBuffer, m_dynamicBufferOffset);
				// コピーした情報を保持し、現在のオフセットを更新
				vboInfo->dynamicBuffer[i] = m_dynamicBuffer;
				vboInfo->offset[i] = m_dynamicBufferOffset;
				m_dynamicBufferOffset = get_aligned_buffer_offset(m_dynamicBufferOffset + buffer_data_size);
			}
		}
	}
	// UBO
	if (uboInfo->useDynamicBuffer) {
		for (int32_t i = 0; i < AXGL_MAX_UNIFORM_BUFFER_BINDINGS; i++) {
			if (uboInfo->buffer[i] != nullptr) {
				BufferMetal* buffer = uboInfo->buffer[i];
				size_t buffer_data_size = buffer->getBufferDataSize();
				// 動的バッファを用意してコピー
				setupDynamicBuffer(buffer_data_size);
				buffer->copyToDynamicBuffer(m_dynamicBuffer, m_dynamicBufferOffset);
				// コピーしたオフセットを保持し、現在のオフセットを更新
				uboInfo->dynamicBuffer[i] = m_dynamicBuffer;
				uboInfo->offset[i] = m_dynamicBufferOffset;
				m_dynamicBufferOffset = get_aligned_buffer_offset(m_dynamicBufferOffset + buffer_data_size);
			}
		}
	}
	// IBO
	if ((iboInfo != nullptr) && iboInfo->useDynamicBuffer) {
		if (iboInfo->buffer != nullptr) {
			BufferMetal* buffer = iboInfo->buffer;
			size_t buffer_data_size = buffer->getBufferDataSize();
			// 動的バッファを用意してコピー
			setupDynamicBuffer(buffer_data_size);
			buffer->copyToDynamicBuffer(m_dynamicBuffer, m_dynamicBufferOffset);
			// コピーしたオフセットを保持し、現在のオフセットを更新
			iboInfo->dynamicBuffer = m_dynamicBuffer;
			iboInfo->offset = m_dynamicBufferOffset;
			m_dynamicBufferOffset = get_aligned_buffer_offset(m_dynamicBufferOffset + buffer_data_size);
		}
	}
	return;
}

// 描画に使用するコマンドバッファを用意する
void ContextMetal::setupDrawCommandBuffer()
{
	if (m_drawCommandBuffer == nil) {
		// 描画用コマンドバッファが存在しなければ、コマンドキューから作成
		m_drawCommandBuffer = [m_commandQueue commandBuffer];
		AXGL_ASSERT(m_drawCommandBuffer != nil);
	}
	return;
}

// コマンドバッファをcommitする
void ContextMetal::commitDrawCommandBuffer(WaitMode waitMode)
{
	if (m_drawCommandBuffer == nil) {
		return;
	}
	// 使用中のコマンドエンコーダがある場合、エンコード終了
	endCommandEncoder();
	// コマンドバッファをcommit
	[m_drawCommandBuffer commit];
	// Scheduled/Completedが指定されている場合、コマンドバッファのwaitを呼び出す
	switch (waitMode) {
	case WaitModeScheduled:
		[m_drawCommandBuffer waitUntilScheduled];
		break;
	case WaitModeCompleted:
		[m_drawCommandBuffer waitUntilCompleted];
		break;
	case WaitModeNone:
	default:
		break;
	}
	m_drawCommandBuffer = nil;
	return;
}

// コマンドエンコーダを用意する
bool ContextMetal::setupRenderCommandEncoder(MTLRenderPassDescriptor* renderPassDesc)
{
	if (m_renderCommandEncoder != nil) {
		// エンコーダ作成済み:false
		return false;
	}
	AXGL_ASSERT(renderPassDesc != nil);
	// 念のためBlitコマンドエンコーダ終了を呼び出す
	endBlitCommandEncoder();
	// 描画コマンドエンコーダを作成
	AXGL_ASSERT(m_drawCommandBuffer != nil);
	m_renderCommandEncoder = [m_drawCommandBuffer renderCommandEncoderWithDescriptor:renderPassDesc];
	AXGL_ASSERT(m_renderCommandEncoder != nil);
	// 描画パラメータ設定をクリア
	m_setDrawParameterToEncoder = false;
	// エンコーダを作成:true
	return true;
}

// コマンドエンコーダを終了させる
void ContextMetal::endRenderCommandEncoder()
{
	if (m_renderCommandEncoder != nil) {
		[m_renderCommandEncoder endEncoding];
		m_renderCommandEncoder = nil;
		// 念のため描画パラメータ設定をクリアしておく
		m_setDrawParameterToEncoder = false;
	}
	return;
}

// Blit用のコマンドエンコーダを用意する
void ContextMetal::setupBlitCommandEncoder()
{
	if (m_blitCommandEncoder == nil) {
		// 描画コマンドエンコーダを終了
		endRenderCommandEncoder();
		AXGL_ASSERT(m_drawCommandBuffer != nil);
		// Blitコマンドエンコーダを作成
		m_blitCommandEncoder = [m_drawCommandBuffer blitCommandEncoder];
		AXGL_ASSERT(m_blitCommandEncoder != nil);
	}
	return;
}

// Blit用のコマンドエンコーダを終了させる
void ContextMetal::endBlitCommandEncoder()
{
	if (m_blitCommandEncoder != nil) {
		[m_blitCommandEncoder endEncoding];
		m_blitCommandEncoder = nil;
	}
	return;
}

// 全てのコマンドエンコーダを終了させる
void ContextMetal::endCommandEncoder()
{
	if (m_renderCommandEncoder != nil) {
		[m_renderCommandEncoder endEncoding];
		m_renderCommandEncoder = nil;
		// 念のため描画パラメータ設定をクリアしておく
		m_setDrawParameterToEncoder = false;
	}
	if (m_blitCommandEncoder != nil) {
		[m_blitCommandEncoder endEncoding];
		m_blitCommandEncoder = nil;
	}
	return;
}

// デフォルトUniformバッファを用意する
void ContextMetal::setupDefaultUniformBuffer(size_t size)
{
	AXGL_ASSERT((m_mtlDevice != nil) && (size > 0));
	if ((m_defaultUniformBuffer == nil) || ((m_defaultUniformBufferOffset + size) > c_default_uniform_buffer_size)) {
		if (m_defaultUniformBuffer != nil) {
			m_defaultUniformBuffer = nil;
		}
		// NOTE: 古いMTLBufferはARCによって描画が完了したら破棄される
		m_defaultUniformBuffer = [m_mtlDevice newBufferWithLength:c_default_uniform_buffer_size options:MTLResourceStorageModeShared];
		AXGL_ASSERT(m_defaultUniformBuffer != nil);
		m_defaultUniformBufferOffset = 0;
	}
	return;
}

// Dynamicバッファを用意する
void ContextMetal::setupDynamicBuffer(size_t size)
{
	AXGL_ASSERT((m_mtlDevice != nil) && (size > 0));
	if ((m_dynamicBuffer == nil) || ((m_dynamicBufferOffset + size) > c_dynamic_buffer_size)) {
		if (m_dynamicBuffer != nil) {
			m_dynamicBuffer = nil;
		}
		// NOTE: 古いMTLBufferはARCによって描画が完了したら破棄される
		m_dynamicBuffer = [m_mtlDevice newBufferWithLength:c_dynamic_buffer_size options:MTLResourceStorageModeShared];
		AXGL_ASSERT(m_dynamicBuffer != nil);
		m_dynamicBufferOffset = 0;
	}
	return;
}

// デフォルトUniform用のバッファを設定する
void ContextMetal::setBufferForDefaultUniform(id<MTLRenderCommandEncoder> encoder,
	int32_t vsIndex, int32_t fsIndex, const void* data, size_t size)
{
	AXGL_ASSERT((encoder != nil) && (data != nullptr) && (size > 0));
	if (m_defaultUniformBuffer == nil) {
		AXGL_DBGOUT("setBufferForDefaultUniform> m_defaultUniformBuffer is nil\n");
		return;
	}
	// MTLBufferにコピーする
	// GPUと競合しない領域を書き換えるため同期等は行わない
	AXGL_ASSERT(m_defaultUniformBuffer != nil);
	uint8_t* dst = static_cast<uint8_t*>([m_defaultUniformBuffer contents]);
	AXGL_ASSERT(dst != nullptr);
	dst += m_defaultUniformBufferOffset;
	AXGL_ASSERT((m_defaultUniformBufferOffset + size) <= [m_defaultUniformBuffer length]);
	memcpy(dst, data, size);
	// VertexBuffer/FragmentBufferとして設定し、オフセットを更新
	if (vsIndex >= 0) {
		[encoder setVertexBuffer:m_defaultUniformBuffer offset:m_defaultUniformBufferOffset atIndex:vsIndex];
	}
	if (fsIndex >= 0) {
		[encoder setFragmentBuffer:m_defaultUniformBuffer offset:m_defaultUniformBufferOffset atIndex:fsIndex];
	}
	m_defaultUniformBufferOffset = get_aligned_buffer_offset(m_defaultUniformBufferOffset + size);
	return;
}

// 各シェーダにデフォルトUniformバッファを設定する
void ContextMetal::setDefaultUniformBuffer(id<MTLRenderCommandEncoder> encoder, const ProgramMetal* program)
{
	AXGL_ASSERT((encoder != nil) && (program != nullptr));
	const void* global_memory = program->getGlobalBlockMemory();
	if (global_memory != nullptr) {
		size_t global_size = program->getGlobalBlockSize();
		setupDefaultUniformBuffer(global_size);
		int32_t vs_native_index = program->getGlobalBlockMetalIndex(GL_VERTEX_SHADER);
		int32_t fs_native_index = program->getGlobalBlockMetalIndex(GL_FRAGMENT_SHADER);
		setBufferForDefaultUniform(encoder, vs_native_index, fs_native_index, global_memory, global_size);
	}
	return;
}

// MTLDepthStencilDescriptorの設定を行う
void ContextMetal::setDepthStencilDescriptor(MTLDepthStencilDescriptor* dsDesc,
	const DepthStencilState& dsState)
{
	AXGL_ASSERT(dsDesc != nil);
	bool depth_enable = (dsState.depthTestParams.depthTestEnable != GL_FALSE);
	bool stencil_enable = (dsState.stencilTestParams.stencilTestEnable != GL_FALSE);
	dsDesc.label = @"TestDepthStencil";
	dsDesc.depthCompareFunction = convert_depth_stencil_compare_func(depth_enable, dsState.depthTestParams.depthFunc);
	dsDesc.depthWriteEnabled = convert_depth_write_mask(depth_enable, dsState.writemaskParams.depthWritemask);
	dsDesc.backFaceStencil.stencilFailureOperation = convert_stencil_op(stencil_enable, dsState.stencilTestParams.stencilBackFail);
	dsDesc.backFaceStencil.depthFailureOperation = convert_stencil_op(stencil_enable, dsState.stencilTestParams.stencilBackPassDepthFail);
	dsDesc.backFaceStencil.depthStencilPassOperation = convert_stencil_op(stencil_enable, dsState.stencilTestParams.stencilBackPassDepthPass);
	dsDesc.backFaceStencil.stencilCompareFunction = convert_depth_stencil_compare_func(stencil_enable,  dsState.stencilTestParams.stencilBackFunc);
	dsDesc.backFaceStencil.readMask = convert_stencil_mask(stencil_enable, dsState.stencilTestParams.stencilBackValueMask);
	dsDesc.backFaceStencil.writeMask = convert_stencil_mask(stencil_enable, dsState.writemaskParams.stencilBackWritemask);
	dsDesc.frontFaceStencil.stencilFailureOperation = convert_stencil_op(stencil_enable, dsState.stencilTestParams.stencilFail);
	dsDesc.frontFaceStencil.depthFailureOperation = convert_stencil_op(stencil_enable, dsState.stencilTestParams.stencilPassDepthFail);
	dsDesc.frontFaceStencil.depthStencilPassOperation = convert_stencil_op(stencil_enable, dsState.stencilTestParams.stencilPassDepthPass);
	dsDesc.frontFaceStencil.stencilCompareFunction = convert_depth_stencil_compare_func(stencil_enable, dsState.stencilTestParams.stencilFunc);
	dsDesc.frontFaceStencil.readMask = convert_stencil_mask(stencil_enable, dsState.stencilTestParams.stencilValueMask);
	dsDesc.frontFaceStencil.writeMask = convert_stencil_mask(stencil_enable, dsState.writemaskParams.stencilWritemask);
	return;
}

// ビューポートの垂直方向反転が必要か
bool ContextMetal::viewportNeedYFlip(const DrawParameters* drawParams)
{
	AXGL_ASSERT(drawParams != nullptr);
	bool need_yflip = false;
	if (drawParams->framebufferDraw != nullptr) {
		FramebufferMetal* framebuffer_metal = static_cast<FramebufferMetal*>(drawParams->framebufferDraw->getBackendFramebuffer());
		AXGL_ASSERT(framebuffer_metal != nullptr);
		need_yflip = framebuffer_metal->onlyOffscreenBufferAttached();
	}
	return need_yflip;
}

// Framebufferのサイズを取得する
void ContextMetal::getFramebufferSize(const DrawParameters* drawParams, RectSize* rectSize)
{
	AXGL_ASSERT((drawParams != nullptr) && (rectSize != nullptr));
	if (drawParams->framebufferDraw != nullptr) {
		FramebufferMetal* framebuffer_metal = static_cast<FramebufferMetal*>(drawParams->framebufferDraw->getBackendFramebuffer());
		AXGL_ASSERT(framebuffer_metal != nullptr);
		framebuffer_metal->getRectSize(rectSize);
	}
	return;
}

// MTLRenderPipelineStateを用意する(キャッシュ制御含む)
id<MTLRenderPipelineState> ContextMetal::setupRenderPipelineState(const DrawParameters* drawParams,
	const uint32_t* adjustedStride, bool* sameAsLastUsed)
{
	AXGL_ASSERT((drawParams != nullptr) && (adjustedStride != nullptr) && (sameAsLastUsed != nullptr));
	id<MTLRenderPipelineState> pipeline_state = nil;
	*sameAsLastUsed = false;
	// キャッシュからMTLRenderPipelineStateを検索
	auto ps_it = m_pipelineStateCache.find(drawParams->renderPipelineState);
	if (ps_it != m_pipelineStateCache.end()) {
		// キャッシュされているRenderPipelineStateを使用、キャッシュ使用履歴を更新
		pipeline_state = ps_it->second;
		AXGL_ASSERT(pipeline_state != nil);
		*sameAsLastUsed = updatePipelineStateUsedOrder(&(ps_it->first));
	} else {
		if (m_pipelineStateCache.size() >= c_pipeline_state_cache_max) {
			// キャッシュエントリ数を超える場合は、古いキャッシュを破棄
			AXGL_ASSERT(m_pipelineStateUsedOrder.size() == m_pipelineStateCache.size());
			const PipelineState* ps = m_pipelineStateUsedOrder.back();
			AXGL_ASSERT(ps != nullptr);
			auto it = m_pipelineStateCache.find(*ps);
			AXGL_ASSERT(it != m_pipelineStateCache.end());
			it->second = nil;
			m_pipelineStateCache.erase(it);
			m_pipelineStateUsedOrder.pop_back();
		}
		// MTLRenderPipelineDescriptorを作成
		MTLRenderPipelineDescriptor* ps_desc = [[MTLRenderPipelineDescriptor alloc] init];
		ps_desc.label = @"TestRenderPipelineDescriptor";
		// vertexFunction, fragmentFunction
		const ProgramMetal* program_metal = static_cast<const ProgramMetal*>(drawParams->renderPipelineState.program);
		AXGL_ASSERT(program_metal != nullptr);
		ps_desc.vertexFunction = program_metal->getVertexFunction();
		ps_desc.fragmentFunction = program_metal->getFragmentFunction();
		// GL頂点属性のlocationを取得
		const int32_t* locations = program_metal->getAttribLocations();
		AXGL_ASSERT(locations != nullptr);
		// VertexDescriptorを設定
		bool is_instanced = drawParams->renderPipelineState.isInstanced;
		if (drawParams->renderPipelineState.vertexArray != nullptr) {
			VertexArrayMetal* vertex_array_metal = static_cast<VertexArrayMetal*>(drawParams->renderPipelineState.vertexArray);
			setVertexDescriptorFromVAO(ps_desc, vertex_array_metal, locations, is_instanced,
				program_metal, adjustedStride);
		} else {
			setVertexDescriptorFromCurrentState(ps_desc, drawParams->renderPipelineState.vertexAttribs, locations, is_instanced,
				program_metal, adjustedStride);
		}
		const TargetAttachment& attachment = drawParams->renderPipelineState.targetAttachment;
		MTLPixelFormat pixel_format = MTLPixelFormatInvalid;
		// Color attachments
		for (int32_t i = 0; i < AXGL_MAX_COLOR_ATTACHMENTS; i++) {
			pixel_format = convert_internalformat(attachment.colorFormat[i]);
			if (pixel_format != MTLPixelFormatInvalid) {
				ps_desc.colorAttachments[i].pixelFormat = pixel_format;
				// writemask
				const ColorWritemaskParams& mask = drawParams->renderPipelineState.writemaskParams;
				ps_desc.colorAttachments[i].writeMask = convert_color_write_mask(
					(mask.colorWritemask[0] == GL_TRUE),
					(mask.colorWritemask[1] == GL_TRUE),
					(mask.colorWritemask[2] == GL_TRUE),
					(mask.colorWritemask[3] == GL_TRUE));
				// blend
				const BlendParams& blend = drawParams->renderPipelineState.blendParams;
				ps_desc.colorAttachments[i].blendingEnabled = (blend.blendEnable == GL_TRUE) ? YES : NO;
				ps_desc.colorAttachments[i].rgbBlendOperation = convert_blend_operation(blend.blendEquation[0]);;
				ps_desc.colorAttachments[i].alphaBlendOperation = convert_blend_operation(blend.blendEquation[1]);
				ps_desc.colorAttachments[i].sourceRGBBlendFactor = convert_blend_factor(blend.blendSrc[0]);
				ps_desc.colorAttachments[i].sourceAlphaBlendFactor = convert_blend_factor(blend.blendSrc[1]);
				ps_desc.colorAttachments[i].destinationRGBBlendFactor = convert_blend_factor(blend.blendDst[0]);
				ps_desc.colorAttachments[i].destinationAlphaBlendFactor = convert_blend_factor(blend.blendDst[1]);
			}
		}
		// Depth attachment
		pixel_format = convert_internalformat(attachment.depthFormat);
		if (pixel_format != MTLPixelFormatInvalid) {
			ps_desc.depthAttachmentPixelFormat = pixel_format;
		}
		// Stencil attachment
		pixel_format = convert_internalformat(attachment.stencilFormat);
		if (pixel_format != MTLPixelFormatInvalid) {
			ps_desc.stencilAttachmentPixelFormat = pixel_format;
		}
		// TODO:
		// sampleCount
		// alphaToCoverageEnabled
		// alphaToOneEnabled
		// rasterizationEnabled
		// inputPrimitiveTopology
		// rasterSampleCount

		// MTLRenderPipelineStateを作成
		NSError* error = nil;
		pipeline_state = [m_mtlDevice newRenderPipelineStateWithDescriptor:ps_desc error:&error];
		if (error != nil) {
			AXGL_DBGOUT([[error localizedDescription] UTF8String]);
		}
		AXGL_ASSERT(pipeline_state != nil);
		// キャッシュに追加
		auto rv_first = m_pipelineStateCache.emplace(drawParams->renderPipelineState, pipeline_state).first;
		// 使用履歴に追加
		m_pipelineStateUsedOrder.emplace_front(&(rv_first->first));
	}
	return pipeline_state;
}

// MTLDepthStencilStateを用意する(キャッシュ制御含む)
id<MTLDepthStencilState> ContextMetal::setupDepthStencilState(const DrawParameters* drawParams,
	bool* sameAsLastUsed)
{
	AXGL_ASSERT((drawParams != nullptr) && (sameAsLastUsed != nullptr));
	id<MTLDepthStencilState> depth_stencil_state = nil;
	*sameAsLastUsed = false;
	// 深度とステンシルのステートはFramebufferにバインドがない場合は無視する
	static DepthStencilState s_nullState;
	const DepthStencilState* findDepthStencilState = &drawParams->depthStencilState;
	if (drawParams->framebufferDraw != nullptr) {
		FramebufferMetal* framebuffer_metal = static_cast<FramebufferMetal*>(drawParams->framebufferDraw->getBackendFramebuffer());
		if (framebuffer_metal != nullptr) {
			if (framebuffer_metal->getMtlRenderPassDescriptor().depthAttachment.texture == nil) {
				findDepthStencilState = &s_nullState;
			}
		}
	}
	// キャッシュからMTLDepthStencilStateを検索
	auto ds_it = m_depthStencilStateCache.find(*findDepthStencilState);
	if (ds_it != m_depthStencilStateCache.end()) {
		// キャッシュされているDepthStencilStateを使用、キャッシュ使用履歴を更新
		depth_stencil_state = ds_it->second;
		AXGL_ASSERT(depth_stencil_state != nil);
		*sameAsLastUsed = updateDepthStencilStateUsedOrder(&(ds_it->first));
	} else {
		if (m_depthStencilStateCache.size() >= c_depth_stencil_state_cache_max) {
			// キャッシュエントリ数を超える場合は、古いキャッシュを破棄
			const DepthStencilState* dss = m_depthStencilStateUsedOrder.back();
			AXGL_ASSERT(dss != nullptr);
			auto it = m_depthStencilStateCache.find(*dss);
			AXGL_ASSERT(it != m_depthStencilStateCache.end());
			it->second = nil;
			m_depthStencilStateCache.erase(it);
			m_depthStencilStateUsedOrder.pop_back();
		}
		// MTLDepthStencilDescriptorを作成
		MTLDepthStencilDescriptor* ds_desc = [[MTLDepthStencilDescriptor alloc] init];
		setDepthStencilDescriptor(ds_desc, *findDepthStencilState);
		// MTLDepthStencilStateを作成
		depth_stencil_state = [m_mtlDevice newDepthStencilStateWithDescriptor:ds_desc];
		AXGL_ASSERT(depth_stencil_state != nil);
		// キャッシュに追加
		auto rv_first = m_depthStencilStateCache.emplace(*findDepthStencilState, depth_stencil_state).first;
		// 使用履歴に追加
		m_depthStencilStateUsedOrder.emplace_front(&(rv_first->first));
	}
	return depth_stencil_state;
}

// MTLRenderPassDescriptorを用意する(実体はFramebufferが持つ)
MTLRenderPassDescriptor* ContextMetal::setupRenderPassDescriptor(const DrawParameters* drawParams,
	const ClearParameters* clearParams, bool* changed)
{
	AXGL_ASSERT((drawParams != nullptr) && (clearParams != nullptr) && (changed != nullptr));
	// RenderPassDescriptorを準備
	MTLRenderPassDescriptor* render_pass_desc = nil;
	if (drawParams->framebufferDraw != nullptr) {
		bool descriptor_modified = false;
		FramebufferMetal* framebuffer_metal = static_cast<FramebufferMetal*>(drawParams->framebufferDraw->getBackendFramebuffer());
		if (framebuffer_metal != nullptr) {
			QueryMetal* query_metal = nullptr;
			if (drawParams->anySamplesPassedQuery != nullptr) {
				query_metal = static_cast<QueryMetal*>(drawParams->anySamplesPassedQuery->getBackendQuery());
			} else if (drawParams->anySamplesPassedConservativeQuery != nullptr) {
				query_metal = static_cast<QueryMetal*>(drawParams->anySamplesPassedConservativeQuery->getBackendQuery());
			}
			descriptor_modified = framebuffer_metal->setupRenderPassDescriptor(query_metal, 0, clearParams);
			render_pass_desc = framebuffer_metal->getMtlRenderPassDescriptor();
			AXGL_ASSERT(render_pass_desc != nil);
		}
		// 変更されたかを格納
		*changed = (m_renderFramebuffer != framebuffer_metal) || descriptor_modified;
		// 使用したFramebufferを保持
		m_renderFramebuffer = framebuffer_metal;
	}
	return render_pass_desc;
}

// ビューポートを設定する
void ContextMetal::setViewport(id<MTLRenderCommandEncoder> encoder, const ViewportParams* viewport, bool flipY)
{
	AXGL_ASSERT((encoder != nil) && (viewport != nullptr));
	double mtl_origin_y;
	double mtl_height;
	if (flipY) {
		mtl_origin_y = static_cast<double>(viewport->viewport[1] + viewport->viewport[3]);
		mtl_height = static_cast<double>(-viewport->viewport[3]);
	} else {
		mtl_origin_y = static_cast<double>(viewport->viewport[1]);
		mtl_height = static_cast<double>(viewport->viewport[3]);
	}
	// TODO: glViewportで部分描画を行なっている場合、Render targetサイズを絡めて更に調整が必要
	MTLViewport mtl_viewport = { // NOTE: struct
		static_cast<double>(viewport->viewport[0]),
		mtl_origin_y,
		static_cast<double>(viewport->viewport[2]),
		mtl_height,
		static_cast<double>(viewport->depthRange[0]),
		static_cast<double>(viewport->depthRange[1])
	};
	[encoder setViewport:mtl_viewport];
	return;
}

// 通常のカレントステートから頂点バッファを設定する
void ContextMetal::setVertexBufferForVBO(id<MTLRenderCommandEncoder> encoder, const CoreBuffer* const* vertexBuffers,
	const ProgramMetal* program, const VertexAttrib* vertexAttribs, const VboDynamicUpdateInfo* dynamicUpdateInfo)
{
	AXGL_ASSERT((encoder != nil) && (vertexBuffers != nullptr) && (program != nullptr) && (vertexAttribs != nullptr) && (dynamicUpdateInfo != nullptr));
	// GL頂点属性のlocationを取得
	const int* locations = program->getAttribLocations();
	AXGL_ASSERT(locations != nullptr);
	// MetalのVertexBufferを設定
	for (int32_t i = 0; i < AXGL_MAX_VERTEX_ATTRIBS; i++) {
		int loc = locations[i];
		if (loc >= 0) {
			if (vertexAttribs[loc].enable == GL_TRUE) {
				if (dynamicUpdateInfo->dynamicBuffer[i] != nil) {
					// 動的バッファを使用
					id<MTLBuffer> mtl_buffer = dynamicUpdateInfo->dynamicBuffer[i];
					size_t offset = dynamicUpdateInfo->offset[i];
					[encoder setVertexBuffer:mtl_buffer offset:offset atIndex:(i + c_vbo_index_offset)];
				} else {
					// VBOからバッファの実体を取得
					id<MTLBuffer> buffer_metal = nil;
					if (vertexBuffers[loc] != nullptr) {
						BufferMetal* backend_buffer = static_cast<BufferMetal*>(vertexBuffers[loc]->getBackendBuffer());
						if (backend_buffer != nullptr) {
							buffer_metal = backend_buffer->getMtlBuffer();
						}
					}
					if (buffer_metal != nil) {
						// バッファを設定
						[encoder setVertexBuffer:buffer_metal offset:0 atIndex:(i + c_vbo_index_offset)];
					} else {
						// バッファが取得できなかった場合は固定値を設定しておく
						static const float constantData[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
						[encoder setVertexBytes:constantData length:sizeof(constantData) atIndex:(i + c_vbo_index_offset)];
					}
				}
			} else {
				// 頂点属性がDisableの場合は固定値バッファを設定
				[encoder setVertexBuffer:m_disableBuffer offset:0 atIndex:(i + c_vbo_index_offset)];
			}
		}
	}

	return;
}

// VertexArrayObjectから頂点バッファを設定する
void ContextMetal::setVertexBufferForVBOFromVAO(id<MTLRenderCommandEncoder> encoder, VertexArrayMetal* vertexArray,
	const ProgramMetal* program, bool setAll, const VboDynamicUpdateInfo* dynamicUpdateInfo)
{
	AXGL_ASSERT((encoder != nil) && (vertexArray != nullptr) && (program != nullptr) && (dynamicUpdateInfo != nullptr));
	// GL頂点属性のlocationを取得
	const int* locations = program->getAttribLocations();
	AXGL_ASSERT(locations != nullptr);
	// Vertex attribute parameters
	const VertexArrayMetal::AttribParamMetal* ap = vertexArray->getAttribParams();
	// MetalのVertexBufferを設定
	BufferMetal* buffer_for_clear[AXGL_MAX_VERTEX_ATTRIBS] = {nullptr};
	unsigned int num_buffer_for_clear = 0;
	for (unsigned int i = 0; i < AXGL_MAX_VERTEX_ATTRIBS; i++) {
		int loc = locations[i];
		if (loc >= 0) {
			AXGL_ASSERT(loc < AXGL_MAX_VERTEX_ATTRIBS);
			if (ap[loc].enabled) {
				if (dynamicUpdateInfo->dynamicBuffer[i] != nil) {
					// 動的バッファを使用
					id<MTLBuffer> mtl_buffer = dynamicUpdateInfo->dynamicBuffer[i];
					size_t offset = dynamicUpdateInfo->offset[i];
					[encoder setVertexBuffer:mtl_buffer offset:offset atIndex:(i + c_vbo_index_offset)];
				} else {
					// VBOからバッファの実体を取得
					BufferMetal* backend_buffer = ap[loc].buffer;
					id<MTLBuffer> mtl_buffer = nil;
					bool mtl_buffer_dirty = false;
					if (backend_buffer != nullptr) {
						mtl_buffer = backend_buffer->getMtlBuffer();
						mtl_buffer_dirty = backend_buffer->isMTLBufferDirty();
						// 一連の処理が終わった後にダーティクリアするために保持
						if (mtl_buffer_dirty) {
							buffer_for_clear[num_buffer_for_clear] = backend_buffer;
							num_buffer_for_clear++;
						}
					}
					// ストライドやTriangleFanのデータ変換が複数回行われるケースを想定して、
					// MTLBuffer が再生成されたかを設定条件に含める
					if (setAll || mtl_buffer_dirty) {
						if (mtl_buffer) {
							// バッファを設定
							[encoder setVertexBuffer:mtl_buffer offset:0 atIndex:(i + c_vbo_index_offset)];
						} else {
							// バッファが取得できなかった場合は固定値を設定しておく
							static const float constantData[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
							[encoder setVertexBytes:constantData length:sizeof(constantData) atIndex:(i + c_vbo_index_offset)];
						}
					}
				}
			} else {
				if (setAll) {
					// 頂点属性がDisableの場合は固定値バッファを設定
					[encoder setVertexBuffer:m_disableBuffer offset:0 atIndex:(i + c_vbo_index_offset)];
				}
			}
		}
	}
	// 設定した MTLBuffer のダーティクリアしておく
	for (unsigned int i = 0; i < num_buffer_for_clear; i++) {
		AXGL_ASSERT(buffer_for_clear[i] != nullptr);
		buffer_for_clear[i]->clearMTLBufferDirty();
	}
	// VertexArray のダーティをクリア
	vertexArray->clearVertexBufferDirty();
	return;
}

// Uniformバッファを設定する
void ContextMetal::setBufferForUBO(id<MTLRenderCommandEncoder> encoder, const ProgramMetal* program,
	const IndexedBuffer* uniformBuffers, bool setAll, const UboDynamicUpdateInfo* dynamicUpdateInfo)
{
	AXGL_ASSERT((encoder != nil) && (program != nullptr) && (uniformBuffers != nullptr) && (dynamicUpdateInfo != nullptr));
	// ub_binding : GL側のqualifier binding
	const int32_t* ub_binding = program->getUniformBlockBinding();
	// vs_metal_indices/fs_metal_indices : VS/FSのMetalにおけるbuffer index
	const int32_t* vs_metal_indices = program->getVsUniformBlockMetalIndices();
	const int32_t* fs_metal_indices = program->getFsUniformBlockMetalIndices();
	bool set_all_ubo = setAll || program->isUniformBlockBindingDirty();
	AXGL_ASSERT((ub_binding != nullptr) && (vs_metal_indices != nullptr) && (fs_metal_indices != nullptr));
	// UBOに相当するMetalのvertex/fragment bufferを設定
	BufferMetal* used_buffer[AXGL_MAX_UNIFORM_BUFFER_BINDINGS] = {nullptr};
	for (unsigned int i = 0; i < AXGL_MAX_UNIFORM_BUFFER_BINDINGS; i++) {
		int32_t ub_index = ub_binding[i]; // GLのbinding
		int32_t vs_metal_index = vs_metal_indices[i]; // Metal VSのバッファインデックス
		int32_t fs_metal_index = fs_metal_indices[i]; // Metal FSのバッファインデックス
		// Vertex shader
		if ((ub_index >= 0) && (vs_metal_index >= 0)) {
			AXGL_ASSERT(ub_index < AXGL_MAX_UNIFORM_BUFFER_BINDINGS);
			const IndexedBuffer* ub = &uniformBuffers[ub_index];
			if (dynamicUpdateInfo->dynamicBuffer[ub_index] != nil) {
				// 動的バッファを使用
				id<MTLBuffer> mtl_buffer = dynamicUpdateInfo->dynamicBuffer[ub_index];
				size_t offset = dynamicUpdateInfo->offset[ub_index] + ub->offset;
				[encoder setVertexBuffer:mtl_buffer offset:offset atIndex:vs_metal_index];
			} else {
				if (ub->buffer != nullptr) {
					id<MTLBuffer> buffer_metal = nil;
					unsigned int buffer_offset = 0;
					BufferMetal* backend_buffer = static_cast<BufferMetal*>(ub->buffer->getBackendBuffer());
					if (backend_buffer != nullptr) {
						if (set_all_ubo || backend_buffer->isMTLBufferDirty()) {
							buffer_metal = backend_buffer->getMtlBuffer();
							buffer_offset = static_cast<unsigned int>(ub->offset);
							used_buffer[ub_index] = backend_buffer;
							[encoder setVertexBuffer:buffer_metal offset:buffer_offset atIndex:vs_metal_index];
						}
					}
				}
			}
		}
		// Fragment shader
		if ((ub_index >= 0) && (fs_metal_index >= 0)) {
			AXGL_ASSERT(ub_index < AXGL_MAX_UNIFORM_BUFFER_BINDINGS);
			const IndexedBuffer* ub = &uniformBuffers[ub_index];
			if (dynamicUpdateInfo->dynamicBuffer[ub_index] != nil) {
				// 動的バッファを使用
				id<MTLBuffer> mtl_buffer = dynamicUpdateInfo->dynamicBuffer[ub_index];
				size_t offset = dynamicUpdateInfo->offset[ub_index] + ub->offset;
				[encoder setFragmentBuffer:mtl_buffer offset:offset atIndex:fs_metal_index];
			} else {
				if (ub->buffer != nullptr) {
					id<MTLBuffer> buffer_metal = nil;
					unsigned int buffer_offset = 0;
					BufferMetal* backend_buffer = static_cast<BufferMetal*>(ub->buffer->getBackendBuffer());
					if (backend_buffer != nullptr) {
						if (set_all_ubo || backend_buffer->isMTLBufferDirty()) {
							buffer_metal = backend_buffer->getMtlBuffer();
							buffer_offset = static_cast<unsigned int>(ub->offset);
							used_buffer[ub_index] = backend_buffer;
							[encoder setFragmentBuffer:buffer_metal offset:buffer_offset atIndex:fs_metal_index];
						}
					}
				}
			}
		}
	}
	// 設定したMTLBufferのダーティをクリアする
	for (unsigned int i = 0; i < AXGL_MAX_UNIFORM_BUFFER_BINDINGS; i++) {
		if (used_buffer[i] != nullptr) {
			used_buffer[i]->clearMTLBufferDirty();
		}
	}
	return;
}

// テクスチャとサンプラを設定する
void ContextMetal::setTextureAndSampler(id<MTLRenderCommandEncoder> encoder, const ProgramMetal* program,
	const DrawParameters* drawParams, bool setAll)
{
	AXGL_ASSERT((encoder != nil) && (program != nullptr) && (drawParams != nullptr));
	const ProgramMetal* program_metal = program;
	bool program_dirty = program_metal->isSamplerUnitDirty();
	bool texture_binding_dirty = setAll || program_dirty ||
		((drawParams->dirtyFlags & (CURRENT_PROGRAM_DIRTY_BIT | TEXTURE_BINDING_DIRTY_BIT)) != 0);
	// NOTE: サンプラは GLのsampler objectとtexture objectが影響
	bool sampler_binding_dirty = setAll || program_dirty ||
		((drawParams->dirtyFlags & (CURRENT_PROGRAM_DIRTY_BIT | TEXTURE_BINDING_DIRTY_BIT | SAMPLER_BINDING_DIRTY_BIT)) != 0);

	// 頂点シェーダにテクスチャとサンプラを設定
	// 設定したテクスチャとサンプラを格納する配列(後でダーティクリアに使用)
	TextureMetal* vs_textures[AXGL_MAX_COMBINED_TEXTURE_IMAGE_UNITS];
	SamplerMetal* vs_samplers[AXGL_MAX_COMBINED_TEXTURE_IMAGE_UNITS];
	const int32_t num_texture_sampler = program_metal->getNumSampler();
	const int32_t* vs_metal_indices = program_metal->getVsTextureSamplerMetalIndices();
	int32_t num_vs_texture = 0;
	int32_t num_vs_sampler = 0;
	{
		// Sampler draw parameters
		const ProgramMetal::SamplerDrawParams* sampler_params = program_metal->getSamplerDrawParams();
		AXGL_ASSERT(sampler_params != nullptr);
		for (int32_t i = 0; i < num_texture_sampler; i++) {
			int32_t unit_index = sampler_params[i].unit; // GLのbinding
			int32_t metal_index = vs_metal_indices[i]; // Metalのtexture/samplerインデックス
			if ((unit_index >= 0) && (metal_index >= 0)) {
				// テクスチャ設定
				// unit_index : GL binding texture unit index
				// metal_index : metal texture index
				const CoreTexture* core_texture = get_core_texture(drawParams, sampler_params[i].type, unit_index);
				if (core_texture != nullptr) {
					TextureMetal* backend_texture = static_cast<TextureMetal*>(core_texture->getBackendTexture());
					if ((backend_texture != nullptr) && (texture_binding_dirty || backend_texture->isDirty())) {
						id<MTLTexture> texture_metal = backend_texture->getMtlTexture();
						AXGL_ASSERT(texture_metal != nil);
						[encoder setVertexTexture:texture_metal atIndex:metal_index];
						// 一連の設定が終わった後でダーティクリアするため保持
						vs_textures[num_vs_texture] = backend_texture;
						num_vs_texture++;
					}
				}
				// サンプラ設定
				// unit_index : GL binding texture unit index
				// metal_index : metal sampler index
				const CoreSampler* const* samplers = drawParams->samplers;
				SamplerMetal* backend_sampler = nullptr;
				if (samplers[unit_index] != nullptr) {
					// from sampler object
					backend_sampler = static_cast<SamplerMetal*>(samplers[unit_index]->getBackendSampler());
				} else if (core_texture != nullptr) {
					// from texture object
					backend_sampler = static_cast<SamplerMetal*>(core_texture->getBackendSampler());
				}
				if ((backend_sampler != nullptr) && (sampler_binding_dirty || backend_sampler->isDirty())) {
					id<MTLSamplerState> sampler_state = backend_sampler->getMtlSamplerState();
					AXGL_ASSERT(sampler_state != nil);
					[encoder setVertexSamplerState:sampler_state atIndex:metal_index];
					// 一連の設定が終わった後でダーティクリアするため保持
					vs_samplers[num_vs_sampler] = backend_sampler;
					num_vs_sampler++;
				}
			}
		}
	}
	// フラグメントシェーダにテクスチャとサンプラを設定
	TextureMetal* fs_textures[AXGL_MAX_COMBINED_TEXTURE_IMAGE_UNITS];
	SamplerMetal* fs_samplers[AXGL_MAX_COMBINED_TEXTURE_IMAGE_UNITS];
	const int32_t* fs_metal_indices = program_metal->getFsTextureSamplerMetalIndices();
	int32_t num_fs_texture = 0;
	int32_t num_fs_sampler = 0;
	{
		// Sampler draw parameters
		const ProgramMetal::SamplerDrawParams* sampler_params = program_metal->getSamplerDrawParams();
		AXGL_ASSERT(sampler_params != nullptr);
		for (int32_t i = 0; i < num_texture_sampler; i++) {
			int32_t unit_index = sampler_params[i].unit; // GLのbinding
			int32_t metal_index = fs_metal_indices[i]; // Metalのtexture/samplerインデックス
			if ((unit_index >= 0) && (metal_index >= 0)) {
				// テクスチャ設定
				// unit_index : GL binding texture unit index
				// metal_index : metal texture index
				const CoreTexture* core_texture = get_core_texture(drawParams, sampler_params[i].type, unit_index);
				if (core_texture != nullptr) {
					TextureMetal* backend_texture = static_cast<TextureMetal*>(core_texture->getBackendTexture());
					if ((backend_texture != nullptr) && (texture_binding_dirty || backend_texture->isDirty())) {
						id<MTLTexture> texture_metal = backend_texture->getMtlTexture();
						AXGL_ASSERT(texture_metal != nil);
						[encoder setFragmentTexture:texture_metal atIndex:metal_index];
						// 一連の設定が終わった後でダーティクリアするため保持
						fs_textures[num_fs_texture] = backend_texture;
						num_fs_texture++;
					}
				}
				// サンプラ設定
				// unit_index : GL binding texture unit index
				// metal_index : metal sampler index
				const CoreSampler* const* samplers = drawParams->samplers;
				SamplerMetal* backend_sampler = nullptr;
				if (samplers[unit_index] != nullptr) {
					// from sampler object
					backend_sampler = static_cast<SamplerMetal*>(samplers[unit_index]->getBackendSampler());
				} else if (core_texture != nullptr) {
					// from texture object
					backend_sampler = static_cast<SamplerMetal*>(core_texture->getBackendSampler());
				}
				if ((backend_sampler != nullptr) && (sampler_binding_dirty || backend_sampler->isDirty())) {
					id<MTLSamplerState> sampler_state = backend_sampler->getMtlSamplerState();
					AXGL_ASSERT(sampler_state != nil);
					[encoder setFragmentSamplerState:sampler_state atIndex:metal_index];
					// 一連の設定が終わった後でダーティクリアするため保持
					fs_samplers[num_fs_sampler] = backend_sampler;
					num_fs_sampler++;
				}
			}
		}
	}

	// clear dirty
	// Texture,Sampler のバインディングが同一で Metal のインスタンスが再作成されていることを検出するための dirty
	// フラグクリアするだけであるため重複していても問題ない
	for (int32_t i = 0; i < num_vs_texture; i++) {
		AXGL_ASSERT(vs_textures[i] != nullptr);
		vs_textures[i]->clearDirty();
	}
	for (int32_t i = 0; i < num_vs_sampler; i++) {
		AXGL_ASSERT(vs_samplers[i] != nullptr);
		vs_samplers[i]->clearDirty();
	}
	for (int32_t i = 0; i < num_fs_texture; i++) {
		AXGL_ASSERT(fs_textures[i] != nullptr);
		fs_textures[i]->clearDirty();
	}
	for (int32_t i = 0; i < num_fs_sampler; i++) {
		AXGL_ASSERT(fs_samplers[i] != nullptr);
		fs_samplers[i]->clearDirty();
	}

	return;
}

// MetalのVisibilityResultModeを設定する(Queryで使用する)
void ContextMetal::setVisibilityResultMode(id<MTLRenderCommandEncoder> encoder, const DrawParameters* drawParams,
	bool setAll)
{
	AXGL_ASSERT((encoder != nil) && (drawParams != nullptr));
	// GL_ANY_SAMPLES_PASSED_QUERY と GL_ANY_SAMPLES_PASSED_CONSERVATIVE_QUERY は
	// その用途から同時に有効にならない前提
	CoreQuery* core_query = drawParams->anySamplesPassedQuery;
	if (core_query == nullptr) {
		core_query = drawParams->anySamplesPassedConservativeQuery;
	}
	if (core_query != nullptr) {
		// Query有効時はVisibilityResultBufferのオフセットを増やしていく必要があるため、毎回設定する
		MTLVisibilityResultMode visibility_result_mode = MTLVisibilityResultModeDisabled;
		NSUInteger offset_value = 0;
		QueryMetal* query_metal = static_cast<QueryMetal*>(core_query->getBackendQuery());
		if (query_metal != nullptr) {
			visibility_result_mode = MTLVisibilityResultModeBoolean;
			offset_value = query_metal->getVisibilityResultBufferOffset();
			bool result = query_metal->incrementOffset();
			AXGL_ASSERT(result);
		}
		[encoder setVisibilityResultMode:visibility_result_mode offset:offset_value];
	} else {
		// Query無効時はCurrent queryに変化があった時のみ設定する
		if (setAll || ((drawParams->dirtyFlags & ANY_SAMPLES_PASSED_QUERY_DIRTY_BIT) != 0)) {
			[encoder setVisibilityResultMode:MTLVisibilityResultModeDisabled offset:0];
		}
	}
	return;
}

// 表裏とカリングの設定を行う
void ContextMetal::setCullMode(id<MTLRenderCommandEncoder> encoder, const CullFaceParams* cullFaceParams, bool flipY)
{
	AXGL_ASSERT((encoder != nil) && (cullFaceParams != nullptr));
	MTLWinding winding = convert_winding(cullFaceParams->frontFace, flipY);
	[encoder setFrontFacingWinding:winding];
	MTLCullMode cull_mode = convert_cull_mode(cullFaceParams->cullFaceEnable, cullFaceParams->cullFaceMode);
	[encoder setCullMode:cull_mode];
	return;
}

// Scissorを設定する
void ContextMetal::setScissor(id<MTLRenderCommandEncoder> encoder, const ScissorParams* scissorParams,
	const RectSize* rectSize, bool flipY)
{
	AXGL_ASSERT((encoder != nil) && (scissorParams != nullptr) && (rectSize != nullptr));
	if (scissorParams->scissorTestEnable == GL_TRUE) {
		const GLint* box_gl = scissorParams->scissorBox;
		int32_t tmp;
		int32_t left = std::min(std::max(box_gl[0], 0), (int32_t)rectSize->width);
		tmp = flipY ? box_gl[1] : rectSize->height - (box_gl[1] + box_gl[3]);
		int32_t top = std::min(std::max(tmp, 0), (int32_t)rectSize->height);
		tmp = box_gl[0] + box_gl[2];
		int32_t right  = std::min(std::max(tmp, 0), (int32_t)rectSize->width);
		tmp = flipY ? (box_gl[1] + box_gl[3]) : (rectSize->height - box_gl[1]);
		int32_t bottom = std::min(std::max(tmp, 0), (int32_t)rectSize->height);
		MTLScissorRect rect;
		rect.x = left;
		rect.width = right - left;
		rect.y = top;
		rect.height = bottom - top;
		[encoder setScissorRect:rect];
	} else {
		// NOTE: エンコーダを使い回すためDisableはデフォルト値に戻す
		MTLScissorRect default_rect = {
			0, // x
			0, // y
			rectSize->width, // width
			rectSize->height // height
		};
		[encoder setScissorRect:default_rect];
	}
	return;
}

// 深度バイアスを設定する
void ContextMetal::setDepthBias(id<MTLRenderCommandEncoder> encoder, const PolygonOffsetParams* polygonOffsetParams)
{
	AXGL_ASSERT((encoder != nil) && (polygonOffsetParams != nullptr));
	if (polygonOffsetParams->polygonOffsetFillEnable == GL_TRUE) {
		[encoder setDepthBias:polygonOffsetParams->polygonOffsetUnits
			slopeScale:polygonOffsetParams->polygonOffsetFactor clamp:0.0f];
	} else {
		// NOTE: エンコーダを使い回すためDisableはデフォルト値に戻す
		[encoder setDepthBias:0.0f slopeScale:0.0f clamp:0.0f];
	}
	return;
}

// ステンシルの参照値を設定する
void ContextMetal::setStencilReference(id<MTLRenderCommandEncoder> encoder, const StencilReference* stencilReference)
{
	AXGL_ASSERT((encoder != nil) && (stencilReference != nullptr));
	[encoder setStencilFrontReferenceValue:stencilReference->stencilRef backReferenceValue:stencilReference->stencilBackRef];
	return;
}

// カレントステートからVertexDescriptor(MTLRenderPipelineDescriptorの子要素)を設定する
void ContextMetal::setVertexDescriptorFromCurrentState(MTLRenderPipelineDescriptor* pipelineDesc, const VertexAttrib* vertexAttribs,
	const int32_t* locations, bool isInstanced, const ProgramMetal* program, const uint32_t* adjustedStride)
{
	AXGL_ASSERT((pipelineDesc != nullptr) && (vertexAttribs != nullptr) && (locations != nullptr) && (program != nullptr));
	AXGL_ASSERT(adjustedStride != nullptr);

	for (int32_t i = 0; i < AXGL_MAX_VERTEX_ATTRIBS; i++) {
		int32_t loc = locations[i];
		if (loc >= 0) {
			AXGL_ASSERT(loc < AXGL_MAX_VERTEX_ATTRIBS);
			const VertexAttrib& va = vertexAttribs[loc];
			int32_t metal_index = program->getActiveVertexAttribIndex(i);
			if (metal_index >= 0) {
				int32_t vb_index = i + c_vbo_index_offset;
				MTLVertexFormat vertex_format = convert_vertex_format(va.type, va.size, (va.normalized == GL_TRUE));
				if (va.enable == GL_TRUE) {
					// 頂点バッファ用の設定
					pipelineDesc.vertexDescriptor.attributes[metal_index].format = vertex_format;
					pipelineDesc.vertexDescriptor.attributes[metal_index].offset = (NSUInteger)((uintptr_t)va.pointer);
					pipelineDesc.vertexDescriptor.attributes[metal_index].bufferIndex = vb_index;
					pipelineDesc.vertexDescriptor.layouts[vb_index].stride = adjustedStride[i];
					if (isInstanced && (va.divisor != 0)) {
						pipelineDesc.vertexDescriptor.layouts[vb_index].stepRate = va.divisor;
						pipelineDesc.vertexDescriptor.layouts[vb_index].stepFunction = MTLVertexStepFunctionPerInstance;
					} else {
						pipelineDesc.vertexDescriptor.layouts[vb_index].stepRate = 1;
						pipelineDesc.vertexDescriptor.layouts[vb_index].stepFunction = MTLVertexStepFunctionPerVertex;
					}
				} else {
					// Disableの場合は、固定値を設定
					pipelineDesc.vertexDescriptor.attributes[metal_index].format = vertex_format;
					pipelineDesc.vertexDescriptor.attributes[metal_index].offset = 0;
					pipelineDesc.vertexDescriptor.attributes[metal_index].bufferIndex = vb_index;
					pipelineDesc.vertexDescriptor.layouts[vb_index].stride = 16;
					pipelineDesc.vertexDescriptor.layouts[vb_index].stepRate = 0;
					pipelineDesc.vertexDescriptor.layouts[vb_index].stepFunction = MTLVertexStepFunctionConstant;
				}
			}
		}
	}
	return;
}

// VertexArrayObjectからVertexDescriptor(MTLRenderPipelineDescriptorの子要素)を設定する
void ContextMetal::setVertexDescriptorFromVAO(MTLRenderPipelineDescriptor* pipelineDesc, const VertexArrayMetal* vertexArray,
   const int32_t* locations, bool isInstanced, const ProgramMetal* program, const uint32_t* adjustedStride)
{
	AXGL_ASSERT((pipelineDesc != nullptr) && (vertexArray != nullptr) && (locations != nullptr) && (program != nullptr));
	AXGL_ASSERT(adjustedStride != nullptr);
	const VertexArrayMetal::AttribParamMetal* attribs = vertexArray->getAttribParams();
	AXGL_ASSERT(attribs != nullptr);
	for (int32_t i = 0; i < AXGL_MAX_VERTEX_ATTRIBS; i++) {
		int32_t loc = locations[i];
		if (loc >= 0) {
			AXGL_ASSERT(loc < AXGL_MAX_VERTEX_ATTRIBS);
			const VertexArrayMetal::AttribParamMetal& ap = attribs[loc];
			int32_t metal_index = program->getActiveVertexAttribIndex(i);
			if (metal_index >= 0) {
				int32_t vb_index = i + c_vbo_index_offset;
				if (ap.enabled == GL_TRUE) {
					// 頂点バッファ用の設定
					pipelineDesc.vertexDescriptor.attributes[metal_index].format = ap.format;
					pipelineDesc.vertexDescriptor.attributes[metal_index].offset = ap.offset;
					pipelineDesc.vertexDescriptor.attributes[metal_index].bufferIndex = vb_index;
					pipelineDesc.vertexDescriptor.layouts[vb_index].stride = adjustedStride[i];
					if (isInstanced && (ap.divisor != 0)) {
						pipelineDesc.vertexDescriptor.layouts[vb_index].stepRate = ap.divisor;
						pipelineDesc.vertexDescriptor.layouts[vb_index].stepFunction = MTLVertexStepFunctionPerInstance;
					} else {
						pipelineDesc.vertexDescriptor.layouts[vb_index].stepRate = 1;
						pipelineDesc.vertexDescriptor.layouts[vb_index].stepFunction = MTLVertexStepFunctionPerVertex;
					}
				} else {
					// Disableの場合は、固定値を設定
					pipelineDesc.vertexDescriptor.attributes[metal_index].format = ap.format;
					pipelineDesc.vertexDescriptor.attributes[metal_index].offset = 0;
					pipelineDesc.vertexDescriptor.attributes[metal_index].bufferIndex = vb_index;
					pipelineDesc.vertexDescriptor.layouts[vb_index].stride = 16;
					pipelineDesc.vertexDescriptor.layouts[vb_index].stepRate = 0;
					pipelineDesc.vertexDescriptor.layouts[vb_index].stepFunction = MTLVertexStepFunctionConstant;
				}
			}
		}
	}
	return;
}

// インデックスバッファの変換モードを取得する
BackendBuffer::ConversionMode ContextMetal::getIboConversionMode(GLenum mode, GLenum type)
{
	BackendBuffer::ConversionMode conversion = BackendBuffer::ConversionModeNone;
	if (mode == GL_TRIANGLE_FAN) {
		switch (type) {
		case GL_UNSIGNED_BYTE:
			conversion = BackendBuffer::ConversionModeTriFanIndices8;
			break;
		case GL_UNSIGNED_SHORT:
			conversion = BackendBuffer::ConversionModeTriFanIndices16;
			break;
		case GL_UNSIGNED_INT:
			conversion = BackendBuffer::ConversionModeTriFanIndices32;
			break;
		default:
			AXGL_ASSERT(0);
			break;
		}
	}
	return conversion;
}

// PipelineStateの使用履歴を更新する(キャッシュ制御に使用)
bool ContextMetal::updatePipelineStateUsedOrder(const PipelineState* pipelineState)
{
	AXGL_ASSERT(pipelineState != nullptr);
	bool same_as_last_used = true;
	// 使用履歴の新しい方から比較していく
	// 重複要素は存在しないことが分かっていることから、一致した時点で先頭に入れ替えて打ち切る
	// std::list の remove が全比較を行うことによる代替実装
	auto it = m_pipelineStateUsedOrder.begin();
	AXGL_ASSERT(it != m_pipelineStateUsedOrder.end());
	if (pipelineState != *it) { // 最新でない場合、入れ替えが必要
		for (; it != m_pipelineStateUsedOrder.end(); it++) {
			if (pipelineState == *it) {
				m_pipelineStateUsedOrder.erase(it);
				m_pipelineStateUsedOrder.emplace_front(pipelineState);
				break;
			}
		}
		same_as_last_used = false;
	}
	return same_as_last_used;
}

// DepthStencilStateの使用履歴を更新する(キャッシュ制御に使用)
bool ContextMetal::updateDepthStencilStateUsedOrder(const DepthStencilState* depthStencilState)
{
	AXGL_ASSERT(depthStencilState != nullptr);
	bool same_as_last_used = true;
	// 使用履歴の新しい方から比較していく
	// 重複要素は存在しないことが分かっていることから、一致した時点で先頭に入れ替えて打ち切る
	// std::list の remove が全比較を行うことによる代替実装
	auto it = m_depthStencilStateUsedOrder.begin();
	AXGL_ASSERT(it != m_depthStencilStateUsedOrder.end());
	if (depthStencilState != *it) { // 最新でない場合、入れ替えが必要
		for (; it != m_depthStencilStateUsedOrder.end(); it++) {
			if (depthStencilState == *it) {
				m_depthStencilStateUsedOrder.erase(it);
				m_depthStencilStateUsedOrder.emplace_front(depthStencilState);
				break;
			}
		}
		same_as_last_used = false;
	}
	return same_as_last_used;
}

} // namespace axgl

#endif // defined(__APPLE_CC__)
