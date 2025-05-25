#include "constant_folder.hpp"
#include "il/il_helper.hpp"

namespace HXSL
{
	static bool IsJumpCondition(std::vector<ILInstruction>& instructions, size_t i)
	{
		if (i + 1 >= instructions.size())
		{
			return false;
		}

		auto nextInstr = instructions[i + 1].opcode;
		return (nextInstr == OpCode_Jump || nextInstr == OpCode_JumpNotZero || nextInstr == OpCode_JumpZero);
	}

	void ConstantFolder::TryFoldOperand(ILOperand& op)
	{
		if (op.IsVar())
		{
			auto it = constants.find(op.varId);
			if (it != constants.end())
			{
				op = it->second; changed = true;
			}
			auto itr = varToVar.find(op.varId);
			if (itr != varToVar.end())
			{
				op = itr->second; changed = true;
			}
		}
	}

	void ConstantFolder::Visit(size_t index, CFGNode& node, EmptyCFGContext& context)
	{
		auto& instructions = node.instructions;
		for (size_t i = 0; i < instructions.size(); i++)
		{
			auto& instr = instructions[i];

			TryFoldOperand(instr.operandLeft);
			TryFoldOperand(instr.operandRight);

			switch (instr.opcode)
			{
			case OpCode_Move:
				if (instr.operandLeft.IsVar() && instr.operandResult.IsVar())
				{
					auto it = constants.find(instr.operandLeft.varId);
					if (it != constants.end())
					{
						constants.insert({ instr.operandResult.varId, it->second });
					}
					else
					{
						varToVar.insert({ instr.operandResult.varId, instr.operandLeft.varId });
					}
				}
				else if (instr.operandLeft.IsImm() && instr.operandResult.IsVar())
				{
					constants.insert({ instr.operandResult.varId, instr.operandLeft.imm() });
				}
				break;
			case OpCode_Cast:
				if (instr.operandLeft.IsImm() && instr.operandResult.IsVar())
				{
					instr.opcode = OpCode_Move;
					instr.operandLeft = Cast(instr.operandLeft.imm(), instr.opKind);
					constants.insert({ instr.operandResult.varId, instr.operandLeft.imm() });
				}
				break;
			case OpCode_LogicalNot:
			case OpCode_BitwiseNot:
			case OpCode_Negate:
				if (instr.operandLeft.IsImm())
				{
					Number imm = FoldImm(instr.operandLeft.imm(), {}, instr.opcode);
					if (IsJumpCondition(instructions, i))
					{
						break;
					}
					constants.insert({ instr.operandResult.varId, imm });
				}
				break;
			case OpCode_Store:
				break;
			default:
				if (instr.operandLeft.IsImm() && instr.operandRight.IsImm())
				{
					Number imm;
					if (TryFold(instr, imm))
					{
						if (IsJumpCondition(instructions, i))
						{
							break;
						}
						constants.insert({ instr.operandResult.varId, imm });
						instr.opcode = OpCode_Move;
						instr.operandLeft = imm;
						instr.operandRight = {};
					}
				}
				break;
			}
		}

		DiscardMarkedInstructs(node);
		constants.clear();
		varToVar.clear();

		std::unordered_map<ILVarId, size_t> defMap;
		for (size_t i = 0; i < instructions.size(); i++)
		{
			auto& instr = instructions[i];

			if (IsBinaryOp(instr.opcode))
			{
				bool varImm = instr.IsVarImm();
				bool immVar = instr.IsImmVar();
				bool isCommutative = (instr.IsOp(OpCode_Add) || instr.IsOp(OpCode_Multiply));

				if (varImm || immVar)
				{
					ILVarId lhs = varImm ? instr.operandLeft.varId : instr.operandRight.varId;
					Number rhs = varImm ? instr.operandRight.imm() : instr.operandLeft.imm();
					auto defIt = defMap.find(lhs);
					if (defIt != defMap.end())
					{
						auto& defInstr = instructions[defIt->second];

						auto isMulDivCandidate0 = defInstr.IsOp(OpCode_Multiply) && instr.IsOp(OpCode_Divide);
						auto isMulDivCandidate1 = defInstr.IsOp(OpCode_Divide) && instr.IsOp(OpCode_Multiply);
						bool fuseMulDiv = isMulDivCandidate0 || isMulDivCandidate1;

						if (defInstr.opcode == instr.opcode || fuseMulDiv)
						{
							bool defRegImm = defInstr.operandLeft.IsVar() && defInstr.operandRight.IsImm();
							bool defImmReg = (isCommutative || fuseMulDiv) && defInstr.operandLeft.IsImm() && defInstr.operandRight.IsVar();

							ILVarId base;
							Number lhs;
							if (defRegImm)
							{
								base = defInstr.operandLeft.varId;
								lhs = defInstr.operandRight.imm();
							}
							else if (defImmReg)
							{
								base = defInstr.operandRight.varId;
								lhs = defInstr.operandLeft.imm();
							}
							else
							{
								defMap[instr.operandResult.varId] = i;
								continue;
							}

							if (isMulDivCandidate1)
							{
								std::swap(lhs, rhs);
							}

							Number total = FoldImm(lhs, rhs, fuseMulDiv ? OpCode_Divide : instr.opcode);
							DiscardInstr(defIt->second);
							instr.operandLeft.varId = base;
							instr.operandRight.imm() = total;
							instr.opcode = fuseMulDiv ? OpCode_Multiply : defInstr.opcode;
							continue;
						}
					}
				}
			}

			if (instr.operandResult.IsVar())
			{
				defMap[instr.operandResult.varId] = i;
			}
		}

		DiscardMarkedInstructs(node);
	}
}