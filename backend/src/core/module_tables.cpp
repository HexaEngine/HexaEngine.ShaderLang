#include "core/module_tables.hpp"
#include "core/module.hpp"

namespace HXSL
{
	namespace Backend
	{
		ExportTableEntry& ExportTable::Append(Layout* layout, const StringSpan& mangledName)
		{
			auto typeId = layout->GetTypeId();
			auto accessMod = layout->GetAccessModifier();
			entries.push_back({ mangledName, typeId, accessMod, 0, layout });
			return entries.back();
		}

		RecordId ExportTableBuilder::Append(Layout* layout, const StringSpan& mangledName)
		{
			auto& entry = table.Append(layout, mangledName);
			sizeInBytes += entry.SizeOf();

			auto id = RecordId::Export(table.size()); // 1 based; 0 is null
			layout->SetExportId(id);
			return id;
		}

		ImportTableEntry& ImportTable::Append(Layout* layout, const StringSpan& mangledName)
		{
			auto typeId = layout->GetTypeId();
			entries.push_back({ mangledName, typeId, layout });
			return entries.back();
		}

		RecordId ImportTableBuilder::Append(Layout* layout, const StringSpan& mangledName)
		{
			auto& entry = table.Append(layout, mangledName);
			sizeInBytes += entry.SizeOf();

			auto id = RecordId::Import(table.size()); // 1 based; 0 is null
			return id;
		}

		ModuleReference& ModuleReferenceTable::Append(const Module* mod, const StringSpan& mangledName)
		{
			auto& version = mod->GetVersion();
			entries.push_back({ mangledName, version });
			return entries.back();
		}
	}
}