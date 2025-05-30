#ifndef IL_MUTATOR_BASE_HPP
#define IL_MUTATOR_BASE_HPP

#include "il_metadata.hpp"
#include "control_flow_graph.hpp"

namespace HXSL
{
	namespace Backend
	{
		class ILMutatorBase
		{
		protected:
			ILMetadata& metadata;
			std::unordered_set<Instruction*> discardList;
			ILMutatorBase(ILMetadata& metadata) : metadata(metadata) {}

		public:
			void DiscardMarkedInstructs(BasicBlock& node)
			{
				for (auto instr : discardList)
				{
					node.RemoveInstr(instr);
				}
				discardList.clear();
			}

			virtual void DiscardInstr(Instruction& instr)
			{
				discardList.insert(&instr);
			}
		};
	}
}

#endif