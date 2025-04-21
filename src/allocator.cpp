#include "allocator.h"

static AllocCallback _allocCallback = nullptr;
static ReAllocCallback _reallocCallback = nullptr;
static FreeCallback _freeCallback = nullptr;

void *HXSLAlloc(size_t size)
{
    if (_allocCallback)
    {
        return _allocCallback(size);
    }
    return malloc(size);
}

HXSL_API void *HXSLReAlloc(void *oldPtr, size_t newSize)
{
    if (_reallocCallback)
    {
        return _reallocCallback(oldPtr, newSize);
    }
    return realloc(oldPtr, newSize);
}

void HXSLFree(void *ptr)
{
    if (_freeCallback)
    {
        _freeCallback(ptr);
        return;
    }
    free(ptr);
}

void HXSLSetAllocatorCallbacks(AllocCallback allocCallback, ReAllocCallback reallocCallback, FreeCallback freeCallback)
{
    _allocCallback = allocCallback;
    _reallocCallback = reallocCallback;
    _freeCallback = freeCallback;
}
