// AXGLAllocator.h
#ifndef __AXGLAllocator_h_
#define __AXGLAllocator_h_

#include <cstddef> // std::size_t

#define AXGL_APIENTRY
#define AXGL_API extern

// memory allocator class
class AXGLAllocator
{
public:
	// constructor
	AXGLAllocator();
	// destructor
	virtual ~AXGLAllocator();
	// memory allocate
	void *alloc(std::size_t size, const char* file, int line);
	// memory release
	void release(void *p);
	// dump memory usage
	void dumpMemUsage(void(*printFunc)(const char *));

protected:
	// allocate memory
	virtual void *allocMem(std::size_t size, const char* file, int line);
	// free memory
	virtual void freeMem(void *p);
};

AXGL_API void AXGL_APIENTRY axglSetAllocator(AXGLAllocator* alloc);

AXGL_API void AXGL_APIENTRY axglDumpMemUsage(void(printFunc)(const char*));

#endif // __AXGLAllocator_h_
