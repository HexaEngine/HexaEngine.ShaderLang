#include "il/il_code_blob.hpp"
#include "il/rpo_merger.hpp"
#include "il/il_encoding.hpp"

namespace HXSL
{
	namespace Backend
	{
		void ILCodeBlob::FromContext(ILContext* context)
		{
			auto& cfg = context->GetCFG();
			auto& ctxMetadata = context->GetMetadata();
			RPOMerger rpoMerger = RPOMerger(cfg);
			jumpTable.Resize(cfg.size());
			rpoMerger.Merge(allocator, ctxMetadata, metadata, instructions, jumpTable);
		}

		void ILCodeBlob::Print()
		{
			std::cout << "{" << std::endl;
			for (auto& instr : instructions)
			{
				size_t offset = 0;
				while (true)
				{
					auto it = std::find(jumpTable.locations.begin() + offset, jumpTable.locations.end(), &instr);
					if (it == jumpTable.locations.end())
					{
						break;
					}
					auto index = std::distance(jumpTable.locations.begin(), it);
					std::cout << "loc_" << index << ":" << std::endl;
					offset = index + 1;
				}
				std::cout << "    " << ToString(instr, metadata) << std::endl;
			}
			std::cout << "}" << std::endl;
		}

		void ILCodeBlob::Write(Stream* stream, ModuleWriterContext& context)
		{
			metadata.Write(stream, context);
			stream->WriteValue(EndianUtils::ToLittleEndian(static_cast<uint32_t>(instructions.size())));
			ILWriterOptions options = { false, metadata };
			ILWriter writer(stream, options);
			dense_map<const Instruction*, uint32_t> instrMap;
			uint32_t instrIndex = 0;
			for (auto& instr : instructions)
			{
				instrMap[&instr] = instrIndex++;
				writer.Write(instr);
			}

			auto& labels = jumpTable.locations;
			stream->WriteValue(EndianUtils::ToLittleEndian(static_cast<uint32_t>(labels.size())));
			for (auto& label : labels)
			{
				uint32_t locIndex = instrMap[label];
				stream->WriteValue(EndianUtils::ToLittleEndian(locIndex));
			}
		}

		void ILCodeBlob::Read(Stream* stream, ModuleReaderContext& context)
		{
			metadata.Read(stream, context);
			auto instrCount = EndianUtils::FromLittleEndian(stream->ReadValue<uint32_t>());
			ILReaderOptions options = { allocator, metadata, false };
			ILReader reader(stream, options);

			std::vector<Instruction*> instructionsArray;
			for (uint32_t i = 0; i < instrCount; ++i)
			{
				auto instr = reader.Read();
				instructions.append_move(instr);
				instructionsArray.push_back(instr);
			}

			auto labelCount = EndianUtils::FromLittleEndian(stream->ReadValue<uint32_t>());
			jumpTable.Resize(labelCount);
			for (uint32_t i = 0; i < labelCount; ++i)
			{
				auto locIndex = EndianUtils::FromLittleEndian(stream->ReadValue<uint32_t>());
				jumpTable.locations[i] = instructionsArray[static_cast<size_t>(locIndex)];
			}
		}
	}
}