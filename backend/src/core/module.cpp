#include "core/module.hpp"
#include "pch/il.hpp"
#include "core/layout_builder.hpp"
#include "utils/endianness.hpp"
#include "il/il_encoding.hpp"

namespace HXSL
{
    namespace Backend
    {
        ModuleWriter::RecordId ModuleWriter::GetRecordId(const Layout* layout)
        {
            auto it = recordMap.find(layout);
            RecordId id = 0;
            if (it != recordMap.end())
            {
                id = it->second;
            }
            else
            {
                id = recordCounter++;
                recordMap.insert({ layout, id });
            }
            return id;
        }

        bool ModuleWriter::WriteRecordHeader(const Layout* layout)
        {
            if (!writtenRecords.insert(layout).second)
            {
                HXSL_ASSERT(false, "Record already written");
                return false;
            }

            WriteLittleEndian(layout->GetTypeId());
            RecordId id = GetRecordId(layout);
            WriteLittleEndian(id);
            return true;
        }

        void ModuleWriter::WriteRecordRef(const Layout* layout)
        {
            RecordId id = GetRecordId(layout);
            WriteLittleEndian(id);
        }

        void ModuleWriter::WriteNamespace(const NamespaceLayout* ns) 
        {
            WriteRecordHeader(ns);
            WriteString(ns->GetName());
            
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
            WriteRecordHeader(strct);
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
            WriteRecordHeader(func);
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

            func->GetCodeBlob()->Write(stream, context);
        }

        void ModuleWriter::WriteOperator(const OperatorLayout* op)
        {
            WriteRecordHeader(op);
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

            op->GetCodeBlob()->Write(stream, context);
        }

        void ModuleWriter::WriteConstructor(const ConstructorLayout* ctor)
        {
            WriteRecordHeader(ctor);
            WriteLittleEndian(ctor->GetAccess());
            WriteLittleEndian(ctor->GetStorageClass());
            WriteLittleEndian(ctor->GetFunctionFlags());

            WriteLittleEndian(static_cast<uint32_t>(ctor->GetParameters().size()));
            for (auto* param : ctor->GetParameters())
            {
                WriteRecordRef(param);
            }

            ctor->GetCodeBlob()->Write(stream, context);
        }

        void ModuleWriter::WriteParameter(const ParameterLayout* param)
        {
            WriteRecordHeader(param);
            WriteString(param->GetName());
            WriteString(param->GetSemantic());
            WriteRecordRef(param->GetType());
            WriteLittleEndian(param->GetStorageClass());
            WriteLittleEndian(param->GetInterpolationModifier());
            WriteLittleEndian(param->GetParameterFlags());
        }

        void ModuleWriter::WriteField(const FieldLayout* field)
        {
            WriteRecordHeader(field);
            WriteString(field->GetName());
            WriteString(field->GetSemantic());
            WriteRecordRef(field->GetType());
            WriteLittleEndian(field->GetAccess());
            WriteLittleEndian(field->GetStorageClass());
            WriteLittleEndian(field->GetInterpolationModifier());
        }

        void ModuleWriter::WritePointerType(const PointerLayout* ptr)
        {
            WriteRecordHeader(ptr);
            WriteString(ptr->GetName());
            WriteLittleEndian(ptr->GetAccess());
            WriteRecordRef(ptr->GetElementType());
        }

        void ModuleWriter::WritePrimitiveType(const PrimitiveLayout* prim)
        {
            WriteRecordHeader(prim);
            WriteString(prim->GetName());
            WriteLittleEndian(prim->GetAccess());
            WriteLittleEndian(prim->GetKind());
            WriteLittleEndian(prim->GetClass());
            WriteLittleEndian(prim->GetRows());
            WriteLittleEndian(prim->GetColumns());
        }

