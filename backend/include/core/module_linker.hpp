#pragma once

#include "module.hpp"

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
			using ModuleId = LayoutDataTypes::ModuleId;
			struct ModuleIndex
			{
				ModuleId id;
				Module* module;
				dense_map<StringSpan, Layout*> nameToSymbol;

				ModuleIndex(ModuleLinker* linker, ModuleId id, Module* module);
			};

			dense_map<Module*, ModuleIndex*> moduleToIndex;
			dense_map<ModuleReference, ModuleIndex*> nameToModule;
			vector<uptr<ModuleIndex>> indices;
			StringPool2 pool;
			IModuleProvider* provider;

			ModuleIndex* ResolveModule(const ModuleReference& ref);

			ModuleIndex* AddModuleIndex(Module* module);

		public:
			ModuleLinker(IModuleProvider& provider) : provider(&provider) {}
			void AddModule(Module* module);
			Layout* ResolveImport(const ModuleReference& ref, const StringSpan& name);
		};
	}
}