#ifndef RPO_MERGER_HPP
#define RPO_MERGER_HPP

#include "pch/il.hpp"

namespace HXSL
{
	namespace Backend
	{
		class RPOMerger : CFGVisitor<>
		{
			std::vector<BasicBlock*> blocksSorted;

			void Visit(size_t index, BasicBlock& node, EmptyCFGContext& context) override {}

			void VisitClose(size_t index, BasicBlock& node, EmptyCFGContext& context) override;
		public:
			RPOMerger(ControlFlowGraph& cfg) : CFGVisitor(cfg) {}
			void Merge(ILContainer& container, JumpTable& jumpTable);
		};
	}
}

#endif