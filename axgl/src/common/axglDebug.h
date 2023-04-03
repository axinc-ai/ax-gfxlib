/// axglDebug.h
#ifndef __axglDebug_h_
#define __axglDebug_h_

// デバッグ用のアサートマクロ定義
#if defined(_MSC_VER) && defined(_DEBUG)
#include <Windows.h>
#define AXGL_ASSERT(a) {if(!(a)){DebugBreak();}}
#elif defined(DEBUG)
#include <assert.h>
#define AXGL_ASSERT assert
#else
#define AXGL_ASSERT AXGL_UNUSED
#endif

// デバッグ出力マクロ定義
#if defined(_DEBUG) || defined(DEBUG)
void axglDebugOutput(const char* fmt, ...);
#define AXGL_DBGOUT axglDebugOutput
#else
#define AXGL_DBGOUT(...)
#endif

void axglMessageOutput(const char* fmt, ...);
#define AXGL_MSGOUT axglMessageOutput

#endif // __axglDebug_h_
