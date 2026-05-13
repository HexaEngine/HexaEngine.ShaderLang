#include "core/module_writer.hpp"
#include "pch/il.hpp"
#include "core/layout_builder.hpp"
#include "core/name_mangler.hpp"
#include "utils/endianness.hpp"
#include "il/il_encoding.hpp"
#include <random>

namespace HXSL
{
	namespace Backend
	{
		using ModuleHeader = LayoutDataTypes::ModuleHeader;

		ModuleWriter::RecordId ModuleWriter::GetRecordId(const Layout* layout)
		{
			auto it = recordMap.find(layout);
			RecordId id = {};
			if (it != recordMap.end())
			{
				id = it->second;
			}
			else
			{
				HXSL_ASSERT(false, "Record not found in map");
			}
			return id;
		}

		ModuleWriter::RecordScope ModuleWriter::WriteRecordHeader(const Layout* layout)
		{
			if (!writtenRecords.insert(layout).second)
			{
				HXSL_ASSERT(false, "Record already written");
				return RecordScope();
			}

			RecordId id = GetRecordId(layout);
			return RecordScope(*this, id);
		}

		void ModuleWriter::WriteRecordRef(const Layout* layout)
		{
			RecordId id = GetRecordId(layout);
			auto value = id.value;
			auto encoded = (value >> 63) | (value << 1);
			stream->WriteLEB128(encoded);
		}

		void ModuleWriter::WriteNamespace(const NamespaceLayout* ns)
		{
			auto scope = WriteRecordHeader(ns);
			WriteString(ns->GetName());

			WriteLittleEndian(static_cast<uint32_t>(ns->GetEnums().size()));
			for (auto* enm : ns->GetEnums())
			{
				WriteRecordRef(enm);
			}

			WriteLittleEndian(static_cast<uint32_t>(ns->GetStructs().size()));
			for (auto* strct : ns->GetStructs())
			{
				WriteRecordRef(strct);
			}

			WriteLittleEndian(static_cast<uint32_t>(ns->GetFunctions().size()));
			for (auto* func : ns->GetFunctions())
			{
				WriteRecordRef(func);
			}

			WriteLittleEndian(static_cast<uint32_t>(ns->GetGlobalFields().size()));
			for (auto* field : ns->GetGlobalFields())
			{
				WriteRecordRef(field);
			}

			WriteLittleEndian(static_cast<uint32_t>(ns->GetNestedNamespaces().size()));
			for (auto* nested : ns->GetNestedNamespaces())
			{
				WriteRecordRef(nested);
			}
		}

		void ModuleWriter::WriteStruct(const StructLayout* strct)
		{
			auto scope = WriteRecordHeader(strct);
			WriteString(strct->GetName());
			WriteLittleEndian(strct->GetAccess());
			WriteLittleEndian(strct->GetStructFlags());

			WriteLittleEndian(static_cast<uint32_t>(strct->GetFields().size()));
			for (auto* field : strct->GetFields())
			{
				WriteRecordRef(field);
			}

			WriteLittleEndian(static_cast<uint32_t>(strct->GetFunctions().size()));
			for (auto* func : strct->GetFunctions())
			{
				WriteRecordRef(func);
			}

			WriteLittleEndian(static_cast<uint32_t>(strct->GetOperators().size()));
			for (auto* op : strct->GetOperators())
			{
				WriteRecordRef(op);
			}

			WriteLittleEndian(static_cast<uint32_t>(strct->GetConstructors().size()));
			for (auto* ctor : strct->GetConstructors())
			{
				WriteRecordRef(ctor);
			}

			WriteLittleEndian(static_cast<uint32_t>(strct->GetStructs().size()));
			for (auto* nested : strct->GetStructs())
			{
				WriteRecordRef(nested);
			}
		}

		void ModuleWriter::WriteFunction(const FunctionLayout* func)
		{
			auto scope = WriteRecordHeader(func);
			WriteString(func->GetName());
			WriteRecordRef(func->GetReturnType());
			WriteLittleEndian(func->GetAccess());
			WriteLittleEndian(func->GetStorageClass());
			WriteLittleEndian(func->GetFunctionFlags());

			WriteLittleEndian(static_cast<uint32_t>(func->GetParameters().size()));
			for (auto* param : func->GetParameters())
			{
				WriteRecordRef(param);
			}

			func->GetCodeBlob()->Write(stream.Get(), context);
		}

