#include "assembly_resolver.hpp"
#include "assembly_loader.hpp"
#include <filesystem>

namespace HXSL
{
	Assembly* AssemblyResolver::ResolveInner(const AssemblyReference& ref)
	{
		for (const auto& path : searchPaths)
		{
			std::string fullPath = path + "/" + ref.name;
			if (std::filesystem::exists(fullPath))
			{
				return context.LoadFromFile(fullPath);
			}
		}
		return nullptr;
	}

	AssemblyResolver::AssemblyResolver(AssemblyLoadContext& context) : context(context)
	{
		searchPaths.push_back(std::filesystem::current_path().string());
	}

	void AssemblyResolver::AddSearchPath(const std::string& path)
	{
		searchPaths.push_back(path);
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

	Backend::Module* AssemblyResolver::LoadModule(const Backend::ModuleReference& ref)
	{
		auto assembly = Resolve({ ref.name.str() });
		if (!assembly) return nullptr;
		return assembly->GetModule();
	}
}