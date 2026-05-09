#pragma once

#include "pch/std.hpp"
#include "semantics/assembly_resolver.hpp"

namespace HXSL
{
	class CompilerContext
	{
		AssemblyResolver resolver;

		static CompilerContext*& GetCurrentRef()
		{
			static thread_local CompilerContext* ctx = nullptr;
			return ctx;
		}

		CompilerContext() {}
	public:
		static [[nodiscard]] uptr<CompilerContext> Create()
		{
			uptr<CompilerContext> ctx(new CompilerContext());
			if (!GetCurrent()) SetCurrent(ctx.get());
			return ctx;
		}

		static CompilerContext* GetCurrent()
		{
			return GetCurrentRef();
		}

		static void SetCurrent(CompilerContext* ctx)
		{
			GetCurrentRef() = ctx;
		}

		~CompilerContext()
		{
			if (GetCurrent() == this)
			{
				SetCurrent(nullptr);
			}
		}

		AssemblyResolver& GetResolver() { return resolver; }
	};
}