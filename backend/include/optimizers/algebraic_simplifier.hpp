#ifndef ALGEBRAIC_SIMPLIFIER_HPP
#define ALGEBRAIC_SIMPLIFIER_HPP

#include "il_optimizer_pass.hpp"

namespace HXSL
{
	namespace Backend
	{
	class AlgebraicSimplifier : public ILOptimizerPass, CFGVisitor<EmptyCFGContext>
	{
		dense_map<ILVarId, ResultInstr*> definitions;

		void Visit(size_t index, BasicBlock& node, EmptyCFGContext& context) override;

	public:
		AlgebraicSimplifier(ILContext* context) : ILOptimizerPass(context), CFGVisitor(context->GetCFG())
		{
		}

		std::string GetName() override { return "AlgebraicSimplifier"; }

		OptimizerPassResult Run() override
		{
			changed = false;
			definitions.clear();
			Traverse();
			return changed ? OptimizerPassResult_Rerun : OptimizerPassResult_None;
		}
	};
	}
}

#endif