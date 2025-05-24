#ifndef ALGEBRAIC_SIMPLIFIER_HPP
#define ALGEBRAIC_SIMPLIFIER_HPP

#include "il_optimizer_pass.hpp"

namespace HXSL
{
	class AlgebraicSimplifier : public ILOptimizerPass, CFGVisitor<EmptyCFGContext>
	{
		void Visit(size_t index, CFGNode& node, EmptyCFGContext& context) override;

	public:
		AlgebraicSimplifier(ILMetadata& metadata, ControlFlowGraph& graph) : ILOptimizerPass(metadata), CFGVisitor(graph)
		{
		}

		OptimizerPassResult Run() override
		{
			changed = false;
			Traverse();
			return changed ? OptimizerPassResult_Rerun : OptimizerPassResult_None;
		}
	};
}

#endif