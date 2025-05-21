#include "algebraic_simplifier.hpp"
#include "il/il_helper.hpp"

namespace HXSL
{
	static bool IsZero(const ILOperand& op)
	{
		if (!op.IsImm()) return false;

		auto& imm = op.imm;
		switch (imm.Kind)
		{
		case NumberType_Int8: return imm.i8 == 0;
		case NumberType_Int16: return imm.i16 == 0;
		case NumberType_Int32: return imm.i32 == 0;
		case NumberType_Int64: return imm.i64 == 0;
		case NumberType_UInt8: return imm.u8 == 0;
		case NumberType_UInt16: return imm.u16 == 0;
		case NumberType_UInt32: return imm.u32 == 0;
		case NumberType_UInt64: return imm.u64 == 0;
		case NumberType_Half: return imm.half_ == 0.0f;
		case NumberType_Float: return imm.float_ == 0.0f;
		case NumberType_Double: return imm.double_ == 0.0;
		default:
			break;
		}
		return false;
	}

	static bool IsOne(const ILOperand& op)
	{
		if (!op.IsImm()) return false;

		auto& imm = op.imm;
		switch (imm.Kind)
		{
		case NumberType_Int8: return imm.i8 == 1;
		case NumberType_Int16: return imm.i16 == 1;
		case NumberType_Int32: return imm.i32 == 1;
		case NumberType_Int64: return imm.i64 == 1;
		case NumberType_UInt8: return imm.u8 == 1;
		case NumberType_UInt16: return imm.u16 == 1;
		case NumberType_UInt32: return imm.u32 == 1;
		case NumberType_UInt64: return imm.u64 == 1;
		case NumberType_Half: return imm.half_ == 1.0f;
		case NumberType_Float: return imm.float_ == 1.0f;
		case NumberType_Double: return imm.double_ == 1.0;
		default:
			break;
		}
		return false;
	}

	static void ConvertToMove(ILInstruction& instr, const ILOperand& left)
	{
		instr.opcode = OpCode_Move;
		instr.operandLeft = left;
		instr.operandRight = {};
	}

	static void ConvertMoveRight(ILInstruction& instr)
	{
		ConvertToMove(instr, instr.operandRight);
	}

	static void ConvertMove(ILInstruction& instr)
	{
		instr.opcode = OpCode_Move;
		instr.operandRight = {};
	}

	static void ConvertMoveImm(ILInstruction& instr, const Number& num)
	{
		ConvertToMove(instr, Cast(num, instr.opKind));
	}

	static void ConvertMoveZero(ILInstruction& instr)
	{
		ConvertMoveImm(instr, Number(0));
	}

	void AlgebraicSimplifier::Visit(size_t index, CFGNode& node, EmptyCFGContext& context)
	{
		auto& instructions = node.instructions;
		const size_t n = instructions.size();

		for (size_t i = 0; i < n; i++)
		{
			auto& instr = instructions[i];

			switch (instr.opcode)
			{
			case OpCode_Multiply:
			{
				if (IsZero(instr.operandLeft) || IsZero(instr.operandRight))
				{
					ConvertMoveZero(instr); changed = true;
				}

				if (IsOne(instr.operandLeft))
				{
					ConvertMoveRight(instr); changed = true;
				}

				if (IsOne(instr.operandRight))
				{
					ConvertMove(instr); changed = true;
				}
			}
			break;
			case OpCode_Divide:
			{
				if (IsZero(instr.operandLeft))
				{
					ConvertMoveZero(instr); changed = true;
				}
				if (IsZero(instr.operandRight))
				{
					// TODO: add warning or error.
					instr.opcode = OpCode_Move;
					instr.operandRight = {};
					changed = true;
				}

				if (IsOne(instr.operandRight))
				{
					ConvertMove(instr); changed = true;
				}

				if (instr.operandLeft == instr.operandRight)
				{
					ConvertMoveImm(instr, Number(1)); changed = true;
				}
			}
			break;
			case OpCode_Subtract:
			{
				if (IsZero(instr.operandLeft))
				{
					ConvertMoveRight(instr); changed = true;
				}
				if (IsZero(instr.operandRight))
				{
					ConvertMove(instr); changed = true;
				}
				if (instr.operandLeft == instr.operandRight)
				{
					ConvertMoveZero(instr); changed = true;
				}
			}
			break;
			case OpCode_Add:
			{
				if (IsZero(instr.operandLeft))
				{
					ConvertMoveRight(instr); changed = true;
				}
				if (IsZero(instr.operandRight))
				{
					ConvertMove(instr); changed = true;
				}
			}
			break;
			case OpCode_Modulus:
			{
				if (IsZero(instr.operandLeft))
				{
					ConvertMoveZero(instr); changed = true;
				}
			}
			break;
			case OpCode_BitwiseAnd:
			{
				if (IsZero(instr.operandRight))
				{
					ConvertMoveZero(instr); changed = true;
				}
			}
			break;
			case OpCode_BitwiseOr:
			{
				if (IsZero(instr.operandRight))
				{
					ConvertMove(instr); changed = true;
				}
			}
			break;
			case OpCode_BitwiseXor:
			{
				if (IsZero(instr.operandRight))
				{
					ConvertMove(instr); changed = true;
				}

				if (instr.operandLeft == instr.operandRight)
				{
					ConvertMoveZero(instr); changed = true;
				}
			}
			break;
			case OpCode_AndAnd:
			{
				if (!instr.operandRight.IsImm())
				{
					break;
				}

				if (instr.operandRight.imm.ToBool())
				{
					ConvertMove(instr); changed = true;
				}
				else
				{
					changed = true;

					bool condition = false;
					auto j = i + 1;
					if (j < n)
					{
						auto& nextInstr = instructions[j];
						bool isTrueBranch = nextInstr.opcode == OpCode_JumpNotZero;
						if (isTrueBranch || nextInstr.opcode == OpCode_JumpZero)
						{
							bool willJump = (isTrueBranch && condition) || (!isTrueBranch && !condition);
							auto& target = nextInstr.operandLeft.varId;

							if (willJump)
							{
								auto& successors = node.successors;
								for (size_t i = 0; i < successors.size(); i++)
								{
									auto succs = successors[i];
									if (succs != target)
									{
										cfg.Unlink(index, succs);
										i--;

										auto& succsNode = cfg.GetNode(succs);
										if (succsNode.predecessors.empty())
										{
											cfg.RemoveNode(succs);
										}
									}
								}

								node.type = ControlFlowType_Normal;
								node.terminator = node.startInstr + i - 1;
								instructions.resize(i);

								auto& targetNode = cfg.GetNode(target);
								if (targetNode.predecessors.size() == 1 && targetNode.predecessors[0] == index)
								{
									cfg.MergeNodes(index, target);
								}
							}
							else
							{
								cfg.Unlink(index, target);
								auto& succsNode = cfg.GetNode(target);
								if (succsNode.predecessors.empty())
								{
									cfg.RemoveNode(target);
								}

								node.type = ControlFlowType_Normal;
								node.terminator = node.startInstr + i - 1;
								instructions.resize(i);
							}

							cfg.RebuildDomTree();
							return;
						}
					}
				}
			}
			break;
			case OpCode_OrOr:
			{
			}
			break;
			default:
				break;
			}
		}
	}
}