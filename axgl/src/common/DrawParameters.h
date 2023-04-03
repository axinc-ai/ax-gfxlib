// DrawParameters.h
#ifndef __DrawParameters_h_
#define __DrawParameters_h_

#include "../common/axglCommon.h"
#include "PipelineState.h"
#include "DepthStencilState.h"

namespace axgl {

class CoreProgram;
class CoreQuery;
class CoreTexture;
class CoreSampler;
class CoreBuffer;
class CoreFramebuffer;
class CoreVertexArray;

// カリングパラメータ
struct CullFaceParams
{
	// GL_CULL_FACE
	GLboolean cullFaceEnable = GL_FALSE;
	// GL_CULL_FACE_MODE
	GLenum cullFaceMode = GL_BACK;
	// GL_FRONT_FACE
	GLenum frontFace = GL_CCW;
};

// ポリゴンオフセットパラメータ
struct PolygonOffsetParams
{
	// GL_POLYGON_OFFSET_FILL
	GLboolean polygonOffsetFillEnable = GL_FALSE;
	// GL_POLYGON_OFFSET_FACTOR
	float polygonOffsetFactor = 0.0f;
	// GL_POLYGON_OFFSET_UNITS
	float polygonOffsetUnits = 0.0f;
};

// ビューポートパラメータ
struct ViewportParams
{
	// GL_VIEWPORT
	GLint viewport[4] = {0,0,AXGL_DEFAULT_FRAMEBUFFER_WIDTH,AXGL_DEFAULT_FRAMEBUFFER_HEIGHT};
	// GL_DEPTH_RANGE
	float depthRange[2] = {0.0f,1.0f};
};

// Scissorパラメータ
struct ScissorParams
{
	// GL_SCISSOR_TEST
	GLboolean scissorTestEnable = GL_FALSE;
	// GL_SCISSOR_BOX
	GLint scissorBox[4] = {0,0,AXGL_DEFAULT_FRAMEBUFFER_WIDTH,AXGL_DEFAULT_FRAMEBUFFER_HEIGHT};
};

// インデックスパラメータ
struct IndexedBuffer
{
	axgl::CoreBuffer* buffer = nullptr;
	GLintptr offset = 0;
	GLsizeiptr size = 0;
};

// ステンシル参照パラメータ
struct StencilReference
{
	// GL_STENCIL_REF
	GLint stencilRef = 0;
	// GL_STENCIL_BACK_REF
	GLint stencilBackRef = 0;
};

// ダーティビット
enum {
	STENCIL_REFERENCE_DIRTY_BIT        = 0x00000001,
	VIEWPORT_DIRTY_BIT                 = 0x00000002,
	CULL_FACE_DIRTY_BIT                = 0x00000004,
	SCISSOR_DIRTY_BIT                  = 0x00000008,
	POLYGON_OFFSET_DIRTY_BIT           = 0x00000010,
	TEXTURE_BINDING_DIRTY_BIT          = 0x00000020,
	SAMPLER_BINDING_DIRTY_BIT          = 0x00000040,
	CURRENT_PROGRAM_DIRTY_BIT          = 0x00000080,
	VERTEX_ARRAY_BINDING_DIRTY_BIT     = 0x00000100,
	UNIFORM_BUFFER_BINDING_DIRTY_BIT   = 0x00000200,
	ANY_SAMPLES_PASSED_QUERY_DIRTY_BIT = 0x00000400,
	DIRTY_FLAGS_ALL                    = 0x000007ff
};

// 描画パラメータ
struct DrawParameters {
	// for setRenderPipelineState (MTLRenderPipelineState)
	PipelineState renderPipelineState;
	// for setDepthStencilState (MTLDepthStencilState)
	DepthStencilState depthStencilState;
	// for FrontFaceWinding, CullMode
	CullFaceParams cullFaceParams;
	// for setDepthBias, setDepthClipMode:ClipOnly
	PolygonOffsetParams polygonOffsetParams;
	// for setViewport
	ViewportParams viewportParams;
	// for setScissorRect
	ScissorParams scissorParams;
	// for setBlendColor
	float blendColor[4] = {0.0f,0.0f,0.0f,0.0f};
	// for setVisibilityResultMode (from GL query object)
	CoreQuery* anySamplesPassedQuery = nullptr;
	CoreQuery* anySamplesPassedConservativeQuery = nullptr;
	// program
	CoreProgram* program = nullptr;
	// uniform buffers
	IndexedBuffer uniformBuffer[AXGL_MAX_UNIFORM_BUFFER_BINDINGS];
	// textures
	CoreTexture* texture2d[AXGL_MAX_COMBINED_TEXTURE_IMAGE_UNITS] = {0};
	CoreTexture* textureCube[AXGL_MAX_COMBINED_TEXTURE_IMAGE_UNITS] = {0};
	CoreTexture* texture3d[AXGL_MAX_COMBINED_TEXTURE_IMAGE_UNITS] = {0};
	CoreTexture* texture2dArray[AXGL_MAX_COMBINED_TEXTURE_IMAGE_UNITS] = {0};
	// samplers
	CoreSampler* samplers[AXGL_MAX_COMBINED_TEXTURE_IMAGE_UNITS] = {0};
	// vertex buffer
	CoreBuffer* vertexBuffer[AXGL_MAX_VERTEX_ATTRIBS] = {0};
	// framebuffer
	CoreFramebuffer* framebufferDraw = nullptr;
	// index buffer
	CoreBuffer* indexBuffer = nullptr;
	// vertex array
	CoreVertexArray* vertexArray = nullptr;
	// stencil reference value
	StencilReference stencilReference;
	// dirty flags
	uint32_t dirtyFlags = DIRTY_FLAGS_ALL;
};

} // namespace axgl

#endif // __DrawParameters_h_
