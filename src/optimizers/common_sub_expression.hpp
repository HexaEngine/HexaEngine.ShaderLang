#ifndef COMMON_SUB_EXPRESSION_HPP
#define COMMON_SUB_EXPRESSION_HPP

#include "pch/il.hpp"

namespace HXSL
{
	class CommonSubExpression : public CFGVisitor<EmptyCFGContext>, ILMutatorBase
	{
		std::unordered_set<ILInstruction> subExpressions;
		std::unordered_map<ILOperand, ILOperand> map;

		void TryMapOperand(ILOperand& op);

		void Visit(size_t index, CFGNode& node, EmptyCFGContext& context);

	public:
		CommonSubExpression(ControlFlowGraph& cfg, ILMetadata& metadata) : ILMutatorBase(metadata), CFGVisitor(cfg)
		{
		}
	};
}

#endif