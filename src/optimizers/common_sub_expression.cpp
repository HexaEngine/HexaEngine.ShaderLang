#include "common_sub_expression.hpp"

namespace HXSL
{
	void CommonSubExpression::TryMapOperand(Operand*& op)
	{
		if (auto var = dyn_cast<Variable>(op))
		{
			auto it = map.find(var->varId);
			if (it != map.end())
			{
				op = context->MakeVariable(it->second); changed = true;
			}
		}
	}

	void CommonSubExpression::Visit(size_t index, CFGNode& node, EmptyCFGContext& context)
	{
		auto& instructions = node.instructions;
		for (auto& instr : instructions)
		{
			TryMapOperand(instr.operandLeft);
			TryMapOperand(instr.operandRight);

			if (instr.opcode == OpCode_Load || instr.opcode == OpCode_Move || instr.opcode == OpCode_Store || instr.opcode == OpCode_StoreParam || instr.opcode == OpCode_LoadParam) continue;

			auto it = subExpressions.insert(&instr);
			if (!it.second)
			{
				DiscardInstr(instr);
				map.insert({ cast<Variable>(instr.operandResult)->varId, cast<Variable>((*it.first)->operandResult)->varId });
			}
		}

		DiscardMarkedInstructs(node);
	}
}