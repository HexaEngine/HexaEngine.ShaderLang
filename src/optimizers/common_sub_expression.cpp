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
		const size_t n = instructions.size();
		for (size_t i = 0; i < n; i++)
		{
			auto& instr = instructions[i];

			TryMapOperand(instr.operandLeft);
			TryMapOperand(instr.operandRight);

			if (instr.opcode == OpCode_Load || instr.opcode == OpCode_Move || instr.opcode == OpCode_Store || instr.opcode == OpCode_StoreParam || instr.opcode == OpCode_LoadParam) continue;

			auto it = subExpressions.find(instr);
			if (it != subExpressions.end())
			{
				DiscardInstr(i);
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