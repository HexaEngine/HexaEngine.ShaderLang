#include "allocator.h"
#include <cstdlib>

static AllocCallback _allocCallback = nullptr;
static ReAllocCallback _reallocCallback = nullptr;
static FreeCallback _freeCallback = nullptr;

HXSL_API void* Alloc(size_t size)
{
	if (_allocCallback)
	{
		return _allocCallback(size);
	}
	return malloc(size);
}

HXSL_API void* ReAlloc(void* oldPtr, size_t newSize)
{
	if (_reallocCallback)
	{
		return _reallocCallback(oldPtr, newSize);
	}
	return realloc(oldPtr, newSize);
}

HXSL_API void Free(void* ptr)
{
	if (_freeCallback)
	{
		_freeCallback(ptr);
		return;
	}
	free(ptr);
}

HXSL_API void SetAllocatorCallbacks(AllocCallback allocCallback, ReAllocCallback reallocCallback, FreeCallback freeCallback)
{
	_allocCallback = allocCallback;
	_reallocCallback = reallocCallback;
	_freeCallback = freeCallback;
}

HXSL_API void GetAllocatorCallbacks(AllocCallback* allocCallback, ReAllocCallback* reallocCallback, FreeCallback* freeCallback)
{
	*allocCallback = _allocCallback;
	*reallocCallback = _reallocCallback;
	*freeCallback = _freeCallback;
}