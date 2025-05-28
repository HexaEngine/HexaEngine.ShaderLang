#include "c/hxsl_compiler.h"
#include "hxls_compiler.hpp"

#if HXSL_ENABLE_CAPI

HXSL_API HXSLCompiler* HXSL_CreateCompiler()
{
	HXSL::Compiler* compiler = new HXSL::Compiler();
	return reinterpret_cast<HXSLCompiler*>(compiler);
}

HXSL_API void HXSL_CompilerRelease(HXSLCompiler* self)
{
	auto compiler = reinterpret_cast<HXSL::Compiler*>(self);
	delete compiler;
}

HXSL_API void HXSL_CompilerSetIncludeHandler(HXSLCompiler* self, IncludeOpen includeOpen, IncludeClose includeClose)
{
	auto compiler = reinterpret_cast<HXSL::Compiler*>(self);
	compiler->SetIncludeHandler(includeOpen, includeClose);
}

HXSL_API HXSLCompilationResult* HXSL_CompilerCompile(HXSLCompiler* self, Blob* blob)
{
	return nullptr;
}

#endif