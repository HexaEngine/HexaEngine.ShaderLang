#ifndef IL_MUTATOR_BASE_HPP
#define IL_MUTATOR_BASE_HPP

#include "il_builder.hpp"
#include "il/control_flow_graph.hpp"

namespace HXSL
{
	class ILMutatorBase
	{
	protected:
		ILBuilder& builder;
		std::vector<size_t> discardList;
		ILMutatorBase(ILBuilder& builder) : builder(builder) {}

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

		void DiscardInstr(size_t index)
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