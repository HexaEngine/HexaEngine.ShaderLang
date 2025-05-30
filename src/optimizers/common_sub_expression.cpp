#include "common_sub_expression.hpp"

namespace HXSL
{
	namespace Backend
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

		void CommonSubExpression::Visit(size_t index, BasicBlock& node, EmptyCFGContext& context)
		{
			for (auto& instr : node)
			{
				for (auto& operand : instr.GetOperands())
				{
					TryMapOperand(operand);
				}

				auto opcode = instr.GetOpCode();
				if (opcode == OpCode_Load || opcode == OpCode_Move || opcode == OpCode_Store || opcode == OpCode_StoreParam || opcode == OpCode_LoadParam) continue;

				if (auto res = dyn_cast<ResultInstr>(&instr))
				{
					auto it = subExpressions.insert(res);
					if (!it.second)
					{
						DiscardInstr(instr);
						map.insert({ res->GetResult(), (*it.first)->GetResult() });
					}
				}
			}

			DiscardMarkedInstructs(node);
		}
	}
}