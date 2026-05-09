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
			auto& allocator = module->GetAllocator();
			auto& referenceTable = module->GetModuleReferenceTable();
			auto& exportTable = module->GetExportTable();
			auto& importTable = module->GetImportTable();
			referenceTable.Read(stream, allocator);
			exportTable.Read(stream, allocator);
			importTable.Read(stream, allocator);
			module->AddStateFlag(ModuleStateFlags::HasTables | ModuleStateFlags::FromFile);
		}

		void ModuleReader::BuildImportRefs(ModuleLinker& linker)
		{
			auto& referenceTable = module->GetModuleReferenceTable();
			auto& importTable = module->GetImportTable();

			for (auto& entry : importTable)
			{
				auto& name = entry.name;
				auto idx = name.IndexOf('@');
				auto moduleIdSpan = name[{0, idx}];
				ModuleId val;
				auto res = std::from_chars(moduleIdSpan.data(), moduleIdSpan.data() + moduleIdSpan.size(), val, 16);
				auto& modRef = referenceTable[val];

				auto path = name[{idx, 0_rr}];
				auto layout = linker.ResolveImport(modRef, path);

				entry.layout = layout;
			}
		}

		ModuleReader::ModuleReader(Stream* s) : stream(s), module(make_uptr<Module>())
		{
			ReadMetadata();
		}

		ModuleReader::RecordId ModuleReader::ReadRecordRef()
		{
			return ReadLittleEndian<RecordId>();
		}

		Layout* ModuleReader::FindSymbol(RecordId id)
		{
			Layout* layout;
			if (id.IsExport())
			{
				layout = module->GetExportTable()[id].layout;
			}
			else
			{
				layout = module->GetImportTable()[id].layout;
			}

			HXSL_ASSERT(layout, "Symbol not found.");
			return layout;
		}

		uptr<Module> ModuleReader::Read(ModuleLinker& linker)
		{
			BuildImportRefs(linker);

			auto& exportTable = module->GetExportTable();
			auto recordCount = exportTable.size();

			std::vector<FunctionLayout*> functions;
			for (uint64_t i = 0; i < recordCount; ++i)
			{
				auto& entry = exportTable[i];
				auto recordId = exportTable.IndexToId(i);
				stream->Position(entry.offset);

				Layout* layout = nullptr;

				switch (entry.type)
				{
				case LayoutType::ModuleLayoutType:
					layout = ReadModule();
					break;
				case LayoutType::NamespaceLayoutType:
					layout = ReadNamespace();
					break;
				case LayoutType::PrimitiveLayoutType:
					layout = ReadPrimitiveType();
					break;
				case LayoutType::PointerLayoutType:
					layout = ReadPointerType();
					break;
				case LayoutType::EnumLayoutType:
					layout = ReadEnum();
					break;
				case LayoutType::StructLayoutType:
					layout = ReadStruct();
					break;
				case LayoutType::FunctionLayoutType:
				{
					auto function = ReadFunction();
					functions.push_back(function);
					layout = function;
				}
				break;
				case LayoutType::OperatorLayoutType:
				{
					auto op = ReadOperator();
					functions.push_back(op);
					layout = op;
				}
				break;
				case LayoutType::ConstructorLayoutType:
				{
					auto ctor = ReadConstructor();
					functions.push_back(ctor);
					layout = ctor;
				}
				break;
				case LayoutType::ParameterLayoutType:
					layout = ReadParameter();
					break;
				case LayoutType::FieldLayoutType:
					layout = ReadField();
					break;
				default:
					HXSL_ASSERT(false, "Unknown layout type in module reader");
					break;
				}

				entry.layout = layout;
				layout->SetExportId(recordId);
			}

			auto& alloc = module->GetAllocator();
			module->SetAllFunctions(alloc.CopySpan(functions));
			module->AddStateFlag(ModuleStateFlags::HasLayouts);

			return std::move(module);
		}

		Module* ModuleReader::ReadModule()
		{
			auto name = ReadStringSpan();
			module->SetName(name);
			Version version;
			version.Read(stream);
			module->SetVersion(version);
			auto nsCount = ReadLittleEndian<uint32_t>();
			for (uint32_t i = 0; i < nsCount; ++i)
			{
				module->AddNamespace(ReadRecordRef<NamespaceLayout>());
			}

			return module.get();
		}

		NamespaceLayout* ModuleReader::ReadNamespace()
		{
			StringSpan name = ReadStringSpan();

			NamespaceLayoutBuilder builder(*module);
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

		StructLayout* ModuleReader::ReadStruct()
		{
			auto name = ReadStringSpan();
			auto access = ReadLittleEndian<AccessModifier>();
			auto flags = ReadLittleEndian<StructLayoutFlags>();

			StructLayoutBuilder builder(*module);
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

		EnumLayout* ModuleReader::ReadEnum()
		{
			auto name = ReadStringSpan();
			auto* baseType = ReadRecordRef<TypeLayout>();
			auto access = ReadLittleEndian<AccessModifier>();

			EnumLayoutBuilder builder(*module);
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

		FunctionLayout* ModuleReader::ReadFunction()
		{
			auto name = ReadStringSpan();
			auto returnType = ReadRecordRef<TypeLayout>();
			auto access = ReadLittleEndian<AccessModifier>();
			auto storageClass = ReadLittleEndian<StorageClass>();
			auto functionFlags = ReadLittleEndian<FunctionFlags>();

			FunctionLayoutBuilder builder(*module);
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

		OperatorLayout* ModuleReader::ReadOperator()
		{
			auto op = ReadLittleEndian<Operator>();
			auto opFlags = ReadLittleEndian<OperatorFlags>();
			auto* returnType = ReadRecordRef<TypeLayout>();
			auto access = ReadLittleEndian<AccessModifier>();
			auto storageClass = ReadLittleEndian<StorageClass>();
			auto funcFlags = ReadLittleEndian<FunctionFlags>();

			OperatorLayoutBuilder builder(*module);
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

		ConstructorLayout* ModuleReader::ReadConstructor()
		{
			auto access = ReadLittleEndian<AccessModifier>();
			auto storageClass = ReadLittleEndian<StorageClass>();
			auto flags = ReadLittleEndian<FunctionFlags>();

			ConstructorLayoutBuilder builder(*module);
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

		ParameterLayout* ModuleReader::ReadParameter()
		{
			auto name = ReadStringSpan();
			auto semantic = ReadStringSpan();
			auto* type = ReadRecordRef<TypeLayout>();
			auto storageClass = ReadLittleEndian<StorageClass>();
			auto interpolMod = ReadLittleEndian<InterpolationModifier>();
			auto parameterFlags = ReadLittleEndian<ParameterFlags>();

			ParameterLayoutBuilder builder(*module);
			builder.Name(name)
				.Semantic(semantic)
				.Type(type)
				.StorageClass(storageClass)
				.InterpolationModifier(interpolMod)
				.ParameterFlags(parameterFlags);

			return builder.Build();
		}

		FieldLayout* ModuleReader::ReadField()
		{
			auto name = ReadStringSpan();
			auto semantic = ReadStringSpan();
			auto* type = ReadRecordRef<TypeLayout>();
			auto access = ReadLittleEndian<AccessModifier>();
			auto storageClass = ReadLittleEndian<StorageClass>();
			auto interpolMod = ReadLittleEndian<InterpolationModifier>();

			FieldLayoutBuilder builder(*module);
			builder.Name(name)
				.Semantic(semantic)
				.Type(type)
				.Access(access)
				.StorageClass(storageClass)
				.InterpolationModifier(interpolMod);

			return builder.Build();
		}

		PointerLayout* ModuleReader::ReadPointerType()
		{
			auto name = ReadStringSpan();
			auto access = ReadLittleEndian<AccessModifier>();
			auto* elementType = ReadRecordRef<TypeLayout>();

			PointerLayoutBuilder builder(*module);
			return builder.Name(name)
				.Access(access)
				.ElementType(elementType)
				.Build();
		}

		PrimitiveLayout* ModuleReader::ReadPrimitiveType()
		{
			StringSpan name = ReadStringSpan();
			PrimitiveKind kind = ReadLittleEndian<PrimitiveKind>();
			PrimitiveClass primClass = ReadLittleEndian<PrimitiveClass>();
			uint32_t rows = ReadLittleEndian<uint32_t>();
			uint32_t columns = ReadLittleEndian<uint32_t>();

			PrimitiveLayoutBuilder builder(*module);
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
			codeBlob->Read(stream, context);
			return codeBlob;
		}
	}
}