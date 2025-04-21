#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "config.h"

#include <cstdlib>

#ifdef __cplusplus
extern "C" {
#endif

	typedef void* (*AllocCallback)(size_t size);
	typedef void* (*ReAllocCallback)(void* oldPtr, size_t newSize);
	typedef void(*FreeCallback)(void* ptr);

	HXSL_API void* HXSLAlloc(size_t size);

	HXSL_API void* HXSLReAlloc(void* oldPtr, size_t newSize);

	HXSL_API void HXSLFree(void* ptr);

	HXSL_API void HXSLSetAllocatorCallbacks(AllocCallback allocCallback, ReAllocCallback reallocCallback, FreeCallback freeCallback);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

template <typename T>
T* HXSLAlloc()
{
	return (T*)HXSLAlloc(sizeof(T));
}

template <typename T>
T* HXSLAllocArray(size_t count)
{
	return (T*)HXSLAlloc(sizeof(T) * count);
}

template <typename T>
T* HXSLReAllocArray(T* oldPtr, size_t newCount)
{
	return (T*)HXSLReAlloc(oldPtr, sizeof(T) * newCount);
}

#endif

#endif