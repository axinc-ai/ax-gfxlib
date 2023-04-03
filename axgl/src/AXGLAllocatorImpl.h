// AXGLAllocatorImpl.h
#ifndef __AXGLAllocatorImpl_h_
#define __AXGLAllocatorImpl_h_
// メモリアロケータの宣言

#include <AXGLAllocator.h>

#include <memory>
#include <new> // placement new

// メモリアロケータの取得
AXGLAllocator* axglGetAllocator();

//==============================================================================
// new/delete
void* operator new(std::size_t size, AXGLAllocator* allocator, const char* file, int line) noexcept;
void operator delete(void* p, AXGLAllocator* allocator, const char* file, int line) noexcept;

template<typename T>
void axglDelete(T* p, AXGLAllocator* allocator)
{
	if (p != nullptr) {
		p->~T();
		allocator->release(p);
	}
	return;
}

// 内部実装で使用するマクロの宣言
#define AXGL_NEW(type) (new(axglGetAllocator(),__FILE__,__LINE__) type)
#define AXGL_DELETE(p) axglDelete((p),axglGetAllocator())

//------------------------------------------------------------------------------
// new[]/delete[]
template<typename T>
T* axglNewArray(std::size_t n, AXGLAllocator* allocator, const char* file, int line)
{
	std::size_t size = sizeof(std::size_t) + sizeof(T) * n;
	void* p = allocator->alloc(size, file, line);
	if (p == nullptr) {
		return nullptr;
	}

	std::size_t* pSize = static_cast<std::size_t*>(p);
	*pSize = n;
	T* pObj = reinterpret_cast<T*>(pSize + 1);
	for (std::size_t i = 0; i < n; i++) {
		new((void*)(pObj + i)) T;
	}

	return pObj;
}

template<typename T>
void axglDeleteArray(T* p, AXGLAllocator* allocator)
{
	if(p != nullptr) {
		std::size_t* pSize = reinterpret_cast<std::size_t*>(p) - 1;
		for (T* pObj = p + (*pSize - 1); pObj >= p; pObj--) {
			pObj->~T();
		}
		allocator->release(pSize);
	}

	return;
}

// 内部実装で使用するマクロの宣言
#define AXGL_NEW_ARRAY(type,n) axglNewArray<type>((n),axglGetAllocator(),__FILE__,__LINE__)
#define AXGL_DELETE_ARRAY(p) axglDeleteArray((p),axglGetAllocator())

//----------------------------------------------------------------------------
// malloc/free に相当するマクロの宣言
#define AXGL_ALLOC(s) (axglGetAllocator()->alloc((s),__FILE__,__LINE__))
#define AXGL_FREE(p) (axglGetAllocator()->release(p))

//----------------------------------------------------------------------------
// C++11 STLアロケータの実装
template <typename T>
class AXGLStlAllocator
{
public:
	typedef T value_type;

	// default constructor
	AXGLStlAllocator()
	{
	}
	// copy constructor
	AXGLStlAllocator(const AXGLStlAllocator&)
	{
	}
	// move constructor
	AXGLStlAllocator(AXGLStlAllocator&&)
	{
	}
	// constructor from another template argument
	template <typename U> AXGLStlAllocator(const AXGLStlAllocator<U>&)
	{
	}
	// allocate memory
	T* allocate(std::size_t n)
	{
		return static_cast<T*>(axglGetAllocator()->alloc((sizeof(T) * n), __FILE__, __LINE__));
	}
	// deallocate memory
	void deallocate(T* p, std::size_t n)
	{
		(void)n;
		axglGetAllocator()->release(p);
	}
};

template <typename T1, typename T2>
bool operator==(const AXGLStlAllocator<T1>&, const AXGLStlAllocator<T2>&)
{
	return true;
}

template <typename T1, typename T2>
bool operator!=(const AXGLStlAllocator<T1>&, const AXGLStlAllocator<T2>&)
{
	return false;
}

#endif // __AXGLAllocatorImpl_h_
