#ifndef IL_MUTATOR_BASE_HPP
#define IL_MUTATOR_BASE_HPP

#include "il_metadata.hpp"
#include "control_flow_graph.hpp"

namespace HXSL
{
	class ILMutatorBase
	{
	protected:
		ILMetadata& metadata;
		std::vector<size_t> discardList;
		ILMutatorBase(ILMetadata& metadata) : metadata(metadata) {}

	public:
		void DiscardMarkedInstructs(CFGNode& node)
		{
			auto& container = node.instructions;
			size_t toDiscard = discardList.size();
			size_t discardIndex = 0;
			size_t writeIndex = 0;
			for (size_t i = 0; i < container.size(); i++)
			{
				auto& instr = container[i];
				if (discardIndex < toDiscard && discardList[discardIndex] == i)
				{
					discardIndex++;
					continue;
				}

				container[writeIndex] = std::move(instr);
				writeIndex++;
			}
			container.resize(writeIndex);

			discardList.clear();
		}

		virtual void DiscardInstr(size_t index)
		{
			auto it = std::lower_bound(discardList.begin(), discardList.end(), index);
			if (it == discardList.end() || *it != index)
			{
				discardList.insert(it, index);
			}
		}
	};
}

#endif