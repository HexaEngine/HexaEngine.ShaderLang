#include "il/il_metadata.hpp"
#include "io/stream.hpp"
#include "core/module_reader.hpp"
#include "core/module_writer.hpp"
#include "il/il_encoding.hpp"

namespace HXSL
{
	namespace Backend
	{
		static void WriteVar(Stream* stream, const ILVariable& var)
		{
			ILWriter::EncodeVarId(stream, var.id);
			stream->WriteLEB128(var.typeId->id);
			stream->WriteLEB128(var.flags);
		}

		void ILMetadata::Write(Stream* stream, ModuleWriterContext& context)
		{
			auto& writer = context.writer;
			stream->WriteLEB128(static_cast<uint32_t>(typeMetadata.size()));
			for (auto& type : typeMetadata)
			{
				stream->WriteLEB128(type->id);
				writer.WriteRecordRef(type->def);
			}

			stream->WriteLEB128(static_cast<uint32_t>(variables.size()));
			for (auto& var : variables)
			{
				WriteVar(stream, var);
			}

			stream->WriteLEB128(static_cast<uint32_t>(tempVariables.size()));
			for (auto& var : tempVariables)
			{
				WriteVar(stream, var);
			}

			stream->WriteLEB128(static_cast<uint32_t>(functions.size()));
			for (auto& func : functions)
			{
				stream->WriteLEB128(func->id);
				writer.WriteRecordRef(func->func);
			}
		}

		static ILVariable ReadVar(Stream* stream, std::vector<ILType> types)
		{
			auto varId = ILReader::DecodeVarId(stream);
			auto typeIdValue = stream->ReadLEB128<ILTypeMetadata::ILTypeId>();
			auto typeId = types[typeIdValue];
			auto flags = stream->ReadLEB128<ILVariableFlags>();
			return ILVariable(varId, typeId, flags);
		}

		template<typename T>
		using CoTask = TrampolineTask<T>;

		CoTask<void> ILMetadata::Read(Stream* stream, ModuleReader& reader)
		{
			auto typeCount = stream->ReadLEB128<uint32_t>();
			for (uint32_t i = 0; i < typeCount; ++i)
			{
				auto id = stream->ReadLEB128<ILTypeMetadata::ILTypeId>();
				auto def = co_await reader.ReadRecordRef<TypeLayout>();
				auto meta = allocator.Alloc<ILTypeMetadata>(id, def);
				typeMetadata.push_back(meta);
				typeMap.insert({ def, meta });
			}

			auto varCount = stream->ReadLEB128<uint32_t>();
			for (uint32_t i = 0; i < varCount; ++i)
			{
				variables.push_back(ReadVar(stream, typeMetadata));
			}

			auto tempVarCount = stream->ReadLEB128<uint32_t>();
			for (uint32_t i = 0; i < tempVarCount; ++i)
			{
				tempVariables.push_back(ReadVar(stream, typeMetadata));
			}

			auto funcCount = stream->ReadLEB128<uint32_t>();
			for (uint32_t i = 0; i < funcCount; ++i)
			{
				auto id = stream->ReadLEB128<ILFuncCallMetadata::ILFuncCallId>();
				auto funcDef = co_await reader.ReadRecordRef<FunctionLayout>();
				auto funcMeta = allocator.Alloc<ILFuncCallMetadata>(id, funcDef);
				functions.push_back(funcMeta);
				funcMap.insert({ funcDef, funcMeta });
			}
		}
	}
}