		void ModuleWriter::WriteOperator(const OperatorLayout* op)
		{
			auto scope = WriteRecordHeader(op);
			WriteLittleEndian(op->GetOperator());
			WriteLittleEndian(op->GetOperatorFlags());
			WriteRecordRef(op->GetReturnType());
			WriteLittleEndian(op->GetAccess());
			WriteLittleEndian(op->GetStorageClass());
			WriteLittleEndian(op->GetFunctionFlags());

			WriteLittleEndian(static_cast<uint32_t>(op->GetParameters().size()));
			for (auto* param : op->GetParameters())
			{
				WriteRecordRef(param);
			}

			op->GetCodeBlob()->Write(stream.Get(), context);
		}

		void ModuleWriter::WriteConstructor(const ConstructorLayout* ctor)
		{
			auto scope = WriteRecordHeader(ctor);
			WriteLittleEndian(ctor->GetAccess());
			WriteLittleEndian(ctor->GetStorageClass());
			WriteLittleEndian(ctor->GetFunctionFlags());

			WriteLittleEndian(static_cast<uint32_t>(ctor->GetParameters().size()));
			for (auto* param : ctor->GetParameters())
			{
				WriteRecordRef(param);
			}

			ctor->GetCodeBlob()->Write(stream.Get(), context);
		}

		void ModuleWriter::WriteParameter(const ParameterLayout* param)
		{
			auto scope = WriteRecordHeader(param);
			WriteString(param->GetName());
			WriteString(param->GetSemantic());
			WriteRecordRef(param->GetType());
			WriteLittleEndian(param->GetStorageClass());
			WriteLittleEndian(param->GetInterpolationModifier());
			WriteLittleEndian(param->GetParameterFlags());
		}

		void ModuleWriter::WriteField(const FieldLayout* field)
		{
			auto scope = WriteRecordHeader(field);
			WriteString(field->GetName());
			WriteString(field->GetSemantic());
			WriteRecordRef(field->GetType());
			WriteLittleEndian(field->GetAccess());
			WriteLittleEndian(field->GetStorageClass());
			WriteLittleEndian(field->GetInterpolationModifier());
		}

		void ModuleWriter::WriteEnum(const EnumLayout* enm)
		{
			auto scope = WriteRecordHeader(enm);
			WriteString(enm->GetName());
			WriteRecordRef(enm->GetBaseType());
			WriteLittleEndian(enm->GetAccess());
			WriteLittleEndian(static_cast<uint32_t>(enm->GetItems().size()));
			for (auto* item : enm->GetItems())
			{
				WriteString(item->GetName());
				WriteLittleEndian(item->GetValue());
			}
		}

		void ModuleWriter::WritePointerType(const PointerLayout* ptr)
		{
			auto scope = WriteRecordHeader(ptr);
			WriteString(ptr->GetName());
			WriteLittleEndian(ptr->GetAccess());
			WriteRecordRef(ptr->GetElementType());
		}

		void ModuleWriter::WritePrimitiveType(const PrimitiveLayout* prim)
		{
			auto scope = WriteRecordHeader(prim);
			WriteString(prim->GetName());
			WriteLittleEndian(prim->GetKind());
			WriteLittleEndian(prim->GetClass());
			WriteLittleEndian(prim->GetRows());
			WriteLittleEndian(prim->GetColumns());
		}

		void ModuleWriter::WriteType(const TypeLayout* type)
		{
			switch (type->GetTypeId())
			{
			case LayoutType::PrimitiveLayoutType:
				WritePrimitiveType(cast<PrimitiveLayout>(type));
				break;
			case LayoutType::PointerLayoutType:
				WritePointerType(cast<PointerLayout>(type));
				break;
			case LayoutType::StructLayoutType:
				WriteStruct(cast<StructLayout>(type));
				break;
			default:
				break;
			}
		}

		void ModuleWriter::WriteModule(const Module* module)
		{
			auto scope = WriteRecordHeader(module);
			WriteString(module->GetName());
			module->GetVersion().Write(stream.Get());
			WriteLittleEndian(static_cast<uint32_t>(module->GetNamespaces().size()));
			for (auto* ns : module->GetNamespaces())
			{
				WriteRecordRef(ns);
			}
		}

