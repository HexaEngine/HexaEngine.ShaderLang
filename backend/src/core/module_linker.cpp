#include "core/module_linker.hpp"
#include "core/module_reader.hpp"

namespace HXSL
{
	namespace Backend
	{
		template<typename T>
		using CoTask = TrampolineTask<T>;

		ModuleLinker::ModuleIndex::ModuleIndex(ModuleLinker* linker, ModuleId id, Module* module) : id(id), module(module)
		{
			auto& exportTable = module->GetExportTable();
			for (size_t i = 0; i < exportTable.size(); ++i)
			{
				auto recordId = exportTable.IndexToId(i);
				auto& symbol = exportTable[i];
				auto& name = symbol.name;
				auto idx = name.IndexOf('@');
				auto path = name[{idx, 0_rr}];
				auto symbolName = linker->pool.add(path);
				nameToSymbol.insert({ symbolName, recordId });
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

		CoTask<Layout*> ModuleLinker::ResolveImport(const ModuleReference& ref, const StringSpan& name)
		{
			auto index = ResolveModule(ref);
			auto it = index->nameToSymbol.find(name);
			if (it == index->nameToSymbol.end())
			{
				co_return nullptr;
			}

			auto module = index->module;
			auto recordId = it->second;

			co_return co_await ReadAsync(module, recordId);
		}

		CoTask<Layout*> ModuleLinker::ReadAsync(Module* module, RecordId recordId)
		{
			if (recordId.IsExport())
			{
				auto& entry = module->GetExportTable()[recordId];
				if (entry.layout) co_return entry.layout;
			}
			else
			{
				auto& entry = module->GetImportTable()[recordId];
				if (entry.layout) co_return entry.layout;
			}

			nextOperation = { module, recordId };
			co_await TrampolineBounce();

			Layout* layout;
			if (recordId.IsExport())
			{
				layout = module->GetExportTable()[recordId].layout;
			}
			else
			{
				layout = module->GetImportTable()[recordId].layout;
			}

			co_return layout;
		}

		Layout* ModuleLinker::Read(Module* module, RecordId recordId)
		{
			TrampolineTaskScheduler sched;

			sched.insert_frame();
			auto task = module->GetReader()->ReadExportSymbol(recordId);
			if (!task.IsCompleted())
			{
				while (true)
				{
					sched.insert_frame();
					std::cout << "Discover: " << nextOperation.record.value << std::endl;
					auto nextRecordId = nextOperation.record;
					auto nextModule = nextOperation.module;
					CoTask<Layout*> taskInner;
					if (nextRecordId.IsExport())
					{
						taskInner = nextModule->GetReader()->ReadExportSymbol(nextRecordId);
					}
					else
					{
						taskInner = nextModule->GetReader()->ResolveImportSymbol(nextRecordId);
					}

					if (!taskInner.IsCompleted())
					{
						continue;
					}
					if (sched.pump())
					{
						break;
					}
				}
			}

			HXSL_ASSERT(task.IsCompleted(), "Task wasn't completed yet");
			return task.GetResult();

			return nullptr;
		}
	}
}