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

		void ModuleReader::BuildImportRefs()
		{
			auto& referenceTable = module->GetModuleReferenceTable();
			auto& importTable = module->GetImportTable();

			for (size_t i = 0; i < importTable.size(); ++i)
			{
				ResolveImportSymbol(importTable.IndexToId(i));
			}
		}

		Layout* ModuleReader::ResolveImportSymbol(RecordId id)
		{
			auto& entry = module->GetImportTable()[id];
			if (entry.layout) return entry.layout;

			auto _ = stream->BeginJump(); // note: we simply backup the stream position here, since the import could come from the same assembly.
			auto& referenceTable = module->GetModuleReferenceTable();

			auto& name = entry.name;
			auto idx = name.IndexOf('@');
			auto moduleIdSpan = name[{0, idx}];
			ModuleId val;
			auto res = std::from_chars(moduleIdSpan.data(), moduleIdSpan.data() + moduleIdSpan.size(), val, 16);
			auto& modRef = referenceTable[val];

			auto path = name[{idx, 0_rr}];
			auto layout = linker.ResolveImport(modRef, path);

			entry.layout = layout;
			return layout;
		}

		ModuleReader::ModuleReader(const ObjPtr<Stream>& s, ModuleLinker& linker) : stream(s), linker(linker), module(make_uptr<Module>())
		{
			ReadMetadata();
		}

		ModuleReader::RecordId ModuleReader::ReadRecordRef()
		{
			return ReadLittleEndian<RecordId>();
		}

		Layout* ModuleReader::FindSymbol(RecordId id)
		{
			Layout* layout = nullptr;
			if (id.IsExport())
			{
				layout = ReadSymbol(id);
			}
			else
			{
				layout = ResolveImportSymbol(id);
			}

			HXSL_ASSERT(layout, "Symbol not found.");
			return layout;
		}

		Layout* ModuleReader::ReadSymbol(RecordId id)
		{
			auto& entry = module->GetExportTable()[id];
			if (entry.layout) return entry.layout;

			auto _ = stream->BeginJump(entry.offset);

			Layout* layout = nullptr;

			switch (entry.type)
			{
			case LayoutType::ModuleLayoutType:
				layout = ReadModule(entry);
				break;
			case LayoutType::NamespaceLayoutType:
				layout = ReadNamespace(entry);
				break;
			case LayoutType::PrimitiveLayoutType:
				layout = ReadPrimitiveType(entry);
				break;
			case LayoutType::PointerLayoutType:
				layout = ReadPointerType(entry);
				break;
			case LayoutType::EnumLayoutType:
				layout = ReadEnum(entry);
				break;
			case LayoutType::StructLayoutType:
				layout = ReadStruct(entry);
				break;
			case LayoutType::FunctionLayoutType:
			{
				auto function = ReadFunction(entry);
				layout = function;
			}
			break;
			case LayoutType::OperatorLayoutType:
			{
				auto op = ReadOperator(entry);
				layout = op;
			}
			break;
			case LayoutType::ConstructorLayoutType:
			{
				auto ctor = ReadConstructor(entry);
				layout = ctor;
			}
			break;
			case LayoutType::ParameterLayoutType:
				layout = ReadParameter(entry);
				break;
			case LayoutType::FieldLayoutType:
				layout = ReadField(entry);
				break;
			default:
				HXSL_ASSERT(false, "Unknown layout type in module reader");
				break;
			}

			layout->SetExportId(id);
			return layout;
		}

		uptr<Module> ModuleReader::Read()
		{
			BuildImportRefs();

			auto& exportTable = module->GetExportTable();
			auto recordCount = exportTable.size();

			std::vector<FunctionLayout*> functions;
			for (uint64_t i = 0; i < recordCount; ++i)
			{
				auto& entry = exportTable[i];
				auto recordId = exportTable.IndexToId(i);
				stream->Position(entry.offset);

				ReadSymbol(recordId);
			}

			auto& alloc = module->GetAllocator();
			module->SetAllFunctions(alloc.CopySpan(functions));
			module->AddStateFlag(ModuleStateFlags::HasLayouts);

			return std::move(module);
		}

		Module* ModuleReader::ReadModule(ExportTableEntry& entry)
		{
			entry.layout = module.get();
			auto name = ReadStringSpan();
			module->SetName(name);
			Version version;
			version.Read(stream.Get());
			module->SetVersion(version);
			auto nsCount = ReadLittleEndian<uint32_t>();
			for (uint32_t i = 0; i < nsCount; ++i)
			{
				module->AddNamespace(ReadRecordRef<NamespaceLayout>());
			}

			return module.get();
		}

		NamespaceLayout* ModuleReader::ReadNamespace(ExportTableEntry& entry)
		{
			NamespaceLayoutBuilder builder(*module);
			entry.layout = builder.Peek();
			StringSpan name = ReadStringSpan();

			builder.Name(name);

			auto enumCount = ReadLittleEndian<uint32_t>();
			for (uint32_t i = 0; i < enumCount; ++i)
			{
				builder.AddEnum(ReadRecordRef<EnumLayout>());
			}

			auto structCount = ReadLittleEndian<uint32_t>();
			for (uint32_t i = 0; i < structCount; ++i)
			{
				builder.AddStruct(ReadRecordRef<StructLayout>());
			}

			auto funcCount = ReadLittleEndian<uint32_t>();
			for (uint32_t i = 0; i < funcCount; ++i)
			{
				builder.AddFunction(ReadRecordRef<FunctionLayout>());
			}

			auto fieldCount = ReadLittleEndian<uint32_t>();
			for (uint32_t i = 0; i < fieldCount; ++i)
			{
				builder.AddGlobalField(ReadRecordRef<FieldLayout>());
			}

			auto nestedCount = ReadLittleEndian<uint32_t>();
			for (uint32_t i = 0; i < nestedCount; ++i)
			{
				builder.AddNestedNamespace(ReadRecordRef<NamespaceLayout>());
			}

			return builder.Build();
		}

		StructLayout* ModuleReader::ReadStruct(ExportTableEntry& entry)
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
				builder.AddField(ReadRecordRef<FieldLayout>());
			}

			auto funcCount = ReadLittleEndian<uint32_t>();
			for (uint32_t i = 0; i < funcCount; ++i)
			{
				builder.AddFunction(ReadRecordRef<FunctionLayout>());
			}

			auto opCount = ReadLittleEndian<uint32_t>();
			for (uint32_t i = 0; i < opCount; ++i)
			{
				builder.AddOperator(ReadRecordRef<OperatorLayout>());
			}

			auto ctorCount = ReadLittleEndian<uint32_t>();
			for (uint32_t i = 0; i < ctorCount; ++i)
			{
				builder.AddConstructor(ReadRecordRef<ConstructorLayout>());
			}

			auto nestedCount = ReadLittleEndian<uint32_t>();
			for (uint32_t i = 0; i < nestedCount; ++i)
			{
				builder.AddType(ReadRecordRef<StructLayout>());
			}

			return builder.Build();
		}

		EnumLayout* ModuleReader::ReadEnum(ExportTableEntry& entry)
		{
			EnumLayoutBuilder builder(*module);
			entry.layout = builder.Peek();
			auto name = ReadStringSpan();
			auto* baseType = ReadRecordRef<TypeLayout>();
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

			return builder.Build();
		}

		FunctionLayout* ModuleReader::ReadFunction(ExportTableEntry& entry)
		{
			FunctionLayoutBuilder builder(*module);
			entry.layout = builder.Peek();
			auto name = ReadStringSpan();
			auto returnType = ReadRecordRef<TypeLayout>();
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
				builder.AddParameter(ReadRecordRef<ParameterLayout>());
			}

			auto blob = ReadILCodeBlob();
			builder.CodeBlob(blob);

			return builder.Build();
		}

		OperatorLayout* ModuleReader::ReadOperator(ExportTableEntry& entry)
		{
			OperatorLayoutBuilder builder(*module);
			entry.layout = builder.Peek();
			auto op = ReadLittleEndian<Operator>();
			auto opFlags = ReadLittleEndian<OperatorFlags>();
			auto* returnType = ReadRecordRef<TypeLayout>();
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
				builder.AddParameter(ReadRecordRef<ParameterLayout>());
			}

			auto blob = ReadILCodeBlob();
			builder.CodeBlob(blob);

			return builder.Build();
		}

		ConstructorLayout* ModuleReader::ReadConstructor(ExportTableEntry& entry)
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
				builder.AddParameter(ReadRecordRef<ParameterLayout>());
			}

			auto blob = ReadILCodeBlob();
			builder.CodeBlob(blob);

			return builder.Build();
		}

		ParameterLayout* ModuleReader::ReadParameter(ExportTableEntry& entry)
		{
			ParameterLayoutBuilder builder(*module);
			entry.layout = builder.Peek();
			auto name = ReadStringSpan();
			auto semantic = ReadStringSpan();
			auto* type = ReadRecordRef<TypeLayout>();
			auto storageClass = ReadLittleEndian<StorageClass>();
			auto interpolMod = ReadLittleEndian<InterpolationModifier>();
			auto parameterFlags = ReadLittleEndian<ParameterFlags>();

			builder.Name(name)
				.Semantic(semantic)
				.Type(type)
				.StorageClass(storageClass)
				.InterpolationModifier(interpolMod)
				.ParameterFlags(parameterFlags);

			return builder.Build();
		}

		FieldLayout* ModuleReader::ReadField(ExportTableEntry& entry)
		{
			FieldLayoutBuilder builder(*module);
			entry.layout = builder.Peek();
			auto name = ReadStringSpan();
			auto semantic = ReadStringSpan();
			auto* type = ReadRecordRef<TypeLayout>();
			auto access = ReadLittleEndian<AccessModifier>();
			auto storageClass = ReadLittleEndian<StorageClass>();
			auto interpolMod = ReadLittleEndian<InterpolationModifier>();

			builder.Name(name)
				.Semantic(semantic)
				.Type(type)
				.Access(access)
				.StorageClass(storageClass)
				.InterpolationModifier(interpolMod);

			return builder.Build();
		}

		PointerLayout* ModuleReader::ReadPointerType(ExportTableEntry& entry)
		{
			PointerLayoutBuilder builder(*module);
			entry.layout = builder.Peek();
			auto name = ReadStringSpan();
			auto access = ReadLittleEndian<AccessModifier>();
			auto* elementType = ReadRecordRef<TypeLayout>();

			return builder.Name(name)
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
			uint32_t len = ReadLittleEndian<uint32_t>();
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

		ILCodeBlob* ModuleReader::ReadILCodeBlob()
		{
			auto& alloc = module->GetAllocator();
			auto codeBlob = alloc.Alloc<ILCodeBlob>();
			codeBlob->Read(stream.Get(), *this);
			return codeBlob;
		}
	}
}