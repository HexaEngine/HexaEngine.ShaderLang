#pragma once

#include "pch/std.hpp"
#include "il/assembly_loader.hpp"

namespace HXSL
{
	class CompilerContext
	{
		AssemblyLoadContext assemblyLoadContext;

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

		AssemblyLoadContext& GetAssemblyLoadContext() { return assemblyLoadContext; }
		AssemblyResolver& GetResolver() { return assemblyLoadContext.GetResolver(); }
	};
}