#pragma once

#include "module.hpp"
#include <utils/co_trampoline.hpp>

namespace HXSL
{
	namespace Backend
	{
		class IModuleProvider
		{
		public:
			virtual ~IModuleProvider() = default;
			virtual Module* LoadModule(const ModuleReference& ref) = 0;
		};

		class ModuleLinker
		{
			template<typename T>
			using CoTask = TrampolineTask<T>;
			using ModuleId = LayoutDataTypes::ModuleId;
			struct ModuleIndex
			{
				ModuleId id;
				Module* module;
				dense_map<StringSpan, RecordId> nameToSymbol;

				ModuleIndex(ModuleLinker* linker, ModuleId id, Module* module);
			};

			dense_map<Module*, ModuleIndex*> moduleToIndex;
			dense_map<ModuleReference, ModuleIndex*> nameToModule;
			vector<uptr<ModuleIndex>> indices;
			StringPool2 pool;
			IModuleProvider* provider;

			ModuleIndex* ResolveModule(const ModuleReference& ref);

			ModuleIndex* AddModuleIndex(Module* module);

			struct SymbolKey
			{
				Module* module = nullptr;
				RecordId record = {};
			};

			SymbolKey nextOperation = {};

		public:
			ModuleLinker(IModuleProvider& provider) : provider(&provider) {}
			void AddModule(Module* module);
			CoTask<Layout*> ResolveImport(const ModuleReference& ref, const StringSpan& name);

			CoTask<Layout*> ReadAsync(Module* module, RecordId recordId);
			Layout* Read(Module* module, RecordId recordId);
		};
	}
}