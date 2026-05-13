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
			sizeInBytes += entry.SizeOfEstimate();

			auto id = RecordId::Export(table.size()); // 1 based; 0 is null
			layout->SetExportId(id);
			return id;
		}

		uint64_t ExportTableBuilder::ComputeSize() const
		{
			uint64_t size = ExportTableHeader(table.size()).SizeOf();
			uint64_t prevOffset = 0;
			for (auto& entry : table)
			{
				HXSL_ASSERT(entry.offset >= prevOffset, "Export table entries must be written in monotonically increasing offset order");

				size += entry.SizeOf(prevOffset);
				prevOffset = entry.offset;
			}

			return size;
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