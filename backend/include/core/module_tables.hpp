#pragma once

#include "module_base.hpp"

namespace HXSL
{
	namespace Backend
	{
		struct ExportTableEntry
		{
			StringSpan name = {};
			LayoutType type = LayoutType::Unknown;
			AccessModifier access = AccessModifier_None;
			uint64_t offset = 0;
			Layout* layout = nullptr;

			size_t SizeOf() const
			{
				return sizeof(uint32_t) + name.size() + sizeof(LayoutType) + sizeof(AccessModifier) + sizeof(uint64_t);
			}

			void Write(Stream* stream) const
			{
				stream->WriteLittleEndian(static_cast<uint32_t>(name.size()));
				stream->Write(name.data(), name.size());
				stream->WriteLittleEndian(type);
				stream->WriteLittleEndian(access);
				stream->WriteLittleEndian(offset);
			}

			void Read(Stream* stream, BumpAllocator& allocator)
			{
				auto len = stream->ReadLittleEndian<uint32_t>();
				auto* pname = allocator.AllocT<char>(len + 1);
				stream->Read(pname, len);
				pname[len] = 0;
				name = { pname, len };
				type = stream->ReadLittleEndian<LayoutType>();
				access = stream->ReadLittleEndian<AccessModifier>();
				offset = stream->ReadLittleEndian<uint64_t>();
			}
		};

		struct ExportTableHeader
		{
			uint64_t count = 0;

			static constexpr size_t SizeOf() { return sizeof(uint64_t); }

			void Write(Stream* stream) const
			{
				stream->WriteLittleEndian(count);
			}

			void Read(Stream* stream)
			{
				count = stream->ReadLittleEndian<uint64_t>();
			}
		};

		struct ExportTable
		{
			std::vector<ExportTableEntry> entries;

			ExportTableEntry& operator[](size_t index) { return entries[index]; }
			const ExportTableEntry& operator[](size_t index) const { return entries[index]; }
			ExportTableEntry& operator[](RecordId id) { return entries[IdToIndex(id)]; }
			const ExportTableEntry& operator[](RecordId id) const { return entries[IdToIndex(id)]; }

			size_t size() const { return entries.size(); }

			void Clear() { entries.clear(); }

			ExportTableEntry& Append(Layout* layout, const StringSpan& mangledName);

			RecordId IndexToId(size_t index) const { return RecordId::Export(static_cast<uint64_t>(index) + 1); }
			size_t IdToIndex(RecordId id) const { HXSL_ASSERT(id.IsExport(), "Not an exported symbol.");  return id.Index() - 1; }

			void SetOffset(RecordId id, uint64_t offset)
			{
				auto index = IdToIndex(id); // -1 due to base 1.
				entries[index].offset = offset;
			}

			auto begin() { return entries.begin(); }
			auto end() { return entries.end(); }
			auto begin() const { return entries.begin(); }
			auto end() const { return entries.end(); }

			auto rbegin() { return entries.rbegin(); }
			auto rend() { return entries.rend(); }
			auto rbegin() const { return entries.rbegin(); }
			auto rend() const { return entries.rend(); }

			void Write(Stream* stream) const
			{
				ExportTableHeader header = { static_cast<uint64_t>(entries.size()) };
				header.Write(stream);

				for (auto& entry : entries)
				{
					entry.Write(stream);
				}
			}

			void Read(Stream* stream, BumpAllocator& allocator)
			{
				ExportTableHeader header;
				header.Read(stream);

				auto count = static_cast<size_t>(header.count);
				entries.resize(count);
				for (size_t i = 0; i < count; ++i)
				{
					entries[i].Read(stream, allocator);
				}
			}
		};

		struct ExportTableBuilder
		{
			ExportTable& table;
			size_t sizeInBytes = 0;

			RecordId Append(Layout* layout, const StringSpan& mangledName);

			void SetOffset(RecordId id, uint64_t offset)
			{
				if (id.IsImport())
				{
					HXSL_ASSERT(false, "Not an exported symbol.");
				}
				auto index = id.Index() - 1; // -1 due to base 1.

				table[index].offset = offset;
			}

			size_t TotalSizeInBytes() const { return sizeInBytes + ExportTableHeader::SizeOf(); }

			void Write(Stream* stream) const
			{
				table.Write(stream);
			}
		};

		struct ImportTableEntry
		{
			StringSpan name = {};
			LayoutType type = LayoutType::Unknown;
			Layout* layout = nullptr;

			size_t SizeOf() const
			{
				return sizeof(uint32_t) + name.size() + sizeof(LayoutType);
			}

			void Write(Stream* stream) const
			{
				stream->WriteLittleEndian(static_cast<uint32_t>(name.size()));
				stream->Write(name.data(), name.size());
				stream->WriteLittleEndian(type);
			}

			void Read(Stream* stream, BumpAllocator& allocator)
			{
				auto len = stream->ReadLittleEndian<uint32_t>();
				auto* pname = allocator.AllocT<char>(len + 1);
				stream->Read(pname, len);
				pname[len] = 0;
				name = { pname, len };
				type = stream->ReadLittleEndian<LayoutType>();
			}
		};

		struct ImportTableHeader
		{
			uint64_t count = 0;

			static constexpr size_t SizeOf() { return sizeof(uint64_t); }

			void Write(Stream* stream) const
			{
				stream->WriteLittleEndian(count);
			}

			void Read(Stream* stream)
			{
				count = stream->ReadLittleEndian<uint64_t>();
			}
		};

		struct ImportTable
		{
			std::vector<ImportTableEntry> entries;