		void ModuleWriter::Write(Module* module)
		{
			auto headerStart = stream->Position();
			stream->Seek(ModuleHeader::SizeOf(), SeekOrigin_Current); // reserve for header section.
			ModuleHeader header;

			referenceTable = make_uptr<ModuleReferenceTableBuilder>(module->GetModuleReferenceTable());
			exportTable = make_uptr<ExportTableBuilder>(module->GetExportTable());
			importTable = make_uptr<ImportTableBuilder>(module->GetImportTable());

			std::stack<std::pair<Layout*, bool>> walkStack;
			dense_set<const Layout*> visited;
			std::vector<const Layout*> sorted;

			NameMangler mangler = { *module, *referenceTable };
			BumpAllocator scratchAllocator{};
			std::string mangleBuffer;

			walkStack.push({ module , false });

			while (!walkStack.empty())
			{
				auto [layout, closing] = walkStack.top();
				walkStack.pop();
				if (closing)
				{
					auto res = recordMap.insert({ layout, RecordId() });
					if (res.second)
					{
						RecordId id;
						if (layout->IsExtern())
						{
							mangler.MangleExtern(layout, mangleBuffer);
							auto mangledName = scratchAllocator.CopyString(mangleBuffer);
							mangleBuffer.clear();

							id = importTable->Append(layout, mangledName);
						}
						else
						{
							mangler.Mangle(layout, mangleBuffer);
							auto mangledName = scratchAllocator.CopyString(mangleBuffer);
							mangleBuffer.clear();

							id = exportTable->Append(layout, mangledName);
							sorted.push_back(layout);
						}

						res.first->second = id;
					}
					continue;
				}
				if (!visited.insert(layout).second)
				{
					continue;
				}
				walkStack.push({ layout, true });
				auto type = layout->GetTypeId();
				switch (type)
				{
				case LayoutType::ModuleLayoutType:
				{
					auto* mod = cast<Module>(layout);
					for (auto* ns : mod->GetNamespaces())
					{
						walkStack.push({ ns, false });
					}
				}
				break;
				case LayoutType::NamespaceLayoutType:
				{
					auto* ns = cast<NamespaceLayout>(layout);
					for (auto* enm : ns->GetEnums())
					{
						walkStack.push({ enm, false });
					}
					for (auto* strct : ns->GetStructs())
					{
						walkStack.push({ strct, false });
					}
					for (auto* func : ns->GetFunctions())
					{
						walkStack.push({ func, false });
					}
					for (auto* field : ns->GetGlobalFields())
					{
						walkStack.push({ field, false });
					}
					for (auto* nested : ns->GetNestedNamespaces())
					{
						walkStack.push({ nested, false });
					}
				}
				break;
				case LayoutType::StructLayoutType:
				{
					auto* strct = cast<StructLayout>(layout);
					if (strct->IsExtern())
					{
					}
					else
					{
						for (auto* field : strct->GetFields())
						{
							walkStack.push({ field, false });
						}
						for (auto* func : strct->GetFunctions())
						{
							walkStack.push({ func, false });
						}
						for (auto* op : strct->GetOperators())
						{
							walkStack.push({ op, false });
						}
						for (auto* ctor : strct->GetConstructors())
						{
							walkStack.push({ ctor, false });
						}
						for (auto* nested : strct->GetStructs())
						{
							walkStack.push({ nested, false });
						}
					}
				}
				break;
				case LayoutType::FunctionLayoutType:
				{
					auto* func = cast<FunctionLayout>(layout);
					walkStack.push({ func->GetReturnType(), false });
					if (func->IsExtern())
					{
						for (auto* param : func->GetParameters())
						{
							walkStack.push({ param->GetType(), false });
						}
					}
					else
					{
						for (auto* param : func->GetParameters())
						{
							walkStack.push({ param, false });
						}
						auto ctx = func->GetContext();
						auto& metadata = ctx->GetMetadata();
						for (auto& func : metadata.functions)
						{
							walkStack.push({ func->func, false });
						}
						for (auto& type : metadata.typeMetadata)
						{
							walkStack.push({ type->def, false });
						}
					}
				}
				break;
				case LayoutType::OperatorLayoutType:
				{
					auto* op = cast<OperatorLayout>(layout);
					walkStack.push({ op->GetReturnType(), false });
					for (auto* param : op->GetParameters())
					{
						walkStack.push({ param, false });
					}
				}
				break;
				case LayoutType::ConstructorLayoutType:
				{
					auto* ctor = cast<ConstructorLayout>(layout);
					for (auto* param : ctor->GetParameters())
					{
						walkStack.push({ param, false });
					}
				}
				break;
				case LayoutType::FieldLayoutType:
				{
					auto* field = cast<FieldLayout>(layout);
					walkStack.push({ field->GetType(), false });
				}
				break;
				case LayoutType::ParameterLayoutType:
				{
					auto* param = cast<ParameterLayout>(layout);
					walkStack.push({ param->GetType(), false });
				}
				break;
				case LayoutType::PointerLayoutType:
				{
					auto* ptr = cast<PointerLayout>(layout);
					walkStack.push({ ptr->GetElementType(), false });
				}
				break;
				case LayoutType::EnumLayoutType:
				{
					auto* enm = cast<EnumLayout>(layout);
					walkStack.push({ enm->GetBaseType(), false });
				}
				break;
				case LayoutType::PrimitiveLayoutType:
					// Nothing to do
					break;
				default:
					HXSL_ASSERT(false, "Unknown layout type in module writer");
					break;
				}
			}

			// reserve space for tables.
			header.moduleReferenceTableSize = referenceTable->TotalSizeInBytes();
			auto referenceTableStart = stream->Position();
			stream->Seek(header.moduleReferenceTableSize, SeekOrigin_Current);

			header.exportTableSize = exportTable->EstimateSizeInBytes();
			auto exportTableStart = stream->Position();
			stream->Seek(header.exportTableSize, SeekOrigin_Current);

			header.importTableSize = importTable->TotalSizeInBytes();
			auto importTableStart = stream->Position();
			stream->Seek(header.importTableSize, SeekOrigin_Current);

			auto recordSectionStart = stream->Position();
			for (auto& layout : sorted)
			{
				if (layout->IsExtern())
				{
					continue;
				}
				switch (layout->GetTypeId())
				{
				case LayoutType::ModuleLayoutType:
					WriteModule(cast<Module>(layout));
					break;
				case LayoutType::NamespaceLayoutType:
					WriteNamespace(cast<NamespaceLayout>(layout));
					break;
				case LayoutType::PrimitiveLayoutType:
					WritePrimitiveType(cast<PrimitiveLayout>(layout));
					break;
				case LayoutType::PointerLayoutType:
					WritePointerType(cast<PointerLayout>(layout));
					break;
				case LayoutType::StructLayoutType:
					WriteStruct(cast<StructLayout>(layout));
					break;
				case LayoutType::FunctionLayoutType:
					WriteFunction(cast<FunctionLayout>(layout));
					break;
				case LayoutType::OperatorLayoutType:
					WriteOperator(cast<OperatorLayout>(layout));
					break;
				case LayoutType::ConstructorLayoutType:
					WriteConstructor(cast<ConstructorLayout>(layout));
					break;
				case LayoutType::ParameterLayoutType:
					WriteParameter(cast<ParameterLayout>(layout));
					break;
				case LayoutType::FieldLayoutType:
					WriteField(cast<FieldLayout>(layout));
					break;
				case LayoutType::EnumLayoutType:
					WriteEnum(cast<EnumLayout>(layout));
					break;
				default:
					HXSL_ASSERT(false, "Unknown layout type in module writer");
					break;
				}
			}

			uint64_t delta = 0;
			uint64_t firstSize = exportTable->EstimateSizeInBytes();
			uint64_t estimateSize = firstSize;
			do
			{
				uint64_t size = exportTable->ComputeSize();
				HXSL_ASSERT(size <= estimateSize, "Actual size is larger than estimate that should never happen.");
				delta = estimateSize - size;
				if (delta)
				{
					for (auto& entry : exportTable->table)
					{
						HXSL_ASSERT(entry.offset >= delta, "Export table offset underflow during compaction");
						entry.offset -= delta;
					}
				}
				estimateSize = size;
			} while (delta > 0);

			auto end = stream->Position();
			header.recordSectionSize = static_cast<uint64_t>(end - recordSectionStart);

			auto moveDelta = firstSize - estimateSize;
			if (moveDelta > 0)
			{
				Array<uint8_t, 4096> buffer;

				auto fromPos = recordSectionStart;
				auto toPos = recordSectionStart - moveDelta;
				auto toMove = header.recordSectionSize;
				while (toMove > 0)
				{
					stream->Position(fromPos);
					auto toCopy = std::min(toMove, buffer.size());
					stream->Read(buffer.data(), toCopy);
					stream->Position(toPos);
					stream->Write(buffer.data(), toCopy);
					fromPos += toCopy;
					toPos += toCopy;
					toMove -= toCopy;
				}

				recordSectionStart -= moveDelta;
				importTableStart -= moveDelta;
				end -= moveDelta;
			}
			header.exportTableSize = estimateSize;

			stream->Position(referenceTableStart);

			referenceTable->Write(stream.Get());

			HXSL_ASSERT(stream->Position() == exportTableStart, "Reference table size is invalid!");

			exportTable->Write(stream.Get());

			HXSL_ASSERT(stream->Position() == importTableStart, "Export table size is invalid!");

			importTable->Write(stream.Get());

			HXSL_ASSERT(stream->Position() == importTableStart + importTable->TotalSizeInBytes(), "Import table size is invalid!");

			stream->Position(headerStart);
			header.Write(stream.Get());

			stream->Position(end);
			stream->Length(end);
		}
	}
}