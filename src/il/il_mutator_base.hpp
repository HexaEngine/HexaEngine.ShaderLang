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
		std::unordered_set<ILInstruction*> discardList;
		ILMutatorBase(ILMetadata& metadata) : metadata(metadata) {}

	public:
		void DiscardMarkedInstructs(CFGNode& node)
		{
			for (auto instr : discardList)
			{
				node.instructions.remove(instr);
			}
			discardList.clear();
		}

		virtual void DiscardInstr(ILInstruction& instr)
		{
			discardList.insert(&instr);
		}
	};
}

#endif