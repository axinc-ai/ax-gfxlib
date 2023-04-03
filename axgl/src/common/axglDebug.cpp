// axglDebug.cpp
#include "axglDebug.h"
#include <stdio.h>
#include <stdarg.h>
#include "../backend/Backend.h"

#if defined(_DEBUG) || defined(DEBUG)

// debug output
void axglDebugOutput(const char* fmt, ...)
{
	char buf[2048];
	va_list args;
	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);
#if defined(_MSC_VER) || defined(__APPLE_CC__)
	axglBackendOutputMessage(buf);
#else
	// standard C common
	fprintf(stderr, "%s", buf);
#endif
	return;
}

#endif // defined(_DEBUG)

// output message
void axglMessageOutput(const char* fmt, ...)
{
	char buf[2048];
	va_list args;
	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);
#if defined(_MSC_VER) || defined(__APPLE_CC__)
	axglBackendOutputMessage(buf);
#else
	// standard C common
	fprintf(stdout, "%s", buf);
#endif
	return;
}
