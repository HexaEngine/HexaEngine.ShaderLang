#include "core/module_reader.hpp"
#include "core/module_linker.hpp"
#include "pch/il.hpp"
#include "core/layout_builder.hpp"
#include "utils/endianness.hpp"
#include "il/il_encoding.hpp"

namespace HXSL
{
	namespace Backend
	{
		template<typename T>
		using CoTask = TrampolineTask<T>;

		void ModuleReader::ReadMetadata()
		{
			header.Read(stream.Get());
			dataSectionStart = stream->Position();

			auto& allocator = module->GetAllocator();
			auto& referenceTable = module->GetModuleReferenceTable();
			auto& exportTable = module->GetExportTable();
			auto& importTable = module->GetImportTable();
			referenceTable.Read(stream.Get(), allocator);
			exportTable.Read(stream.Get(), allocator);
			importTable.Read(stream.Get(), allocator);
			module->AddStateFlag(ModuleStateFlags::HasTables | ModuleStateFlags::FromFile);

			auto end = dataSectionStart + static_cast<int64_t>(header.moduleReferenceTableSize + header.exportTableSize + header.importTableSize + header.recordSectionSize);
			stream->Position(end);
		}

		CoTask<Layout*> ModuleReader::ResolveImportSymbol(RecordId id)
		{
			auto& entry = module->GetImportTable()[id];
			if (entry.layout) co_return entry.layout;

			auto _ = stream->BeginJump(); // note: we simply backup the stream position here, since the import could come from the same assembly.
			auto& referenceTable = module->GetModuleReferenceTable();

			auto& name = entry.name;
			auto idx = name.IndexOf('@');
			auto moduleIdSpan = name[{0, idx}];
			ModuleId val;
			auto res = std::from_chars(moduleIdSpan.data(), moduleIdSpan.data() + moduleIdSpan.size(), val, 16);
			auto& modRef = referenceTable[val];

			auto path = name[{idx, 0_rr}];
			auto layout = co_await linker.ResolveImport(modRef, path);

			entry.layout = layout;
			co_return layout;
		}

		ModuleReader::ModuleReader(const ObjPtr<Stream>& s, ModuleLinker& linker, Module* module) : stream(s), linker(linker), module(module)
		{
			ReadMetadata();
		}

		ModuleReader::RecordId ModuleReader::ReadRecordRef()
		{
			auto encoded = stream->ReadLEB128<uint64_t>();
			auto id = (encoded >> 1) | (encoded << 63);
			return RecordId(id);
		}

		static size_t gid = 0;

		CoTask<Layout*> ModuleReader::FindSymbol(RecordId id)
		{
			return linker.ReadAsync(module, id);
		}

		CoTask<Layout*> ModuleReader::ReadExportSymbol(RecordId id)
		{
			auto& entry = module->GetExportTable()[id];
			if (entry.layout) co_return entry.layout;

			auto _ = stream->BeginJump(entry.offset);

			Layout* layout = nullptr;

			switch (entry.type)
			{
			case LayoutType::ModuleLayoutType:
				layout = co_await ReadModule(entry);
				break;
			case LayoutType::NamespaceLayoutType:
				layout = co_await ReadNamespace(entry);
				break;
			case LayoutType::PrimitiveLayoutType:
				layout = ReadPrimitiveType(entry);
				break;
			case LayoutType::PointerLayoutType:
				layout = co_await ReadPointerType(entry);
				break;
			case LayoutType::EnumLayoutType:
				layout = co_await ReadEnum(entry);
				break;
			case LayoutType::StructLayoutType:
				layout = co_await ReadStruct(entry);
				break;
			case LayoutType::FunctionLayoutType:
			{
				auto function = co_await ReadFunction(entry);
				layout = function;
			}
			break;
			case LayoutType::OperatorLayoutType:
			{
				auto op = co_await ReadOperator(entry);
				layout = op;
			}
			break;
			case LayoutType::ConstructorLayoutType:
			{
				auto ctor = co_await ReadConstructor(entry);
				layout = ctor;
			}
			break;
			case LayoutType::ParameterLayoutType:
				layout = co_await ReadParameter(entry);
				break;
			case LayoutType::FieldLayoutType:
				layout = co_await ReadField(entry);
				break;
			default:
				HXSL_ASSERT(false, "Unknown layout type in module reader");
				break;
			}

			layout->SetExportId(id);
			co_return layout;
		}

