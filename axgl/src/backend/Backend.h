#ifndef __Backend_h_
#define __Backend_h_

namespace axgl {

class CoreContext;

void backendSetCurrentContext(CoreContext* context);
CoreContext* backendGetCurrentContext();

} // namespace axgl

// C function interface
void axglBackendOutputMessage(const char* str);

#endif // __Backend_h_
