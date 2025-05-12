#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "config.h"

C_API_BEGIN

typedef void* (*AllocCallback)(size_t size);
typedef void* (*ReAllocCallback)(void* oldPtr, size_t newSize);
typedef void(*FreeCallback)(void* ptr);

HXSL_API void* HXSL_Alloc(size_t size);

HXSL_API void* HXSL_ReAlloc(void* oldPtr, size_t newSize);

HXSL_API void HXSL_Free(void* ptr);

HXSL_API void HXSL_SetAllocatorCallbacks(AllocCallback allocCallback, ReAllocCallback reallocCallback, FreeCallback freeCallback);

HXSL_API void HXSL_GetAllocatorCallbacks(AllocCallback* allocCallback, ReAllocCallback* reallocCallback, FreeCallback* freeCallback);

C_API_END

#ifdef __cplusplus

template <typename T>
T* HXSL_Alloc()
{
	return (T*)HXSL_Alloc(sizeof(T));
}

#endif

#endif