			ImportTableEntry& operator[](size_t index) { return entries[index]; }
			const ImportTableEntry& operator[](size_t index) const { return entries[index]; }
			ImportTableEntry& operator[](RecordId id) { return entries[IdToIndex(id)]; }
			const ImportTableEntry& operator[](RecordId id) const { return entries[IdToIndex(id)]; }

			size_t size() const { return entries.size(); }

			RecordId IndexToId(size_t index) const { return RecordId::Import(static_cast<uint64_t>(index) + 1); }
			size_t IdToIndex(RecordId id) const { HXSL_ASSERT(id.IsImport(), "Not an import symbol.");  return id.Index() - 1; }

			ImportTableEntry& Append(Layout* layout, const StringSpan& mangledName);

			void Clear()
			{
				entries.clear();
			}

			auto begin() { return entries.begin(); }
			auto end() { return entries.end(); }
			auto begin() const { return entries.begin(); }
			auto end() const { return entries.end(); }

			auto rbegin() { return entries.rbegin(); }
			auto rend() { return entries.rend(); }
			auto rbegin() const { return entries.rbegin(); }
			auto rend() const { return entries.rend(); }

			void Write(Stream* stream) const
			{
				ImportTableHeader header = { static_cast<uint64_t>(entries.size()) };
				header.Write(stream);

				for (auto& entry : entries)
				{
					entry.Write(stream);
				}
			}

			void Read(Stream* stream, BumpAllocator& allocator)
			{
				ImportTableHeader header;
				header.Read(stream);

				auto count = static_cast<size_t>(header.count);
				entries.resize(count);
				for (size_t i = 0; i < count; ++i)
				{
					entries[i].Read(stream, allocator);
				}
			}
		};

		struct ImportTableBuilder
		{
			ImportTable& table;
			size_t sizeInBytes = 0;

			RecordId Append(Layout* layout, const StringSpan& mangledName);

			size_t TotalSizeInBytes() const { return sizeInBytes + ImportTableHeader::SizeOf(); }

			void Write(Stream* stream) const
			{
				table.Write(stream);
			}
		};

		struct ModuleReference
		{
			StringSpan name;
			Version version;

			size_t SizeOf() const { return sizeof(uint32_t) + name.size() + Version::SizeOf(); }

			void Write(Stream* stream) const
			{
				stream->WriteLittleEndian(static_cast<uint32_t>(name.size()));
				stream->Write(name.data(), name.size());
				version.Write(stream);
			}

			void Read(Stream* stream, BumpAllocator& allocator)
			{
				auto len = stream->ReadLittleEndian<uint32_t>();
				auto* pname = allocator.AllocT<char>(len + 1);
				stream->Read(pname, len);
				pname[len] = 0;
				name = { pname, len };
				version.Read(stream);
			}

			bool operator==(const ModuleReference& ref) const
			{
				return name == ref.name && version == ref.version;
			}
		};

		struct ModuleReferenceTableHeader
		{
			using ModuleIdCount = LayoutDataTypes::ModuleIdCount;
			ModuleIdCount count = 0;

			static constexpr size_t SizeOf() { return sizeof(ModuleIdCount); }

			void Write(Stream* stream) const
			{
				stream->WriteLittleEndian(count);
			}

			void Read(Stream* stream)
			{
				count = stream->ReadLittleEndian<ModuleIdCount>();
			}
		};

		struct ModuleReferenceTable
		{
			using ModuleIdCount = LayoutDataTypes::ModuleIdCount;
			using ModuleId = LayoutDataTypes::ModuleId;
			std::vector<ModuleReference> entries;

			ModuleReference& operator[](size_t index) { return entries[index]; }
			const ModuleReference& operator[](size_t index) const { return entries[index]; }
			ModuleReference& operator[](ModuleId id) { return entries[IdToIndex(id)]; }
			const ModuleReference& operator[](ModuleId id) const { return entries[IdToIndex(id)]; }

			ModuleId IndexToId(size_t index) const { return static_cast<ModuleId>(index); }
			size_t IdToIndex(ModuleId id) const { return static_cast<size_t>(id); }

			size_t size() const { return entries.size(); }

			ModuleReference& Append(const Module* mod, const StringSpan& mangledName);

			void Clear()
			{
				entries.clear();
			}

			void Write(Stream* stream) const
			{
				ModuleReferenceTableHeader header = { static_cast<ModuleIdCount>(entries.size()) };
				header.Write(stream);

				for (auto& entry : entries)
				{
					entry.Write(stream);
				}
			}

			void Read(Stream* stream, BumpAllocator& allocator)
			{
				ModuleReferenceTableHeader header = {};
				header.Read(stream);

				auto count = static_cast<size_t>(header.count);
				entries.resize(count);
				for (size_t i = 0; i < count; ++i)
				{
					entries[i].Read(stream, allocator);
				}
			}
		};

		struct ModuleReferenceTableBuilder
		{
			using ModuleId = LayoutDataTypes::ModuleId;

			ModuleReferenceTable& table;
			size_t sizeInBytes = 0;

			ModuleId Append(const Module* module, const StringSpan& mangledName)
			{
				auto index = table.size();
				auto& entry = table.Append(module, mangledName);
				sizeInBytes += entry.SizeOf();

				return table.IndexToId(index);
			}

			size_t TotalSizeInBytes() const { return sizeInBytes + ModuleReferenceTableHeader::SizeOf(); }

			void Write(Stream* stream) const
			{
				table.Write(stream);
			}
		};
	}
}

namespace std
{
	template<>
	struct hash<HXSL::Backend::ModuleReference>
	{
		size_t operator()(const HXSL::Backend::ModuleReference& ref) const noexcept
		{
			XXHash3_64 hash;
			hash.Combine(ref.name.hash());
			hash.Combine(ref.version.hash());
			return static_cast<size_t>(hash.Finalize());
		}
	};
}