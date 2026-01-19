#include "il/il_metadata.hpp"
#include "io/stream.hpp"

namespace HXSL
{
	namespace Backend
	{
		static void WriteString(Stream* stream, const StringSpan& str)
		{
			uint32_t length = static_cast<uint32_t>(str.size());
			stream->Write(EndianUtils::ToLittleEndian(length));
			if (length > 0)
			{
				stream->Write(str.data(), length);
			}
		}

		static void WriteVar(ModuleWriter& writer, const ILVariable& var)
		{
			writer.WriteLittleEndian(var.id.raw);
			writer.WriteLittleEndian(var.typeId->id);
			writer.WriteLittleEndian(var.flags);
		}

		void ILMetadata::Write(Stream* stream, ModuleWriterContext& context)
		{
			auto& writer = context.writer;
			writer.WriteLittleEndian(static_cast<uint32_t>(typeMetadata.size()));
			for (auto& type : typeMetadata)
			{
				writer.WriteLittleEndian(type->id);
				writer.WriteRecordRef(type->def);
			}

			writer.WriteLittleEndian(static_cast<uint32_t>(variables.size()));
			for (auto& var : variables)
			{
				WriteVar(writer, var);
			}

			writer.WriteLittleEndian(static_cast<uint32_t>(tempVariables.size()));
			for (auto& var : tempVariables)
			{
				WriteVar(writer, var);
			}

			writer.WriteLittleEndian(static_cast<uint32_t>(functions.size()));
			for (auto& func : functions)
			{
				writer.WriteLittleEndian(func->id);
				writer.WriteRecordRef(func->func);
			}
		}

		static ILVariable ReadVar(ModuleReader& reader, std::vector<ILType> types)
		{
			auto varId = ILVarId(reader.ReadLittleEndian<uint64_t>());
			auto typeIdValue = reader.ReadLittleEndian<ILTypeMetadata::ILTypeId>();
			auto typeId = types[typeIdValue];
			auto flags = reader.ReadLittleEndian<ILVariableFlags>();
			return ILVariable(varId, typeId, flags);
		}

		void ILMetadata::Read(Stream* stream, ModuleReaderContext& context)
		{
			auto& reader = context.reader;

			auto typeCount = reader.ReadLittleEndian<uint32_t>();
			for (uint32_t i = 0; i < typeCount; ++i)
			{
				auto id = reader.ReadLittleEndian<ILTypeMetadata::ILTypeId>();
				auto def = reader.ReadRecordRef<TypeLayout>();
				auto meta = allocator.Alloc<ILTypeMetadata>(id, def);
				typeMetadata.push_back(meta);
				typeMap.insert({ def, meta });
			}

			auto varCount = reader.ReadLittleEndian<uint32_t>();
			for (uint32_t i = 0; i < varCount; ++i)
			{
				variables.push_back(ReadVar(reader, typeMetadata));
			}

			auto tempVarCount = reader.ReadLittleEndian<uint32_t>();
			for (uint32_t i = 0; i < tempVarCount; ++i)
			{
				tempVariables.push_back(ReadVar(reader, typeMetadata));
			}

			auto funcCount = reader.ReadLittleEndian<uint32_t>();
			for (uint32_t i = 0; i < funcCount; ++i)
			{
				auto id = reader.ReadLittleEndian<ILFuncCallMetadata::ILFuncCallId>();
				auto funcDef = reader.ReadRecordRef<FunctionLayout>();
				auto funcMeta = allocator.Alloc<ILFuncCallMetadata>(id, funcDef);
				functions.push_back(funcMeta);
				funcMap.insert({ funcDef, funcMeta });
			}
		}
	}
}