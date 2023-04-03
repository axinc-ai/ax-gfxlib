// ClearParameters.h
#ifndef __ClearParameters_h_
#define __ClearParameters_h_

#include "../common/axglCommon.h"

namespace axgl {

// クリアパラメータ
struct ClearParameters {
	float colorClearValue[AXGL_MAX_COLOR_ATTACHMENTS][4] = {{0.0f,0.0f,0.0f,0.0f}};
	float depthClearValue = 1.0f;
	uint32_t stencilClearValue = 0;
};

} // namespace axgl

#endif // __ClearParameters_h_
