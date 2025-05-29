#ifndef STRENGTH_REDUCTION_HPP
#define STRENGTH_REDUCTION_HPP

#include "il_optimizer_pass.hpp"

namespace HXSL
{
	class StrengthReduction : public ILOptimizerPass, CFGVisitor<EmptyCFGContext>
	{
		bool changed = false;

		void Visit(size_t index, BasicBlock& node, EmptyCFGContext& context) override;

		void MulDivReduce(ILInstruction& instr);

	public:
		StrengthReduction(ILContext* context) : ILOptimizerPass(context), CFGVisitor(context->GetCFG())
		{
		}

		std::string GetName() override { return "StrengthReduction"; }

		OptimizerPassResult Run() override
		{
			changed = false;
			Traverse();
			return changed ? OptimizerPassResult_Changed : OptimizerPassResult_None;
		}
	};
}

#endif