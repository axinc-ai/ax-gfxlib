// TestInterface.h
#ifndef TestInterface_h
#define TestInterface_h

#ifdef __cplusplus
extern "C" {
#endif

// Initialization
void* exampleInitialize(void);
// Termination
void exampleTerminate(void* instance);
// Create GLES resources
void exampleCreateResources(void* instance);
// Destroy GLES resources
void exampleDestroyResources(void* instance);
// Render the scene
void exampleRender(void* instance, uint32_t framebuffer);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // TestInterface_h
