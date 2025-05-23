#ifndef ASSEMBLY_COLLECTION_HPP
#define ASSEMBLY_COLLECTION_HPP

#include "assembly.hpp"
#include "pch/ast.hpp"

namespace HXSL
{
	class AssemblyCollection
	{
	private:
		std::vector<std::unique_ptr<Assembly>> assemblies;
		std::unordered_map<StringSpan, Assembly*, StringSpanHash, StringSpanEqual> nameToAssemblies;
	public:
		void AddAssembly(std::unique_ptr<Assembly> assembly)
		{
			nameToAssemblies.emplace(assembly->GetName(), assembly.get());
			assemblies.push_back(std::move(assembly));
		}

		Assembly* GetAssembly(const StringSpan& name) const
		{
			auto it = nameToAssemblies.find(name);
			if (it != nameToAssemblies.end())
			{
				return it->second;
			}
			return nullptr;
		}

		const std::vector<std::unique_ptr<Assembly>>& GetAssemblies() const
		{
			return assemblies;
		}

		void FindAssembliesByNamespace(const StringSpan& target, std::vector<AssemblySymbolRef>& assemblyRefs, size_t lookupIndex = 0) const
		{
			for (auto& assembly : assemblies)
			{
				auto index = assembly->GetSymbolTable()->FindNodeIndexFullPath(target, lookupIndex);
				if (index.valid())
				{
					assemblyRefs.push_back(AssemblySymbolRef(assembly.get(), index));
				}
			}
		}

		void LoadAssemblyFromFile(const std::string& path)
		{
			std::unique_ptr<Assembly> assembly;
			if (Assembly::LoadFromFile(path, assembly) == 0)
			{
				AddAssembly(std::move(assembly));
			}
		}
	};
}
#endif