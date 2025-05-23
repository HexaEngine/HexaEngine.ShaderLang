#ifndef STRENGTH_REDUCTION_HPP
#define STRENGTH_REDUCTION_HPP

#include "il_optimizer_pass.hpp"

namespace HXSL
{
	class StrengthReduction : public ILOptimizerPass, CFGVisitor<EmptyCFGContext>
	{
		bool changed = false;

		void Visit(size_t index, CFGNode& node, EmptyCFGContext& context) override;

		void MulDivReduce(ILInstruction& instr);

	public:
		StrengthReduction(ILMetadata& metadata, ControlFlowGraph& cfg) : ILOptimizerPass(metadata), CFGVisitor(cfg)
		{
		}

		OptimizerPassResult Run() override
		{
			changed = false;
			Traverse();
			return changed ? OptimizerPassResult_Changed : OptimizerPassResult_None;
		}
	};
}

#endif