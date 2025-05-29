#include "constant_folder.hpp"
#include "il/il_helper.hpp"

namespace HXSL
{
	static bool IsJumpCondition(Instruction& instruction)
	{
		return isa<JumpInstr>(instruction.GetNext());
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

	void ConstantFolder::Visit(size_t index, BasicBlock& node, EmptyCFGContext& ctx)
	{
		for (auto& instr : node)
		{
			for (Operand*& operand : instr.GetOperands())
			{
				TryFoldOperand(operand);
			}

			switch (instr.GetOpCode())
			{
			case OpCode_Move:
			{
				auto in = *cast<MoveInstr>(&instr);
				if (auto varL = dyn_cast<Variable>(in.GetSource()))
				{
					auto it = constants.find(varL->varId);
					if (it != constants.end())
					{
						constants.insert({ in.GetResult(), it->second });
					}
					else
					{
						varToVar.insert({ in.GetResult(), varL->varId });
					}
				}
				else if (auto immL = dyn_cast<Constant>(in.GetSource()))
				{
					constants.insert({ in.GetResult(), immL->imm() });
				}
			}
			break;
			case OpCode_Cast:
			{
				auto in = *cast<UnaryInstr>(&instr);
				if (auto immL = dyn_cast<Constant>(in.GetOperand()))
				{
					node.ReplaceInstrO<MoveInstr>(&instr, in.GetResult(), Cast(instr, in.GetResult(), immL->imm()));
					constants.insert({ in.GetResult(), immL->imm() });
				}
			}

			break;
			case OpCode_LogicalNot:
			case OpCode_BitwiseNot:
			case OpCode_Negate:
			{
				auto in = *cast<UnaryInstr>(&instr);
				if (auto immL = dyn_cast<Constant>(in.GetOperand()))
				{
					Number imm = FoldImm(immL->imm(), {}, in.GetOpCode());
					if (IsJumpCondition(instr))
					{
						break;
					}
					constants.insert({ in.GetResult(), imm });
				}
			}
			break;
			case OpCode_Store:
				break;
			default:
			{
				auto opCount = instr.OperandCount();
				if (opCount == 0) break;

				auto res = dyn_cast<ResultInstr>(&instr);
				if (!res) break;
				if (!isa<Constant>(instr.GetOperand(0))) break;
				if (opCount > 0)
				{
					if (!isa<Constant>(instr.GetOperand(1))) break;
				}

				Number imm;
				if (TryFold(instr, imm))
				{
					if (IsJumpCondition(instr))
					{
						break;
					}
					constants.insert({ res->GetResult(), imm });
					node.ReplaceInstrO<MoveInstr>(&instr, res->GetResult(), context->MakeConstant(imm));
				}
			}
			break;
			}
		}

		DiscardMarkedInstructs(node);
		constants.clear();
		varToVar.clear();

		std::unordered_map<ILVarId, BinaryInstr*> defMap;
		for (auto& instr : node)
		{
			if (auto bin = dyn_cast<BinaryInstr>(&instr))
			{
				auto& binary = *bin;
				bool varImm = binary.IsVarImm();
				bool immVar = binary.IsImmVar();
				bool isCommutative = (binary.IsOp(OpCode_Add) || binary.IsOp(OpCode_Multiply));

				if (varImm || immVar)
				{
					ILVarId lhs = varImm ? cast<Variable>(binary.GetLHS())->varId : cast<Variable>(binary.GetRHS())->varId;
					Number rhs = varImm ? cast<Constant>(binary.GetRHS())->imm() : cast<Constant>(binary.GetLHS())->imm();
					auto defIt = defMap.find(lhs);
					if (defIt != defMap.end())
					{
						auto& defInstr = *defIt->second;

						auto isMulDivCandidate0 = defInstr.IsOp(OpCode_Multiply) && binary.IsOp(OpCode_Divide);
						auto isMulDivCandidate1 = defInstr.IsOp(OpCode_Divide) && binary.IsOp(OpCode_Multiply);
						bool fuseMulDiv = isMulDivCandidate0 || isMulDivCandidate1;

						if (defInstr.GetOpCode() == binary.GetOpCode() || fuseMulDiv)
						{
							bool defRegImm = Operand::IsVar(defInstr.GetLHS()) && Operand::IsImm(defInstr.GetRHS());
							bool defImmReg = (isCommutative || fuseMulDiv) && Operand::IsImm(defInstr.GetLHS()) && Operand::IsVar(defInstr.GetRHS());

							ILVarId base;
							Number lhs;
							if (defRegImm)
							{
								base = cast<Variable>(defInstr.GetLHS())->varId;
								lhs = cast<Constant>(defInstr.GetRHS())->imm();
							}
							else if (defImmReg)
							{
								base = cast<Variable>(defInstr.GetRHS())->varId;
								lhs = cast<Constant>(defInstr.GetLHS())->imm();
							}
							else
							{
								defMap[binary.GetResult()] = bin;
								continue;
							}

							if (isMulDivCandidate1)
							{
								std::swap(lhs, rhs);
							}

							Number total = FoldImm(lhs, rhs, fuseMulDiv ? OpCode_Divide : binary.GetOpCode());
							auto opcode = fuseMulDiv ? OpCode_Multiply : defInstr.GetOpCode();
							node.ReplaceInstr<BinaryInstr>(&instr, opcode, binary.GetResult(), base, total);
							continue;
						}
					}
				}

				defMap[binary.GetResult()] = bin;
			}
		}

		DiscardMarkedInstructs(node);
	}
}