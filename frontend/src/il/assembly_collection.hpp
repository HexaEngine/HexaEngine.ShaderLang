#ifndef ASSEMBLY_COLLECTION_HPP
#define ASSEMBLY_COLLECTION_HPP

#include "assembly.hpp"
#include "pch/ast.hpp"

namespace HXSL
{
	class AssemblyCollection
	{
	private:
		std::vector<Assembly*> assemblies;
		std::unordered_map<StringSpan, Assembly*> nameToAssemblies;
	public:
		void AddAssembly(Assembly* assembly)
		{
			nameToAssemblies.emplace(assembly->GetName(), assembly);
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

		const std::vector<Assembly*>& GetAssemblies() const
		{
			return assemblies;
		}

		void FindAssembliesByNamespace(const StringSpan& target, std::vector<AssemblySymbolRef>& assemblyRefs, SymbolTableNode* lookupIndex = nullptr) const
		{
			for (auto& assembly : assemblies)
			{
				auto index = assembly->GetSymbolTable()->FindNodeIndexFullPath(target, lookupIndex);
				if (index.valid())
				{
					assemblyRefs.push_back(AssemblySymbolRef(assembly, index));
				}
			}
		}
	};
}
#endif