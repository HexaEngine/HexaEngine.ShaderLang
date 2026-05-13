#ifndef MODULE_WRITER_HPP
#define MODULE_WRITER_HPP

#include "module.hpp"
#include "module_tables.hpp"

namespace HXSL
{
	namespace Backend
	{
		class ModuleWriter;

		struct ModuleWriterContext
		{
			using RecordId = LayoutDataTypes::RecordId;
			using RecordSize = LayoutDataTypes::RecordSize;
			ModuleWriter& writer;
			const dense_map<const Layout*, RecordId>& recordMap;
		};

		class ModuleWriter
		{
			using RecordId = LayoutDataTypes::RecordId;
			using RecordSize = LayoutDataTypes::RecordSize;
			ObjPtr<Stream> stream;
			dense_map<const Layout*, RecordId> recordMap;
			dense_set<const Layout*> writtenRecords;
			uptr<ModuleReferenceTableBuilder> referenceTable;
			uptr<ExportTableBuilder> exportTable;
			uptr<ImportTableBuilder> importTable;

			ModuleWriterContext context{ *this, recordMap };

		public:
			ModuleWriter(const ObjPtr<Stream>& s) : stream(s) {}

			template<typename T>
			inline void WriteLittleEndian(T value)
			{
				stream->WriteLEB128(value);
			}

			inline void WriteString(const StringSpan& str)
			{
				uint32_t len = static_cast<uint32_t>(str.size());
				stream->WriteLEB128(len);
				if (len == 0) return;
				stream->Write(str.data(), len);
			}

			struct RecordScope
			{
				ModuleWriter* writer = nullptr;
				int64_t start = -1;
				RecordId id;

				RecordScope() = default;
				RecordScope(ModuleWriter& writer, RecordId id) : writer(&writer), start(writer.stream->Position()), id(id)
				{
				}

				RecordScope(const RecordScope&) = delete;
				RecordScope& operator=(const RecordScope&) = delete;

				RecordScope(RecordScope&& other) noexcept
					: writer(other.writer), start(other.start), id(other.id)
				{
					other.writer = nullptr;
					other.start = -1;
				}

				RecordScope& operator=(RecordScope&& other) noexcept
				{
					if (this != &other)
					{
						End();

						writer = other.writer;
						start = other.start;
						id = other.id;

						other.writer = nullptr;
						other.start = -1;
					}

					return *this;
				}

				void End()
				{
					if (!writer) return;

					auto stream = writer->stream;

					if (id.IsExport())
					{
						writer->exportTable->SetOffset(id, static_cast<uint64_t>(start));
					}

					writer = nullptr;
					start = -1;
				}

				~RecordScope()
				{
					End();
				}
			};

			RecordId GetRecordId(const Layout* layout);
			[[nodiscard]] RecordScope WriteRecordHeader(const Layout* layout);
			void WriteRecordRef(const Layout* layout);
			void WriteNamespace(const NamespaceLayout* ns);
			void WriteStruct(const StructLayout* strct);
			void WriteFunction(const FunctionLayout* func);
			void WriteOperator(const OperatorLayout* op);
			void WriteConstructor(const ConstructorLayout* ctor);
			void WriteParameter(const ParameterLayout* param);
			void WriteField(const FieldLayout* field);
			void WriteEnum(const EnumLayout* enm);
			void WritePointerType(const PointerLayout* ptr);
			void WritePrimitiveType(const PrimitiveLayout* prim);
			void WriteType(const TypeLayout* type);
			void WriteModule(const Module* module);

			void Write(Module* module);
		};
	}
}

#endif // MODULE_WRITER_HPP