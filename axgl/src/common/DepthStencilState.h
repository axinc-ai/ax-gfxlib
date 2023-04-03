// DepthStencilState.h
#ifndef __DepthStencilState_h_ 
#define __DepthStencilState_h_

#include "../common/axglCommon.h"

namespace axgl {

#pragma pack(push, 1)
// 深度テストパラメータ
struct DepthTestParams
{
	// GL_DEPTH_TEST
	GLboolean depthTestEnable = GL_FALSE;
	// GL_DEPTH_FUNC
	GLenum depthFunc = GL_LESS;
};

// ステンシルテストパラメータ
struct StencilTestParams
{
	// GL_STENCIL_TEST
	GLboolean stencilTestEnable = GL_FALSE;
	// GL_STENCIL_FUNC
	GLenum stencilFunc = GL_ALWAYS;
	// GL_STENCIL_VALUE_MASK
	GLuint stencilValueMask = ~0U;
	// GL_STENCIL_BACK_FUNC
	GLenum stencilBackFunc = GL_ALWAYS;
	// GL_STENCIL_BACK_VALUE_MASK
	GLuint stencilBackValueMask = ~0U;
	// GL_STENCIL_FAIL
	GLenum stencilFail = GL_KEEP;
	// GL_STENCIL_PASS_DEPTH_FAIL
	GLenum stencilPassDepthFail = GL_KEEP;
	// GL_STENCIL_PASS_DEPTH_PASS
	GLenum stencilPassDepthPass = GL_KEEP;
	// GL_STENCIL_BACK_FAIL
	GLenum stencilBackFail = GL_KEEP;
	// GL_STENCIL_BACK_PASS_DEPTH_FAIL
	GLenum stencilBackPassDepthFail = GL_KEEP;
	// GL_STENCIL_BACK_PASS_DEPTH_PASS
	GLenum stencilBackPassDepthPass = GL_KEEP;
};

// 深度/ステンシル書き込みマスクパラメータ
struct DepthStencilWritemaskParams
{
	// GL_DEPTH_WRITEMASK
	GLboolean depthWritemask = GL_TRUE;
	// GL_STENCIL_WRITEMASK
	GLuint stencilWritemask = ~0U;
	// GL_STENCIL_BACK_WRITEMASK
	GLuint stencilBackWritemask = ~0U;
};
#pragma pack(pop)

// 深度/ステンシルステート
class DepthStencilState
{
public:
	DepthStencilState();
	~DepthStencilState();

	// operator == for unordered_map
	bool operator==(const DepthStencilState& rhs) const;

	// hash function struct for unordered_map
	struct Hash
	{
		size_t operator()(const DepthStencilState& state) const;
	};

public:
	DepthTestParams depthTestParams;
	StencilTestParams stencilTestParams;
	DepthStencilWritemaskParams writemaskParams;
};

} // namespace axgl

#endif // __DepthStencilState_h_
