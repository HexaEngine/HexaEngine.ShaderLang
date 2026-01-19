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
			auto p = stream->Position();
			metadata.Write(stream, context);
			auto p2 = stream->Position();
			stream->WriteValue(EndianUtils::ToLittleEndian(static_cast<uint32_t>(instructions.size())));
			ILWriterOptions options = { false, metadata };
			ILWriter writer(stream, options);
			auto p3 = stream->Position();
			for (auto& instr : instructions)
			{
				writer.Write(instr);
			}
			return;
		}

		void ILCodeBlob::Read(Stream* stream, ModuleReaderContext& context)
		{
			metadata.Read(stream, context);
			auto instrCount = EndianUtils::FromLittleEndian(stream->ReadValue<uint32_t>());
			ILReaderOptions options = { allocator, metadata, false };
			ILReader reader(stream, options);

			for (uint32_t i = 0; i < instrCount; ++i)
			{
				auto instr = reader.Read();
				instructions.append_move(instr);
			}		
			return;
		}
	}
}