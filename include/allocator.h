#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "config.h"

C_API_BEGIN

typedef void* (*AllocCallback)(size_t size);
typedef void* (*ReAllocCallback)(void* oldPtr, size_t newSize);
typedef void(*FreeCallback)(void* ptr);

HXSL_API void* Alloc(size_t size);

HXSL_API void* ReAlloc(void* oldPtr, size_t newSize);

HXSL_API void Free(void* ptr);

HXSL_API void SetAllocatorCallbacks(AllocCallback allocCallback, ReAllocCallback reallocCallback, FreeCallback freeCallback);

HXSL_API void GetAllocatorCallbacks(AllocCallback* allocCallback, ReAllocCallback* reallocCallback, FreeCallback* freeCallback);

C_API_END

#ifdef __cplusplus

template <typename T>
T* Alloc()
{
	return (T*)Alloc(sizeof(T));
}

template <typename T>
T* AllocArray(size_t count)
{
	return (T*)Alloc(sizeof(T) * count);
}

template <typename T>
T* ReAllocArray(T* oldPtr, size_t newCount)
{
	return (T*)ReAlloc(oldPtr, sizeof(T) * newCount);
}

#endif

#endif