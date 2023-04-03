// axglCommon.h
#ifndef __axglCommon_h_
#define __axglCommon_h_

#define USE_GL_MANGLE 1
#include <axgl/ES3/gl.h>
#include <axgl/ES3/glext.h>
#include <stdint.h>
#include <stddef.h>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <list>

#include "axglDebug.h"
#include "../AXGLAllocatorImpl.h"

#define AXGL_UNUSED(a) ((void)(a))

#define AXGL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 32
#define AXGL_MAX_VERTEX_ATTRIBS 16
#define AXGL_MAX_COLOR_ATTACHMENTS 4
#define AXGL_MAX_DRAW_BUFFERS 8
#define AXGL_MAX_UNIFORM_BUFFER_BINDINGS 24
#define AXGL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS 16

#define AXGL_DEFAULT_FRAMEBUFFER_WIDTH 640
#define AXGL_DEFAULT_FRAMEBUFFER_HEIGHT 480

namespace axgl {

void combineHash(size_t* h, size_t v);
void combineHashFromArray(size_t* h, const uint8_t* p, size_t size);
void setErrorCode(GLenum code);

// STLアロケータを指定したvector
template<class T>
using AXGLVector = std::vector<T, AXGLStlAllocator<T>>;

// STLアロケータを指定したunordered_map
template<class T1, class T2>
using AXGLUnorderedMap = std::unordered_map<T1, T2, std::hash<T1>, std::equal_to<T1>, AXGLStlAllocator<std::pair<const T1, T2>>>;

// STLアロケータを指定したunordered_set
template<class T>
using AXGLUnorderedSet = std::unordered_set<T, std::hash<T>, std::equal_to<T>, AXGLStlAllocator<T>>;

template<class T>
using AXGLList = std::list<T, AXGLStlAllocator<T>>;


} // namespace axgl

#endif // __axglCommon_h_
