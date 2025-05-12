#include "allocator.h"
#include <cstdlib>

static AllocCallback _allocCallback = nullptr;
static ReAllocCallback _reallocCallback = nullptr;
static FreeCallback _freeCallback = nullptr;

HXSL_API void* HXSL_Alloc(size_t size)
{
	if (_allocCallback)
	{
		return _allocCallback(size);
	}
	return malloc(size);
}

HXSL_API void* HXSL_ReAlloc(void* oldPtr, size_t newSize)
{
	if (_reallocCallback)
	{
		return _reallocCallback(oldPtr, newSize);
	}
	return realloc(oldPtr, newSize);
}

HXSL_API void HXSL_Free(void* ptr)
{
	if (_freeCallback)
	{
		_freeCallback(ptr);
		return;
	}
	free(ptr);
}

HXSL_API void HXSL_SetAllocatorCallbacks(AllocCallback allocCallback, ReAllocCallback reallocCallback, FreeCallback freeCallback)
{
	_allocCallback = allocCallback;
	_reallocCallback = reallocCallback;
	_freeCallback = freeCallback;
}

HXSL_API void HXSL_GetAllocatorCallbacks(AllocCallback* allocCallback, ReAllocCallback* reallocCallback, FreeCallback* freeCallback)
{
	*allocCallback = _allocCallback;
	*reallocCallback = _reallocCallback;
	*freeCallback = _freeCallback;
}