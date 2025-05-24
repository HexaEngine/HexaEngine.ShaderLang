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
		if (op.IsReg())
		{
			auto it = constants.find(op.reg);
			if (it != constants.end())
			{
				op = it->second; changed = true;
			}
			auto itr = registerToRegister.find(op.reg);
			if (itr != registerToRegister.end())
			{
				op = itr->second; changed = true;
			}
		}
		if (op.IsVar())
		{
			auto it = varConstants.find(op.varId);
			if (it != varConstants.end())
			{
				op = it->second; changed = true;
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
				if (instr.operandLeft.IsImm() && instr.operandResult.IsReg())
				{
					constants.insert({ instr.operandResult.reg, instr.operandLeft.imm() });
					//DiscardInstr(i);
				}
				else if (instr.operandLeft.IsReg() && instr.operandResult.IsReg())
				{
					auto it = constants.find(instr.operandLeft.reg);
					if (it != constants.end())
					{
						constants.insert({ instr.operandResult.reg, it->second });
					}
					else
					{
						registerToRegister.insert({ instr.operandResult.reg, instr.operandLeft.reg });
					}
					//DiscardInstr(i);
				}
				else if (instr.operandLeft.IsImm() && instr.operandResult.IsVar())
				{
					varConstants.insert({ instr.operandResult.varId, instr.operandLeft.imm() });
					//DiscardInstr(i);
				}
				break;
			case OpCode_Cast:
				if (instr.operandLeft.IsImm() && instr.operandResult.IsReg())
				{
					instr.opcode = OpCode_Move;
					instr.operandLeft = Cast(instr.operandLeft.imm(), instr.opKind);
					constants.insert({ instr.operandResult.reg, instr.operandLeft.imm() });
					//DiscardInstr(i);
				}
				if (instr.operandLeft.IsImm() && instr.operandResult.IsVar())
				{
					instr.opcode = OpCode_Move;
					instr.operandLeft = Cast(instr.operandLeft.imm(), instr.opKind);
					varConstants.insert({ instr.operandResult.varId, instr.operandLeft.imm() });
					//DiscardInstr(i);
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
					constants.insert({ instr.operandResult.reg, imm });
					//DiscardInstr(i);
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
						if (instr.operandResult.IsVar())
						{
							varConstants.insert({ instr.operandResult.varId, imm });
						}
						else
						{
							constants.insert({ instr.operandResult.reg, imm });
						}
						instr.opcode = OpCode_Move;
						instr.operandLeft = imm;
						instr.operandRight = {};
						//DiscardInstr(i);
					}
				}
				break;
			}
		}

		DiscardMarkedInstructs(node);
		constants.clear();
		registerToRegister.clear();

		std::unordered_map<ILRegister, size_t> defMap;
		for (size_t i = 0; i < instructions.size(); i++)
		{
			auto& instr = instructions[i];

			if (IsBinaryOp(instr.opcode))
			{
				bool regImm = instr.IsRegImm();
				bool immReg = instr.IsImmReg();
				bool isCommutative = (instr.IsOp(OpCode_Add) || instr.IsOp(OpCode_Multiply));

				if (regImm || immReg)
				{
					ILRegister lhs = regImm ? instr.operandLeft.reg : instr.operandRight.reg;
					Number rhs = regImm ? instr.operandRight.imm() : instr.operandLeft.imm();
					auto defIt = defMap.find(lhs);
					if (defIt != defMap.end())
					{
						auto& defInstr = instructions[defIt->second];

						auto isMulDivCandidate0 = defInstr.IsOp(OpCode_Multiply) && instr.IsOp(OpCode_Divide);
						auto isMulDivCandidate1 = defInstr.IsOp(OpCode_Divide) && instr.IsOp(OpCode_Multiply);
						bool fuseMulDiv = isMulDivCandidate0 || isMulDivCandidate1;

						if (defInstr.opcode == instr.opcode || fuseMulDiv)
						{
							bool defRegImm = defInstr.operandLeft.IsReg() && defInstr.operandRight.IsImm();
							bool defImmReg = (isCommutative || fuseMulDiv) && defInstr.operandLeft.IsImm() && defInstr.operandRight.IsReg();

							ILRegister base;
							Number lhs;
							if (defRegImm)
							{
								base = defInstr.operandLeft.reg;
								lhs = defInstr.operandRight.imm();
							}
							else if (defImmReg)
							{
								base = defInstr.operandRight.reg;
								lhs = defInstr.operandLeft.imm();
							}
							else
							{
								defMap[instr.operandResult.reg] = i;
								continue;
							}

							if (isMulDivCandidate1)
							{
								std::swap(lhs, rhs);
							}

							Number total = FoldImm(lhs, rhs, fuseMulDiv ? OpCode_Divide : instr.opcode);
							DiscardInstr(defIt->second);
							instr.operandLeft.reg = base;
							instr.operandRight.imm() = total;
							instr.opcode = fuseMulDiv ? OpCode_Multiply : defInstr.opcode;
							continue;
						}
					}
				}
			}

			if (instr.operandResult.IsReg())
			{
				defMap[instr.operandResult.reg] = i;
			}
		}

		DiscardMarkedInstructs(node);
	}
}