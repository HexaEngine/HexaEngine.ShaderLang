#ifndef MODULE_READER_HPP
#define MODULE_READER_HPP

#include "module.hpp"
#include "module_tables.hpp"

namespace HXSL
{
	namespace Backend
	{
		class ModuleReader;
		class ModuleLinker;

		class ModuleReader
		{
			using RecordId = LayoutDataTypes::RecordId;
			using RecordSize = LayoutDataTypes::RecordSize;
			using ModuleId = LayoutDataTypes::ModuleId;
			using ModuleHeader = LayoutDataTypes::ModuleHeader;
			ObjPtr<Stream> stream;
			ModuleLinker& linker;
			uptr<Module> module;
			ModuleHeader header;
			int64_t dataSectionStart = 0;

			void ReadMetadata();
			void BuildImportRefs();
			Layout* ResolveImportSymbol(RecordId id);
			Module* ReadModule(ExportTableEntry& entry);
			NamespaceLayout* ReadNamespace(ExportTableEntry& entry);
			StructLayout* ReadStruct(ExportTableEntry& entry);
			EnumLayout* ReadEnum(ExportTableEntry& entry);
			FunctionLayout* ReadFunction(ExportTableEntry& entry);
			OperatorLayout* ReadOperator(ExportTableEntry& entry);
			ConstructorLayout* ReadConstructor(ExportTableEntry& entry);
			ParameterLayout* ReadParameter(ExportTableEntry& entry);
			FieldLayout* ReadField(ExportTableEntry& entry);
			PointerLayout* ReadPointerType(ExportTableEntry& entry);
			PrimitiveLayout* ReadPrimitiveType(ExportTableEntry& entry);
			StringSpan ReadStringSpan();
			ILCodeBlob* ReadILCodeBlob();

		public:
			ModuleReader(const ObjPtr<Stream>& s, ModuleLinker& linker);

			ModuleReader(ModuleReader&&) = delete;
			ModuleReader(const ModuleReader&) = delete;
			ModuleReader& operator=(ModuleReader&&) = delete;
			ModuleReader& operator=(const ModuleReader&) = delete;

			template<typename T>
			inline T ReadLittleEndian()
			{
				return stream->ReadLittleEndian<T>();
			}

			template<>
			inline RecordId ReadLittleEndian()
			{
				return RecordId(stream->ReadLittleEndian<uint64_t>());
			}

			RecordId ReadRecordRef();
			Layout* FindSymbol(RecordId id);

			template<typename T>
			T* FindSymbol(RecordId id)
			{
				Layout* layout = FindSymbol(id);
				return cast<T>(layout);
			}

			template<typename T>
			T* ReadRecordRef()
			{
				return FindSymbol<T>(ReadRecordRef());
			}

			Module* GetModule() { return module.get(); }
			Layout* ReadSymbol(RecordId id);
			uptr<Module> Read();
		};
	}
}

#endif // MODULE_READER_HPP