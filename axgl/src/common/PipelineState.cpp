// PipelineState.cpp
#include "PipelineState.h"
#include "axglCommon.h"

#include <functional>

namespace axgl {

PipelineState::PipelineState()
{
}

PipelineState::~PipelineState()
{
}

// operator == for unordered_map
bool PipelineState::operator==(const PipelineState& rhs) const
{
	const PipelineState& lhs = *this;
	// program
	if (lhs.program != rhs.program) {
		return false;
	}
	// vertex attributes
	if (memcmp(lhs.vertexAttribs, rhs.vertexAttribs, sizeof(vertexAttribs)) != 0) {
		return false;
	}
	// target attachment
	if (memcmp(&lhs.targetAttachment, &rhs.targetAttachment, sizeof(targetAttachment)) != 0) {
		return false;
	}
	// blend
	if (memcmp(&lhs.blendParams, &rhs.blendParams, sizeof(blendParams)) != 0) {
		return false;
	}
	// write mask
	if (memcmp(&lhs.writemaskParams, &rhs.writemaskParams, sizeof(writemaskParams)) != 0) {
		return false;
	}
	// sample coverage
	if (memcmp(&lhs.sampleCoverageParams, &rhs.sampleCoverageParams, sizeof(sampleCoverageParams)) != 0) {
		return false;
	}
	// vertex array object
	if (lhs.vertexArray != rhs.vertexArray) {
		return false;
	}
	// instanced draw
	if (lhs.isInstanced != rhs.isInstanced) {
		return false;
	}
	return true;
}

size_t PipelineState::Hash::operator()(const PipelineState& state) const
{
	size_t hash_val = 0;
	combineHash(&hash_val, std::hash<void*>()((void*)(state.program)));
	combineHashFromArray(&hash_val, reinterpret_cast<const uint8_t*>(state.vertexAttribs), sizeof(state.vertexAttribs));
	combineHashFromArray(&hash_val, reinterpret_cast<const uint8_t*>(&state.targetAttachment), sizeof(state.targetAttachment));
	combineHashFromArray(&hash_val, reinterpret_cast<const uint8_t*>(&state.blendParams), sizeof(state.blendParams));
	combineHashFromArray(&hash_val, reinterpret_cast<const uint8_t*>(&state.writemaskParams), sizeof(state.writemaskParams));
	combineHashFromArray(&hash_val, reinterpret_cast<const uint8_t*>(&state.sampleCoverageParams), sizeof(state.sampleCoverageParams));
	combineHash(&hash_val, std::hash<void*>()((void*)(state.vertexArray)));
	combineHash(&hash_val, std::hash<bool>()(state.isInstanced));
	return hash_val;
}

} // namespace axgl