		Layout* ModuleReader::ReadSymbol(RecordId id)
		{
			return linker.Read(module, id);
		}

		void ModuleReader::ReadFull()
		{
			auto& exportTable = module->GetExportTable();
			auto recordCount = exportTable.size();

			for (uint64_t i = recordCount; i-- > 0;)
			{
				auto& entry = exportTable[i];
				if (entry.layout) continue;

				auto recordId = exportTable.IndexToId(i);
				stream->Position(entry.offset);

				ReadSymbol(recordId);
			}

			auto& alloc = module->GetAllocator();
			module->AddStateFlag(ModuleStateFlags::HasLayouts);
		}

		CoTask<Module*> ModuleReader::ReadModule(ExportTableEntry& entry)
		{
			entry.layout = module;
			auto name = ReadStringSpan();
			module->SetName(name);
			Version version;
			version.Read(stream.Get());
			module->SetVersion(version);
			auto nsCount = ReadLittleEndian<uint32_t>();
			for (uint32_t i = 0; i < nsCount; ++i)
			{
				module->AddNamespace(co_await ReadRecordRef<NamespaceLayout>());
			}

			co_return module;
		}

		CoTask<NamespaceLayout*> ModuleReader::ReadNamespace(ExportTableEntry& entry)
		{
			NamespaceLayoutBuilder builder(*module);
			entry.layout = builder.Peek();
			StringSpan name = ReadStringSpan();

			builder.Name(name);

			auto enumCount = ReadLittleEndian<uint32_t>();
			for (uint32_t i = 0; i < enumCount; ++i)
			{
				builder.AddEnum(co_await ReadRecordRef<EnumLayout>());
			}

			auto structCount = ReadLittleEndian<uint32_t>();
			for (uint32_t i = 0; i < structCount; ++i)
			{
				builder.AddStruct(co_await ReadRecordRef<StructLayout>());
			}

			auto funcCount = ReadLittleEndian<uint32_t>();
			for (uint32_t i = 0; i < funcCount; ++i)
			{
				builder.AddFunction(co_await ReadRecordRef<FunctionLayout>());
			}

			auto fieldCount = ReadLittleEndian<uint32_t>();
			for (uint32_t i = 0; i < fieldCount; ++i)
			{
				builder.AddGlobalField(co_await ReadRecordRef<FieldLayout>());
			}

			auto nestedCount = ReadLittleEndian<uint32_t>();
			for (uint32_t i = 0; i < nestedCount; ++i)
			{
				builder.AddNestedNamespace(co_await ReadRecordRef<NamespaceLayout>());
			}

			co_return builder.Build();
		}

		CoTask<StructLayout*> ModuleReader::ReadStruct(ExportTableEntry& entry)
		{
			StructLayoutBuilder builder(*module);
			entry.layout = builder.Peek();
			auto name = ReadStringSpan();
			auto access = ReadLittleEndian<AccessModifier>();
			auto flags = ReadLittleEndian<StructLayoutFlags>();

			builder.Name(name)
				.Access(access)
				.StructFlags(flags);

			auto fieldCount = ReadLittleEndian<uint32_t>();
			for (uint32_t i = 0; i < fieldCount; ++i)
			{
				builder.AddField(co_await ReadRecordRef<FieldLayout>());
			}

			auto funcCount = ReadLittleEndian<uint32_t>();
			for (uint32_t i = 0; i < funcCount; ++i)
			{
				builder.AddFunction(co_await ReadRecordRef<FunctionLayout>());
			}

			auto opCount = ReadLittleEndian<uint32_t>();
			for (uint32_t i = 0; i < opCount; ++i)
			{
				builder.AddOperator(co_await ReadRecordRef<OperatorLayout>());
			}

			auto ctorCount = ReadLittleEndian<uint32_t>();
			for (uint32_t i = 0; i < ctorCount; ++i)
			{
				builder.AddConstructor(co_await ReadRecordRef<ConstructorLayout>());
			}

			auto nestedCount = ReadLittleEndian<uint32_t>();
			for (uint32_t i = 0; i < nestedCount; ++i)
			{
				builder.AddType(co_await ReadRecordRef<StructLayout>());
			}

			co_return builder.Build();
		}

