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

		struct ModuleReaderContext
		{
			using RecordId = LayoutDataTypes::RecordId;
			using RecordSize = LayoutDataTypes::RecordSize;
			ModuleReader& reader;
		};

		class ModuleReader
		{
			using RecordId = LayoutDataTypes::RecordId;
			using RecordSize = LayoutDataTypes::RecordSize;
			using ModuleId = LayoutDataTypes::ModuleId;
			Stream* stream;
			uptr<Module> module;
			ModuleReaderContext context{ *this };

			void ReadMetadata();
			void BuildImportRefs(ModuleLinker& linker);
			Module* ReadModule();
			NamespaceLayout* ReadNamespace();
			StructLayout* ReadStruct();
			EnumLayout* ReadEnum();
			FunctionLayout* ReadFunction();
			OperatorLayout* ReadOperator();
			ConstructorLayout* ReadConstructor();
			ParameterLayout* ReadParameter();
			FieldLayout* ReadField();
			PointerLayout* ReadPointerType();
			PrimitiveLayout* ReadPrimitiveType();
			StringSpan ReadStringSpan();
			ILCodeBlob* ReadILCodeBlob();

		public:
			ModuleReader(Stream* s);

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
			uptr<Module> Read(ModuleLinker& linker);
		};
	}
}

#endif // MODULE_READER_HPP