        void ModuleWriter::WriteType(const TypeLayout* type)
        {
            switch (type->GetTypeId())
            {
            case Layout::PrimitiveLayoutType:
                WritePrimitiveType(cast<PrimitiveLayout>(type));
                break;
            case Layout::PointerLayoutType:
                WritePointerType(cast<PointerLayout>(type));
                break;
            case Layout::StructLayoutType:
                WriteStruct(cast<StructLayout>(type));
                break;
            default:
                break;
            }
        }

        void ModuleWriter::WriteModule(const Module* module)
        {
            WriteRecordHeader(module);
            WriteLittleEndian(static_cast<uint32_t>(module->GetNamespaces().size()));
            for (auto* ns : module->GetNamespaces())
            {
                WriteRecordRef(ns);
            }
        }

        void ModuleWriter::Write(const Module* module)
        {
            std::stack<std::pair<const Layout*, bool>> walkStack;
            dense_set<const Layout*> visited;
            std::vector<const Layout*> sorted;

            RecordId recordCounter = 1;
            walkStack.push({ module , false });
            while (!walkStack.empty())
            {
                auto[layout, closing] = walkStack.top();
                walkStack.pop();
                if (closing)
                {
                    recordMap.insert({ layout, recordCounter++ });
                    sorted.push_back(layout);
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
                case Layout::ModuleLayoutType:
                    {
                    auto* mod = cast<Module>(layout);
                    for (auto* ns : mod->GetNamespaces())
                    {
                        walkStack.push({ ns, false });
                    }
                }
                break;
                case Layout::NamespaceLayoutType:
                {
                    auto* ns = cast<NamespaceLayout>(layout);
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
                case Layout::StructLayoutType:
                {
                    auto* strct = cast<StructLayout>(layout);
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
                break;
                case Layout::FunctionLayoutType:
                {
                    auto* func = cast<FunctionLayout>(layout);
                    walkStack.push({ func->GetReturnType(), false });
                    for (auto* param : func->GetParameters())
                    {
                        walkStack.push({ param, false });
                    }
                }
                break;
                case Layout::OperatorLayoutType:
                {
                    auto* op = cast<OperatorLayout>(layout);
                    walkStack.push({ op->GetReturnType(), false });
                    for (auto* param : op->GetParameters())
                    {
                        walkStack.push({ param, false });
                    }
                }
                break;
                case Layout::ConstructorLayoutType:
                {
                    auto* ctor = cast<ConstructorLayout>(layout);
                    for (auto* param : ctor->GetParameters())
                    {
                        walkStack.push({ param, false });
                    }
                }
                break;
                case Layout::FieldLayoutType:
                {
                    auto* field = cast<FieldLayout>(layout);
                    walkStack.push({ field->GetType(), false });
                }
                break;
                case Layout::ParameterLayoutType:
                {
                    auto* param = cast<ParameterLayout>(layout);
                    walkStack.push({ param->GetType(), false });
                }
                break;
                case Layout::PointerLayoutType:
                {
                    auto* ptr = cast<PointerLayout>(layout);
                    walkStack.push({ ptr->GetElementType(), false });
                }
                break;
                case Layout::PrimitiveLayoutType:
                    // Nothing to do
                    break;
                default:
                    HXSL_ASSERT(false, "Unknown layout type in module writer");
                    break;
                }
            }

            auto recordCount = static_cast<uint64_t>(sorted.size());
            stream->Write(recordCount);
            for (auto& layout : sorted)
            {
                switch (layout->GetTypeId())
                {
                case Layout::ModuleLayoutType:
                    WriteModule(cast<Module>(layout));
                    break;
                case Layout::NamespaceLayoutType:
                    WriteNamespace(cast<NamespaceLayout>(layout));
                    break;
                case Layout::PrimitiveLayoutType:
                    WritePrimitiveType(cast<PrimitiveLayout>(layout));
                    break;
                case Layout::PointerLayoutType:
                    WritePointerType(cast<PointerLayout>(layout));
                    break;
                case Layout::StructLayoutType:
                    WriteStruct(cast<StructLayout>(layout));
                    break;
                case Layout::FunctionLayoutType:
                    WriteFunction(cast<FunctionLayout>(layout));
                    break;
                case Layout::OperatorLayoutType:
                    WriteOperator(cast<OperatorLayout>(layout));
                    break;
                case Layout::ConstructorLayoutType:
                    WriteConstructor(cast<ConstructorLayout>(layout));
                    break;
                case Layout::ParameterLayoutType:
                    WriteParameter(cast<ParameterLayout>(layout));
                    break;
                case Layout::FieldLayoutType:
                    WriteField(cast<FieldLayout>(layout));
                    break;
                default:
                    HXSL_ASSERT(false, "Unknown layout type in module writer");
                    break;
                }
            }
        }

        template<typename T>
        static inline T ReadLittleEndian(Stream* stream)
        {
            return EndianUtils::FromLittleEndian(stream->ReadValue<T>());
        }

        ModuleReader::RecordId ModuleReader::ReadRecordRef()
        {
            return ReadLittleEndian<RecordId>();
        }

        uptr<Module> ModuleReader::Read()
        {
            auto mod = make_uptr<Module>();
            module = mod.get();

            auto recordCount = ReadLittleEndian<uint64_t>();
            std::vector<Layout*> records;
            records.reserve(recordCount);

			std::vector<FunctionLayout*> functions;
            for (uint64_t i = 0; i < recordCount; ++i)
            {
                Layout::LayoutType typeId = ReadLittleEndian<Layout::LayoutType>();
                RecordId id = ReadLittleEndian<RecordId>();

                Layout* layout = nullptr;

                switch (typeId)
                {
                case Layout::ModuleLayoutType:
                    ReadModule();
                    continue;
                case Layout::NamespaceLayoutType:
                    layout = ReadNamespace();
                    break;
                case Layout::PrimitiveLayoutType:
                    layout = ReadPrimitiveType();
                    break;
                case Layout::PointerLayoutType:
                    layout = ReadPointerType();
                    break;
                case Layout::StructLayoutType:
                    layout = ReadStruct();
                    break;
                case Layout::FunctionLayoutType:
                {
                    auto function = ReadFunction();
                    functions.push_back(function);
                    layout = function;
                }
                    break;
                case Layout::OperatorLayoutType:
                {
                    auto op = ReadOperator();
                    functions.push_back(op);
                    layout = op;
                }
                    break;
                case Layout::ConstructorLayoutType:
                {
                    auto ctor = ReadConstructor();
                    functions.push_back(ctor);
                    layout = ctor;
                }
                    break;
                case Layout::ParameterLayoutType:
                    layout = ReadParameter();
                    break;
                case Layout::FieldLayoutType:
                    layout = ReadField();
                    break;
                default:
                    HXSL_ASSERT(false, "Unknown layout type in module reader");
                    break;
                }

                recordMap.insert({ id, layout });
                records.push_back(layout);
            }

			auto& alloc = mod->GetAllocator();
            mod->SetAllFunctions(alloc.CopySpan(functions));

            return mod;
        }

        void ModuleReader::ReadModule()
        {
            auto nsCount = ReadLittleEndian<uint32_t>();
            for (uint32_t i = 0; i < nsCount; ++i)
            {
                module->AddNamespace(ReadRecordRef<NamespaceLayout>());
            }
        }

        NamespaceLayout* ModuleReader::ReadNamespace()
        {
            StringSpan name = ReadStringSpan();

            NamespaceLayoutBuilder builder(*module);
            builder.Name(name);

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
            AccessModifier access = ReadLittleEndian<AccessModifier>();
            PrimitiveKind kind = ReadLittleEndian<PrimitiveKind>();
            PrimitiveClass primClass = ReadLittleEndian<PrimitiveClass>();
            uint32_t rows = ReadLittleEndian<uint32_t>();
            uint32_t columns = ReadLittleEndian<uint32_t>();

            PrimitiveLayoutBuilder builder(*module);
            return builder.Name(name)
                .Access(access)
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

        
