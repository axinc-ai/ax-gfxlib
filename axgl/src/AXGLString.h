// AXGLString.h
#ifndef __AXGLString_h_
#define __AXGLString_h_

#include "AXGLAllocatorImpl.h"
#include <string>

// std::stringのアロケータを置き換え
typedef std::basic_string<char, std::char_traits<char>, AXGLStlAllocator<char>> AXGLString;

#if defined(__GLIBCXX__)
// libstdc++ の std::hash 対策
namespace std {
template<>
struct hash<AXGLString>
{
	typedef AXGLString argument_type;
	typedef size_t result_type;

	result_type operator () (const argument_type& x) const
	{
		size_t val = 0;
		for (auto it = x.begin(); it != x.end(); ++it) {
			val ^= std::hash<char>()(*it) + 0x9e3779b9 + (val << 6) + (val >> 2);
		}
		return val;
	}
};
} // namespace std
#endif // defined(__GLIBCXX__)

#endif // __AXGLString_h_
