#ifndef COMMON_SUB_EXPRESSION_HPP
#define COMMON_SUB_EXPRESSION_HPP

#include "il_optimizer_pass.hpp"

namespace HXSL
{
	class CommonSubExpression : public ILOptimizerPass, CFGVisitor<EmptyCFGContext>
	{
		std::unordered_set<ILInstruction> subExpressions;
		std::unordered_map<ILOperand, ILOperand> map;

		void TryMapOperand(ILOperand& op);

		void Visit(size_t index, CFGNode& node, EmptyCFGContext& context) override;

	public:
		CommonSubExpression(ILMetadata& metadata, ControlFlowGraph& cfg) : ILOptimizerPass(metadata), CFGVisitor(cfg)
		{
		}

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