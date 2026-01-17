#include "il/rpo_merger.hpp"

namespace HXSL
{
	namespace Backend
	{
		void RPOMerger::VisitClose(size_t index, BasicBlock& node, EmptyCFGContext& context)
		{
			blocksSorted.push_back(&node);
		}

		void RPOMerger::Merge(ILContainer& container, JumpTable& jumpTable)
		{
			TraverseDFS();

			std::reverse(blocksSorted.begin(), blocksSorted.end());
			
			container.clear();
			for (auto& block : blocksSorted)
			{
				auto& instructions = block->GetInstructions();
				if (instructions.empty())
				{
					continue;
				}
				jumpTable.SetLocation(ILLabel(block->GetId()), &instructions.front());
				container.append_move(instructions);
			}
		}
	}
}