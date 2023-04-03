// axglCommon.cpp
#include "axglCommon.h"
#include "../core/CoreContext.h"
#include "../backend/Backend.h"
#include <functional>

namespace axgl {

void combineHash(size_t* h, size_t v)
{
	AXGL_ASSERT(h != nullptr);
	size_t tmp = v + 0x9e3779b9 + ((*h) << 6) + ((*h) >> 2);
	*h ^= tmp;
	return;
}

void combineHashFromArray(size_t* h, const uint8_t* p, size_t size)
{
	AXGL_ASSERT((h != nullptr) && (p != nullptr));
	for (size_t i = 0; i < size; i++) {
		combineHash(h, std::hash<uint8_t>()(*p));
		p++;
	}
	return;
}

void setErrorCode(GLenum code)
{
	CoreContext* p_ctx = backendGetCurrentContext();
	if (p_ctx != nullptr) {
		p_ctx->setErrorCode(code);
	}
}

} // namespace axgl
