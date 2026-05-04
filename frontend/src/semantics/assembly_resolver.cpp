#include "assembly_resolver.hpp"
#include <filesystem>

namespace HXSL
{
	Assembly* AssemblyResolver::ResolveInner(const AssemblyReference& ref)
	{
		auto it = assemblies.find(ref.name);
		if (it != assemblies.end())
		{
			return it->second.get();
		}

		for (const auto& path : searchPaths)
		{
			std::string fullPath = path + "/" + ref.name;
			if (std::filesystem::exists(fullPath))
			{
				uptr<Assembly> assembly;
				auto result = Assembly::LoadFromFile(fullPath, assembly);
				if (result == AssemblyLoadResult_Success)
				{
					auto ptr = assembly.get();
					auto spanName = pool.add(ref.name);
					assemblies.insert({ spanName, std::move(assembly) });
					return ptr;
				}
			}
		}
		return nullptr;
	}

	AssemblyResolver::AssemblyResolver()
	{
		searchPaths.push_back(std::filesystem::current_path().string());
	}

	void AssemblyResolver::AddSearchPath(const std::string& path)
	{
		searchPaths.push_back(path);
	}

	void AssemblyResolver::AddAssembly(std::unique_ptr<Assembly> && assembly)
	{
		auto& name = assembly->GetName();
		auto spanName = pool.add(name);
		assemblies.insert({ spanName, std::move(assembly) });
	}

	Assembly* AssemblyResolver::Resolve(const AssemblyReference& name)
	{
		auto* assembly = ResolveInner(name);
		if (!assembly)
		{
			return nullptr;
		}

		std::stack<Assembly*> walkStack;
		walkStack.push(assembly);
		while (!walkStack.empty())
		{
			auto* ass = walkStack.top();
			walkStack.pop();

			for (auto& ref : ass->GetReferencedAssemblies())
			{
				auto referencedAss = ResolveInner(ref);
				if (!referencedAss)
				{
					return nullptr;
				}
				walkStack.push(referencedAss);
			}
		}

		return assembly;
	}

	AssemblyCollection AssemblyResolver::BuildCollection()
	{
		AssemblyCollection collection;
		auto it = assemblies.begin();
		auto end = assemblies.end();
		for (; it != end; ++it)
		{
			collection.AddAssembly(std::move(it->second));
		}
		assemblies.clear();
		return collection;
	}
}