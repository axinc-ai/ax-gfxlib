// DepthStencilState.cpp
#include "DepthStencilState.h"
#include "axglCommon.h"

#include <functional>

namespace axgl {

DepthStencilState::DepthStencilState()
{
}

DepthStencilState::~DepthStencilState()
{
}

// operator == for unordered_map
bool DepthStencilState::operator==(const DepthStencilState& rhs) const
{
	const DepthStencilState& lhs = *this;
	// depth test
	if (memcmp(&lhs.depthTestParams, &rhs.depthTestParams, sizeof(depthTestParams)) != 0) {
		return false;
	}
	// stencil test
	if (memcmp(&lhs.stencilTestParams, &rhs.stencilTestParams, sizeof(stencilTestParams)) != 0) {
		return false;
	}
	// write mask
	if (memcmp(&lhs.writemaskParams, &rhs.writemaskParams, sizeof(writemaskParams)) != 0) {
		return false;
	}
	return true;
}

size_t DepthStencilState::Hash::operator()(const DepthStencilState& state) const
{
	size_t hash_val = 0;
	combineHashFromArray(&hash_val, reinterpret_cast<const uint8_t*>(&state.depthTestParams), sizeof(state.depthTestParams));
	combineHashFromArray(&hash_val, reinterpret_cast<const uint8_t*>(&state.stencilTestParams), sizeof(state.stencilTestParams));
	combineHashFromArray(&hash_val, reinterpret_cast<const uint8_t*>(&state.writemaskParams), sizeof(state.writemaskParams));
	return hash_val;
}

} // namespace axgl
