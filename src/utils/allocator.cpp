#include "allocator.h"

static AllocCallback _allocCallback = nullptr;
static ReAllocCallback _reallocCallback = nullptr;
static FreeCallback _freeCallback = nullptr;

void* Alloc(size_t size)
{
	if (_allocCallback)
	{
		return _allocCallback(size);
	}
	return malloc(size);
}

void* ReAlloc(void* oldPtr, size_t newSize)
{
	if (_reallocCallback)
	{
		return _reallocCallback(oldPtr, newSize);
	}
	return realloc(oldPtr, newSize);
}

void Free(void* ptr)
{
	if (_freeCallback)
	{
		_freeCallback(ptr);
		return;
	}
	free(ptr);
}

void SetAllocatorCallbacks(AllocCallback allocCallback, ReAllocCallback reallocCallback, FreeCallback freeCallback)
{
	_allocCallback = allocCallback;
	_reallocCallback = reallocCallback;
	_freeCallback = freeCallback;
}