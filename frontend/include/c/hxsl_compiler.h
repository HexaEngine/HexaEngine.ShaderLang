#ifndef HXSL_COMPILER_H
#define HXSL_COMPILER_H

#include "common.h"
#include "blob.h"

typedef void(*IncludeOpen)(const char* pFile, void** dataOut, size_t* sizeOut, void* userdata);
typedef void(*IncludeClose)(const char* pFile, void* data, void* userdata);

#if HXSL_ENABLE_CAPI
C_API_BEGIN
typedef struct HXSLCompiler HXSLCompiler;
typedef struct HXSLCompilationResult HXSLCompilationResult;

HXSL_API HXSLCompiler* HXSL_CreateCompiler();

HXSL_API void HXSL_CompilerRelease(HXSLCompiler* self);

HXSL_API void HXSL_CompilerSetIncludeHandler(HXSLCompiler* self, IncludeOpen includeOpen, IncludeClose includeClose);

HXSL_API HXSLCompilationResult* HXSL_CompilerCompile(HXSLCompiler* self, Blob* blob);

C_API_END
#endif

#endif