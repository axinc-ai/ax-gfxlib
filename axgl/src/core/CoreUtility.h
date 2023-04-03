// CoreUtility.h
#ifndef __CoreUtility_h_
#define __CoreUtility_h_

#include "../common/axglCommon.h"

namespace axgl {

int getAlphaBitsFromFormat(GLenum format);
int getRedBitsFromFormat(GLenum format);
int getGreenBitsFromFormat(GLenum format);
int getBlueBitsFromFormat(GLenum format);
int getDepthBitsFromFormat(GLenum format);
int getStencilBitsFromFormat(GLenum format);
GLenum getComponentTypeFromFormat(GLenum format, bool isStencil = false);
GLenum getColorEncodingFromFormat(GLenum format);
GLboolean isIntegerFromType(GLenum type);

} // namespace

#endif // __CoreUtility_h_
