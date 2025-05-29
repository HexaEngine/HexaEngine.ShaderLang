#ifndef COMMON_SUB_EXPRESSION_HPP
#define COMMON_SUB_EXPRESSION_HPP

#include "il_optimizer_pass.hpp"
#include "utils/dense_map.hpp"

namespace HXSL
{
	class CommonSubExpression : public ILOptimizerPass, CFGVisitor<EmptyCFGContext>
	{
		std::unordered_set<ResultInstr*, InstructionPtrHash, InstructionPtrEquals> subExpressions;
		dense_map<ILVarId, ILVarId> map;

		void TryMapOperand(Operand*& op);

		void Visit(size_t index, BasicBlock& node, EmptyCFGContext& context) override;

	public:
		CommonSubExpression(ILContext* context) : ILOptimizerPass(context), CFGVisitor(context->GetCFG())
		{
		}

		std::string GetName() override { return "CommonSubExpression"; }

		OptimizerPassResult Run() override
		{
			changed = false;
			subExpressions.clear();
			map.clear();
			Traverse();
			return changed ? OptimizerPassResult_Changed : OptimizerPassResult_None;
		}
	};
}

#endif