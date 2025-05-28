#include "constant_folder.hpp"
#include "il/il_helper.hpp"

namespace HXSL
{
	static bool IsJumpCondition(ILInstruction& instruction)
	{
		if (!instruction.GetNext())
		{
			return false;
		}

		auto nextInstr = instruction.GetNext()->opcode;
		return (nextInstr == OpCode_Jump || nextInstr == OpCode_JumpNotZero || nextInstr == OpCode_JumpZero);
	}

	void ConstantFolder::TryFoldOperand(Operand*& op)
	{
		if (auto var = dyn_cast<Variable>(op))
		{
			auto it = constants.find(var->varId);
			if (it != constants.end())
			{
				op = context->MakeConstant(it->second); changed = true;
			}
			auto itr = varToVar.find(var->varId);
			if (itr != varToVar.end())
			{
				op = context->MakeVariable(itr->second); changed = true;
			}
		}
	}

	void ConstantFolder::Visit(size_t index, CFGNode& node, EmptyCFGContext& ctx)
	{
		auto& instructions = node.instructions;
		for (auto& instr : instructions)
		{
			TryFoldOperand(instr.operandLeft);
			TryFoldOperand(instr.operandRight);

			switch (instr.opcode)
			{
			case OpCode_Move:
			{
				auto varR = dyn_cast<Variable>(instr.operandResult);
				if (!varR) break;

				if (auto varL = dyn_cast<Variable>(instr.operandLeft))
				{
					auto it = constants.find(varL->varId);
					if (it != constants.end())
					{
						constants.insert({ varR->varId, it->second });
					}
					else
					{
						varToVar.insert({ varR->varId, varL->varId });
					}
				}
				else if (auto immL = dyn_cast<Constant>(instr.operandLeft))
				{
					constants.insert({ varR->varId, immL->imm() });
				}
			}
			break;
			case OpCode_Cast:
			{
				auto varR = dyn_cast<Variable>(instr.operandResult);
				if (!varR) break;
				if (auto immL = dyn_cast<Constant>(instr.operandLeft))
				{
					instr.opcode = OpCode_Move;
					instr.operandLeft = context->MakeConstant(Cast(immL->imm(), instr.opKind));
					constants.insert({ varR->varId, immL->imm() });
				}
			}

			break;
			case OpCode_LogicalNot:
			case OpCode_BitwiseNot:
			case OpCode_Negate:
			{
				auto varR = dyn_cast<Variable>(instr.operandResult);
				if (!varR) break;
				if (auto immL = dyn_cast<Constant>(instr.operandLeft))
				{
					Number imm = FoldImm(immL->imm(), {}, instr.opcode);
					if (IsJumpCondition(instr))
					{
						break;
					}
					constants.insert({ varR->varId, imm });
				}
			}
			break;
			case OpCode_Store:
				break;
			default:
			{
				auto varR = dyn_cast<Variable>(instr.operandResult);
				if (!varR) break;
				if (!isa<Constant>(instr.operandLeft) || !isa<Constant>(instr.operandRight)) break;

				Number imm;
				if (TryFold(instr, imm))
				{
					if (IsJumpCondition(instr))
					{
						break;
					}
					constants.insert({ varR->varId, imm });
					instr.opcode = OpCode_Move;
					instr.operandLeft = context->MakeConstant(imm);
					instr.operandRight = {};
				}
			}
			break;
			}
		}

		DiscardMarkedInstructs(node);
		constants.clear();
		varToVar.clear();

		std::unordered_map<ILVarId, ILInstruction*> defMap;
		for (auto& instr : instructions)
		{
			if (IsBinary(instr.opcode))
			{
				bool varImm = instr.IsVarImm();
				bool immVar = instr.IsImmVar();
				bool isCommutative = (instr.IsOp(OpCode_Add) || instr.IsOp(OpCode_Multiply));

				if (varImm || immVar)
				{
					ILVarId lhs = varImm ? cast<Variable>(instr.operandLeft)->varId : cast<Variable>(instr.operandRight)->varId;
					Number rhs = varImm ? cast<Constant>(instr.operandRight)->imm() : cast<Constant>(instr.operandLeft)->imm();
					auto defIt = defMap.find(lhs);
					if (defIt != defMap.end())
					{
						auto& defInstr = *defIt->second;

						auto isMulDivCandidate0 = defInstr.IsOp(OpCode_Multiply) && instr.IsOp(OpCode_Divide);
						auto isMulDivCandidate1 = defInstr.IsOp(OpCode_Divide) && instr.IsOp(OpCode_Multiply);
						bool fuseMulDiv = isMulDivCandidate0 || isMulDivCandidate1;

						if (defInstr.opcode == instr.opcode || fuseMulDiv)
						{
							bool defRegImm = Operand::IsVar(defInstr.operandLeft) && Operand::IsImm(defInstr.operandRight);
							bool defImmReg = (isCommutative || fuseMulDiv) && Operand::IsImm(defInstr.operandLeft) && Operand::IsVar(defInstr.operandRight);

							ILVarId base;
							Number lhs;
							if (defRegImm)
							{
								base = cast<Variable>(defInstr.operandLeft)->varId;
								lhs = cast<Constant>(defInstr.operandRight)->imm();
							}
							else if (defImmReg)
							{
								base = cast<Variable>(defInstr.operandRight)->varId;
								lhs = cast<Constant>(defInstr.operandLeft)->imm();
							}
							else
							{
								defMap[cast<Variable>(instr.operandResult)->varId] = &instr;
								continue;
							}

							if (isMulDivCandidate1)
							{
								std::swap(lhs, rhs);
							}

							Number total = FoldImm(lhs, rhs, fuseMulDiv ? OpCode_Divide : instr.opcode);
							//DiscardInstr(*defIt->second);
							instr.operandLeft = context->MakeVariable(base);
							instr.operandRight = context->MakeConstant(total);
							instr.opcode = fuseMulDiv ? OpCode_Multiply : defInstr.opcode;
							continue;
						}
					}
				}
			}

			if (auto var = dyn_cast<Variable>(instr.operandResult))
			{
				defMap[var->varId] = &instr;
			}
		}

		DiscardMarkedInstructs(node);
	}
}