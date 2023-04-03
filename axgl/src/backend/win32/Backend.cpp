#if defined(_MSC_VER)

#include "../Backend.h"
#include <windows.h>

namespace axgl {

// Current context : Windows TLS
__declspec(thread) CoreContext* tls_currentContext = nullptr;

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
	OutputDebugStringA(str);
	return;
}

#endif // defined(_MSC_VER)
