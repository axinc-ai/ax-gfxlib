// AXGLAllocatorImpl.cpp
// メモリアロケータの実装

#include "AXGLAllocatorImpl.h"
#include "common/axglCommon.h"

#if defined(_MSC_VER) && defined(_DEBUG)
// Use debug C runtime of Visual Studio
#define USE_VS_CRTDBG 1
#define USE_DEBUG_ALLOCATOR 1
#endif

#include <stdio.h>
#include <stdint.h>
#ifdef USE_VS_CRTDBG
#include <crtdbg.h>
#endif

#if defined(USE_DEBUG_ALLOCATOR) 
// デバッグ用のメモリ情報構造体
struct AXGLMemInfo
{
	const char* file; // file name (__FILE__)
	int          line; // line number (__LINE__)
	std::size_t  size; // memory size
	uint32_t     id;   // ID
	AXGLMemInfo* prev; // pointer to previous
	AXGLMemInfo* next; // pointer to next
};

// リスト
static AXGLMemInfo s_memInfoList =
{
	nullptr, // file
	0,       // line
	0,       // size
	0,       // id
	nullptr, // prev
	nullptr  // next
};

// 現在のアロケーションサイズ
static std::size_t s_currentAllocSize = 0;
// 最大アロケーションサイズ
static std::size_t s_maxAllocSize = 0;
// アロケーション数
static uint32_t s_allocCount = 0;
#endif // defined(USE_DEBUG_ALLOCATOR)

static AXGLAllocator s_defaultAllocator;
static AXGLAllocator* s_pAllocator = &s_defaultAllocator;

//-------------------------------------------------------------------
// メモリアロケータの設定
void AXGL_APIENTRY axglSetAllocator(AXGLAllocator* alloc)
{
	if (alloc == nullptr) {
		s_pAllocator = &s_defaultAllocator;
	} else {
		s_pAllocator = alloc;
	}

	return;
}

// メモリ使用情報をダンプ
void AXGL_APIENTRY axglDumpMemUsage(void(printFunc)(const char*))
{
	AXGLAllocator* alloc = axglGetAllocator();
	if (alloc != nullptr) {
		alloc->dumpMemUsage(printFunc);
	}
	return;
}

// メモリアロケータを取得
AXGLAllocator* axglGetAllocator()
{
	return s_pAllocator;
}

//-------------------------------------------------------------------
// operator new
void* operator new(std::size_t size, AXGLAllocator* allocator, const char* file, int line) noexcept
{
	AXGL_ASSERT(allocator != nullptr);
	return allocator->alloc(size, file, line);
}

// operator delete
void operator delete(void* p, AXGLAllocator* allocator, const char* file, int line) noexcept
{
	AXGL_UNUSED(file);
	AXGL_UNUSED(line);
	AXGL_ASSERT(allocator != nullptr);
	allocator->release(p);
	return;
}

//-------------------------------------------------------------------
// constructor
AXGLAllocator::AXGLAllocator()
{
}

// destructor
AXGLAllocator::~AXGLAllocator()
{
}

// allocate
void* AXGLAllocator::alloc(std::size_t size, const char* file, int line)
{
#if defined(USE_DEBUG_ALLOCATOR)
	size += sizeof(AXGLMemInfo);
#endif

	void* p = allocMem(size, file, line);

#if defined(USE_DEBUG_ALLOCATOR)
	if (p != nullptr) {
		// NOTE: スレッドセーフにするには、排他制御が必要
		AXGLMemInfo* pInfo = reinterpret_cast<AXGLMemInfo*>(p);
		p = (pInfo + 1);

		pInfo->file = file;
		pInfo->line = line;
		pInfo->size = size - sizeof(AXGLMemInfo);
		pInfo->id = s_allocCount;

		pInfo->next = s_memInfoList.next;
		if (pInfo->next != nullptr) {
			pInfo->next->prev = pInfo;
		}
		s_memInfoList.next = pInfo;
		pInfo->prev = &s_memInfoList;

		s_currentAllocSize += size;
		if (s_currentAllocSize > s_maxAllocSize) {
			s_maxAllocSize = s_currentAllocSize;
		}
	}
	s_allocCount++;
#endif // defined(USE_DEBUG_ALLOCATOR)

	return p;
}

// release
void AXGLAllocator::release(void* p)
{
#if defined(USE_DEBUG_ALLOCATOR)
	if (p != nullptr) {
		// NOTE: スレッドセーフにするには、排他制御が必要
		AXGLMemInfo* pInfo = reinterpret_cast<AXGLMemInfo*>(p) - 1;

		pInfo->prev->next = pInfo->next;
		if (pInfo->next != nullptr) {
			pInfo->next->prev = pInfo->prev;
		}

		s_currentAllocSize -= pInfo->size;

		p = pInfo;
	}
#endif // defined(USE_DEBUG_ALLOCATOR)

	freeMem(p);

	return;
}

#if defined(USE_DEBUG_ALLOCATOR) 
// output information function
static void output_info(void(*printFunc)(const char*), const char* info)
{
	if (printFunc != nullptr) {
		printFunc(info);
	} else {
		AXGL_DBGOUT(info);
	}
	return;
}
#endif // defined(USE_DEBUG_ALLOCATOR) 

// dump memory usage
void AXGLAllocator::dumpMemUsage(void(*printFunc)(const char*))
{
#if defined(USE_DEBUG_ALLOCATOR) 
	char buf[256];
	output_info(printFunc, "-----------------------------------\n");
	output_info(printFunc, "AXGL:INFO:Dumping memory usage info.\n");

	snprintf(buf, 256,
		"AXGL:INFO:Max memory allocated size. %zubyte\n", s_maxAllocSize);
	output_info(printFunc, buf);

	AXGLMemInfo* info = s_memInfoList.next;
	if (info != nullptr) {
		while (info != nullptr) {
			snprintf(buf, 256,
				"AXGL:%s(%d){%d} %zu byte\n",
				info->file, info->line, info->id, info->size);
			output_info(printFunc, buf);
			info = info->next;
		}
	} else {
		output_info(printFunc, "AXGL:INFO:Detected no memory leak.\n");
	}
	output_info(printFunc, "-----------------------------------\n");
#else
	AXGL_UNUSED(printFunc);
#endif // defined(USE_DEBUG_ALLOCATOR) 

	return;
}

// allocate memory
void* AXGLAllocator::allocMem(std::size_t size, const char* file, int line)
{
#ifdef USE_VS_CRTDBG
	return _malloc_dbg(size, _NORMAL_BLOCK, file, line);
#else
	AXGL_UNUSED(file);
	AXGL_UNUSED(line);
	return malloc(size);
#endif
}

// free memory
void AXGLAllocator::freeMem(void* p)
{
#ifdef USE_VS_CRTDBG
	_free_dbg(p, _NORMAL_BLOCK);
#else
	free(p);
#endif
	return;
}