		CoTask<EnumLayout*> ModuleReader::ReadEnum(ExportTableEntry& entry)
		{
			EnumLayoutBuilder builder(*module);
			entry.layout = builder.Peek();
			auto name = ReadStringSpan();
			auto* baseType = co_await ReadRecordRef<TypeLayout>();
			auto access = ReadLittleEndian<AccessModifier>();

			builder.Name(name)
				.BaseType(baseType)
				.Access(access);

			auto itemCount = ReadLittleEndian<uint32_t>();
			for (uint32_t i = 0; i < itemCount; ++i)
			{
				auto itemName = ReadStringSpan();
				auto itemValue = ReadLittleEndian<uint64_t>();
				builder.AddItem(itemName, itemValue);
			}

			co_return builder.Build();
		}

		CoTask<FunctionLayout*> ModuleReader::ReadFunction(ExportTableEntry& entry)
		{
			FunctionLayoutBuilder builder(*module);
			entry.layout = builder.Peek();
			auto name = ReadStringSpan();
			auto returnType = co_await ReadRecordRef<TypeLayout>();
			auto access = ReadLittleEndian<AccessModifier>();
			auto storageClass = ReadLittleEndian<StorageClass>();
			auto functionFlags = ReadLittleEndian<FunctionFlags>();

			builder.Name(name)
				.ReturnType(returnType)
				.Access(access)
				.StorageClass(storageClass)
				.FunctionFlags(functionFlags);

			auto paramCount = ReadLittleEndian<uint32_t>();
			for (uint32_t i = 0; i < paramCount; ++i)
			{
				builder.AddParameter(co_await ReadRecordRef<ParameterLayout>());
			}

			auto blob = co_await ReadILCodeBlob();
			builder.CodeBlob(blob);

			co_return builder.Build();
		}

		CoTask<OperatorLayout*> ModuleReader::ReadOperator(ExportTableEntry& entry)
		{
			OperatorLayoutBuilder builder(*module);
			entry.layout = builder.Peek();
			auto op = ReadLittleEndian<Operator>();
			auto opFlags = ReadLittleEndian<OperatorFlags>();
			auto* returnType = co_await ReadRecordRef<TypeLayout>();
			auto access = ReadLittleEndian<AccessModifier>();
			auto storageClass = ReadLittleEndian<StorageClass>();
			auto funcFlags = ReadLittleEndian<FunctionFlags>();

			builder
				.Operator(op)
				.OperatorFlags(opFlags)
				.ReturnType(returnType)
				.Access(access)
				.StorageClass(storageClass)
				.FunctionFlags(funcFlags);

			auto paramCount = ReadLittleEndian<uint32_t>();
			for (uint32_t i = 0; i < paramCount; ++i)
			{
				builder.AddParameter(co_await ReadRecordRef<ParameterLayout>());
			}

			auto blob = co_await ReadILCodeBlob();
			builder.CodeBlob(blob);

			co_return builder.Build();
		}

