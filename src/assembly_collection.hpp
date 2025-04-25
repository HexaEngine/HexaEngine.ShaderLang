#ifndef ASSEMBLY_COLLECTION_HPP
#define ASSEMBLY_COLLECTION_HPP

#include "text_span.h"
#include "assembly.hpp"
#include "ast.hpp"

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
namespace HXSL
{
	class AssemblyCollection
	{
	private:
		std::vector<std::unique_ptr<Assembly>> assemblies;
		std::unordered_map<TextSpan, Assembly*, TextSpanHash, TextSpanEqual> nameToAssemblies;
	public:
		void AddAssembly(std::unique_ptr<Assembly> assembly)
		{
			auto& name = assembly->GetName();
			TextSpan span = TextSpan(name);
			nameToAssemblies.emplace(span, assembly.get());
			assemblies.push_back(std::move(assembly));
		}

		Assembly* GetAssembly(const TextSpan& name) const
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

		void FindAssembliesByNamespace(const TextSpan& target, std::vector<AssemblySymbolRef>& assemblyRefs, size_t lookupIndex = 0) const
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