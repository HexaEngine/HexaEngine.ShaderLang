#include "optimizers/dead_code_eliminator.hpp"

namespace HXSL
{
	namespace Backend
	{
		void DeadCodeEliminator::ProcessOperand(Operand* op)
		{
			if (Operand::IsVar(op))
			{
				usedVars.insert(cast<Variable>(op)->varId);
			}
		}

		void DeadCodeEliminator::ProcessInstr(Instruction& instr, bool protectedInstr)
		{
			for (auto& operand : instr.GetOperands())
			{
				ProcessOperand(operand);
			}

			if (protectedInstr) return;
			if (auto res = dyn_cast<ResultInstr>(&instr))
			{
				if (usedVars.find(res->GetResult()) == usedVars.end())
				{
					deadVars.insert(res->GetResult());
					DiscardInstr(instr);
				}
			}
		}

		void DeadCodeEliminator::VisitClose(size_t index, BasicBlock& node, EmptyCFGContext& context)
		{
			bool protectedInstr = false;
			for (auto it = node.rbegin(); it != node.rend(); ++it)
			{
				auto& instr = *it;
				auto opcode = instr.GetOpCode();
				protectedInstr |= opcode == OpCode_Store;

				ProcessInstr(instr, protectedInstr);
				if (opcode == OpCode_JumpZero || opcode == OpCode_JumpNotZero)
				{
					protectedInstr = true;
				}
				else
				{
					protectedInstr = false;
				}
			}

			DiscardMarkedInstructs(node);
		}

		OptimizerPassResult DeadCodeEliminator::Run()
		{
			changed = false;
			usedVars.clear();

			for (auto& phi : metadata.phiNodes)
			{
				for (auto& p : phi->GetOperands())
				{
					usedVars.insert(cast<Variable>(p)->varId);
				}
			}

			Traverse();
			return changed ? OptimizerPassResult_Changed : OptimizerPassResult_None;
		}
	}
}