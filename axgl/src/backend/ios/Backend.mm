// Backend.mm
#if defined(__APPLE_CC__)

#include <Foundation/Foundation.h>
#include <TargetConditionals.h>

#if TARGET_OS_IPHONE
#include "../Backend.h"

namespace axgl {

// C++11 thread local storage
thread_local CoreContext* tls_currentContext = nullptr;

void backendSetCurrentContext(CoreContext* context)
{
	tls_currentContext = context;
}

CoreContext* backendGetCurrentContext()
{
	return tls_currentContext;
}

} // namespace axgl

// C function interface
void axglBackendOutputMessage(const char* str)
{
	NSLog(@"%s", str);
	return;
}

#endif // TARGET_OS_IPHONE

#endif // defined(__APPLE_CC__)
