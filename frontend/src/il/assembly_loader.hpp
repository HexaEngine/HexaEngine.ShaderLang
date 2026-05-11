#pragma once

#include "assembly_resolver.hpp"
#include "assembly_format.hpp"
#include <core/module_linker.hpp>

namespace HXSL
{
	class AssemblyLoadContext
	{
		using ModuleLinker = Backend::ModuleLinker;
		AssemblyResolver resolver;
		ModuleLinker linker;
		std::vector<uptr<Assembly>> assemblies;
		dense_map<StringSpan, Assembly*> nameToAssembly;
		std::shared_mutex mutex;

		static std::atomic<AssemblyLoadContext*>& GetDefaultRef()
		{
			static std::atomic<AssemblyLoadContext*> context = { nullptr };
			return context;
		}

	public:
		AssemblyLoadContext() : resolver(*this), linker({ resolver }) {}
		Assembly* LoadFromStream(uptr<Stream>&& stream);
		Assembly* LoadFromFile(const StringSpan& path);
		AssemblyResolver& GetResolver() { return resolver; }
		ModuleLinker& GetLinker() { return linker; }
		AssemblyCollection BuildCollection();
		std::shared_mutex& GetMutex() { return mutex; }

		~AssemblyLoadContext()
		{
			auto expected = this;
			GetDefaultRef().compare_exchange_strong(expected, nullptr, std::memory_order_release, std::memory_order_relaxed);
		}

		static AssemblyLoadContext* GetDefault() { return GetDefaultRef().load(std::memory_order_acquire); }
		static void SetDefault(AssemblyLoadContext* context) { GetDefaultRef().store(context, std::memory_order_release); }
	};

	class AssemblyReader
	{
		AssemblyLoadContext& ctx;
		ObjPtr<Stream> stream;
	public:
		AssemblyReader(AssemblyLoadContext& ctx, const ObjPtr<Stream>& stream) : ctx(ctx), stream(stream) {}

		AssemblyLoadResult Read(const StringSpan& pathOrName, uptr<Assembly>& assembly);
	};
}