#include "core/module_linker.hpp"

namespace HXSL
{
	namespace Backend
	{
		ModuleLinker::ModuleIndex::ModuleIndex(ModuleLinker* linker, ModuleId id, Module* module) : id(id), module(module)
		{
			auto& exportTable = module->GetExportTable();
			for (auto& symbol : exportTable)
			{
				auto& name = symbol.name;
				auto idx = name.IndexOf('@');
				auto path = name[{idx, 0_rr}];
				auto symbolName = linker->pool.add(path);
				nameToSymbol.insert({ symbolName, symbol.layout });
			}
		}

		void ModuleLinker::AddModule(Module* module)
		{
			AddModuleIndex(module);
		}

		ModuleLinker::ModuleIndex* ModuleLinker::AddModuleIndex(Module* module)
		{
			auto index = make_uptr<ModuleIndex>(this, indices.size(), module);
			auto pIndex = index.get();
			indices.push_back(std::move(index));

			auto name = pool.add(module->GetName());
			auto version = module->GetVersion();
			nameToModule.insert({ {name, version}, pIndex });

			return pIndex;
		}

		ModuleLinker::ModuleIndex* ModuleLinker::ResolveModule(const ModuleReference& ref)
		{
			auto it = nameToModule.find(ref);
			if (it != nameToModule.end())
			{
				return it->second;
			}

			auto module = provider->LoadModule(ref);
			if (!module)
			{
				return nullptr;
			}
			auto index = AddModuleIndex(module);
			return index;
		}

		Layout* ModuleLinker::ResolveImport(const ModuleReference& ref, const StringSpan& name)
		{
			auto index = ResolveModule(ref);
			auto it = index->nameToSymbol.find(name);
			if (it == index->nameToSymbol.end())
			{
				return nullptr;
			}

			return it->second;
		}
	}
}