		CoTask<ConstructorLayout*> ModuleReader::ReadConstructor(ExportTableEntry& entry)
		{
			ConstructorLayoutBuilder builder(*module);
			entry.layout = builder.Peek();
			auto access = ReadLittleEndian<AccessModifier>();
			auto storageClass = ReadLittleEndian<StorageClass>();
			auto flags = ReadLittleEndian<FunctionFlags>();

			builder.Access(access)
				.StorageClass(storageClass)
				.FunctionFlags(flags);

			auto paramCount = ReadLittleEndian<uint32_t>();
			for (uint32_t i = 0; i < paramCount; ++i)
			{
				builder.AddParameter(co_await ReadRecordRef<ParameterLayout>());
			}

			auto blob = co_await ReadILCodeBlob();
			builder.CodeBlob(blob);

			co_return builder.Build();
		}

		CoTask<ParameterLayout*> ModuleReader::ReadParameter(ExportTableEntry& entry)
		{
			ParameterLayoutBuilder builder(*module);
			entry.layout = builder.Peek();
			auto name = ReadStringSpan();
			auto semantic = ReadStringSpan();
			auto* type = co_await ReadRecordRef<TypeLayout>();
			auto storageClass = ReadLittleEndian<StorageClass>();
			auto interpolMod = ReadLittleEndian<InterpolationModifier>();
			auto parameterFlags = ReadLittleEndian<ParameterFlags>();

			builder.Name(name)
				.Semantic(semantic)
				.Type(type)
				.StorageClass(storageClass)
				.InterpolationModifier(interpolMod)
				.ParameterFlags(parameterFlags);

			co_return builder.Build();
		}

		CoTask<FieldLayout*> ModuleReader::ReadField(ExportTableEntry& entry)
		{
			FieldLayoutBuilder builder(*module);
			entry.layout = builder.Peek();
			auto name = ReadStringSpan();
			auto semantic = ReadStringSpan();
			auto* type = co_await ReadRecordRef<TypeLayout>();
			auto access = ReadLittleEndian<AccessModifier>();
			auto storageClass = ReadLittleEndian<StorageClass>();
			auto interpolMod = ReadLittleEndian<InterpolationModifier>();

			builder.Name(name)
				.Semantic(semantic)
				.Type(type)
				.Access(access)
				.StorageClass(storageClass)
				.InterpolationModifier(interpolMod);

			co_return builder.Build();
		}

		CoTask<PointerLayout*> ModuleReader::ReadPointerType(ExportTableEntry& entry)
		{
			PointerLayoutBuilder builder(*module);
			entry.layout = builder.Peek();
			auto name = ReadStringSpan();
			auto access = ReadLittleEndian<AccessModifier>();
			auto* elementType = co_await ReadRecordRef<TypeLayout>();

			co_return builder.Name(name)
				.Access(access)
				.ElementType(elementType)
				.Build();
		}

		PrimitiveLayout* ModuleReader::ReadPrimitiveType(ExportTableEntry& entry)
		{
			PrimitiveLayoutBuilder builder(*module);
			entry.layout = builder.Peek();
			StringSpan name = ReadStringSpan();
			PrimitiveKind kind = ReadLittleEndian<PrimitiveKind>();
			PrimitiveClass primClass = ReadLittleEndian<PrimitiveClass>();
			uint32_t rows = ReadLittleEndian<uint32_t>();
			uint32_t columns = ReadLittleEndian<uint32_t>();

			return builder.Name(name)
				.Access(AccessModifier_Public)
				.Kind(kind)
				.Class(primClass)
				.Rows(rows)
				.Columns(columns)
				.Build();
		}

		StringSpan ModuleReader::ReadStringSpan()
		{
			uint32_t len = stream->ReadLEB128<uint32_t>();
			if (len == 0)
			{
				return {};
			}

			auto& allocator = module->GetAllocator();
			char* buffer = static_cast<char*>(allocator.Alloc(static_cast<size_t>(len) + 1, alignof(char)));

			stream->Read(buffer, len);
			buffer[len] = '\0';
			return StringSpan(buffer, len);
		}

		CoTask<ILCodeBlob*> ModuleReader::ReadILCodeBlob()
		{
			auto& alloc = module->GetAllocator();
			auto codeBlob = alloc.Alloc<ILCodeBlob>();
			co_await codeBlob->Read(stream.Get(), *this);
			co_return codeBlob;
		}
	}
}