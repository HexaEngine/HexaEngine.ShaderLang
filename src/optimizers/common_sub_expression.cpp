#include "common_sub_expression.hpp"

namespace HXSL
{
	void CommonSubExpression::TryMapOperand(ILOperand& op)
	{
		if (op.IsVar())
		{
			auto it = map.find(op);
			if (it != map.end())
			{
				op = it->second; changed = true;
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

			auto it = subExpressions.find(instr);
			if (it != subExpressions.end())
			{
				DiscardInstr(instr);
				map.insert({ instr.operandResult, it->operandResult });
			}
			else
			{
				subExpressions.insert(instr);
			}
		}

		DiscardMarkedInstructs(node);
	}
}