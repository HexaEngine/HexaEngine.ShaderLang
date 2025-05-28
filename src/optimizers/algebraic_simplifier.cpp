#include "algebraic_simplifier.hpp"
#include "il/il_helper.hpp"

namespace HXSL
{
	DEFINE_IMM_COMP(IsZero, 0);

	DEFINE_IMM_COMP(IsOne, 1);

	static void ConvertToMove(ILInstruction& instr, Value* left)
	{
		instr.opcode = OpCode_Move;
		instr.operandLeft = left;
		instr.operandRight = nullptr;
	}

	static void ConvertMoveRight(ILInstruction& instr)
	{
		ConvertToMove(instr, instr.operandRight);
	}

	static void ConvertMove(ILInstruction& instr)
	{
		instr.opcode = OpCode_Move;
		instr.operandRight = nullptr;
	}

	static void ConvertMoveImm(ILContext* context, ILInstruction& instr, const Number& num)
	{
		ConvertToMove(instr, context->MakeConstant(Cast(num, instr.opKind)));
	}

	static void ConvertMoveZero(ILContext* context, ILInstruction& instr)
	{
		ConvertMoveImm(context, instr, Number(0));
	}

	void AlgebraicSimplifier::Visit(size_t index, CFGNode& node, EmptyCFGContext& ctx)
	{
		auto& instructions = node.instructions;

		for (auto& instr : instructions)
		{
			switch (instr.opcode)
			{
			case OpCode_Multiply:
			{
				if (IsZero(instr.operandLeft) || IsZero(instr.operandRight))
				{
					ConvertMoveZero(context, instr); changed = true;
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
					ConvertMoveZero(context, instr); changed = true;
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
					ConvertMoveImm(context, instr, Number(1)); changed = true;
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
					ConvertMoveZero(context, instr); changed = true;
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
					ConvertMoveZero(context, instr); changed = true;
				}
			}
			break;
			case OpCode_BitwiseAnd:
			{
				if (IsZero(instr.operandRight))
				{
					ConvertMoveZero(context, instr); changed = true;
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
					ConvertMoveZero(context, instr); changed = true;
				}
			}
			break;
			case OpCode_AndAnd:
			{
				auto immR = dyn_cast<Constant>(instr.operandRight);
				if (!immR) break;
				if (immR->imm().ToBool())
				{
					ConvertMove(instr); changed = true;
				}
				else
				{
					changed = true;

					bool condition = false;
					if (instr.GetNext())
					{
						auto& nextInstr = *instr.GetNext();
						bool isTrueBranch = nextInstr.opcode == OpCode_JumpNotZero;
						if (isTrueBranch || nextInstr.opcode == OpCode_JumpZero)
						{
							bool willJump = (isTrueBranch && condition) || (!isTrueBranch && !condition);
							auto& target = cast<Label>(nextInstr.operandLeft)->label.value;

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
								instructions.trim_end(&instr);

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
								instructions.trim_end(&instr);
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