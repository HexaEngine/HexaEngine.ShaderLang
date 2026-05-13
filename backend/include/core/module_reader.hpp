#ifndef MODULE_READER_HPP
#define MODULE_READER_HPP

#include "module.hpp"
#include "module_tables.hpp"
#include <utils/co_trampoline.hpp>

namespace HXSL
{
	namespace Backend
	{
		class ModuleReader;
		class ModuleLinker;

		struct ModuleLinkerTask
		{
			RecordId recordId;
			void** slot;
		};

		class ModuleReader
		{
			friend class ModuleLinker;
			template<typename T>
			using CoTask = TrampolineTask<T>;
			using RecordId = LayoutDataTypes::RecordId;
			using RecordSize = LayoutDataTypes::RecordSize;
			using ModuleId = LayoutDataTypes::ModuleId;
			using ModuleHeader = LayoutDataTypes::ModuleHeader;
			ObjPtr<Stream> stream;
			ModuleLinker& linker;
			Module* module;
			ModuleHeader header;
			int64_t dataSectionStart = 0;
			RecordId nextRecordId = {};

			void ReadMetadata();
			CoTask<Layout*> ResolveImportSymbol(RecordId id);
			CoTask<Module*> ReadModule(ExportTableEntry& entry);
			CoTask<NamespaceLayout*> ReadNamespace(ExportTableEntry& entry);
			CoTask<StructLayout*> ReadStruct(ExportTableEntry& entry);
			CoTask<EnumLayout*> ReadEnum(ExportTableEntry& entry);
			CoTask<FunctionLayout*> ReadFunction(ExportTableEntry& entry);
			CoTask<OperatorLayout*> ReadOperator(ExportTableEntry& entry);
			CoTask<ConstructorLayout*> ReadConstructor(ExportTableEntry& entry);
			CoTask<ParameterLayout*> ReadParameter(ExportTableEntry& entry);
			CoTask<FieldLayout*> ReadField(ExportTableEntry& entry);
			CoTask<PointerLayout*> ReadPointerType(ExportTableEntry& entry);
			PrimitiveLayout* ReadPrimitiveType(ExportTableEntry& entry);
			StringSpan ReadStringSpan();
			CoTask<ILCodeBlob*> ReadILCodeBlob();

			CoTask<Layout*> ReadExportSymbol(RecordId id);

			ModuleReader(const ObjPtr<Stream>& s, ModuleLinker& linker, Module* module);
		public:
			ModuleReader(ModuleReader&&) = delete;
			ModuleReader(const ModuleReader&) = delete;
			ModuleReader& operator=(ModuleReader&&) = delete;
			ModuleReader& operator=(const ModuleReader&) = delete;

			[[nodiscard]] static ModuleReader* Create(const ObjPtr<Stream>& s, ModuleLinker& linker, Module* module) { return new ModuleReader(s, linker, module); }

			template<typename T>
			inline T ReadLittleEndian()
			{
				return stream->ReadLEB128<T>();
			}

			RecordId ReadRecordRef();
			CoTask<Layout*> FindSymbol(RecordId id);

			template<typename T>
			CoTask<T*> FindSymbol(RecordId id)
			{
				Layout* layout = co_await FindSymbol(id);
				co_return cast<T>(layout);
			}

			template<typename T>
			CoTask<T*> ReadRecordRef()
			{
				return FindSymbol<T>(ReadRecordRef());
			}

			Layout* ReadSymbol(RecordId id);
			void ReadFull();
		};
	}
}

#endif // MODULE_READER_HPP