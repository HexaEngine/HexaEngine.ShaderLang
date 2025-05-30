#include "optimizers/algebraic_simplifier.hpp"
#include "il/il_helper.hpp"

namespace HXSL
{
	namespace Backend
	{
		DEFINE_IMM_COMP(IsZero, 0);

		DEFINE_IMM_COMP(IsOne, 1);

		static void ConvertToMove(ResultInstr& instr, Value* left)
		{
			auto block = instr.GetParent();
			block->ReplaceInstrO<MoveInstr>(&instr, instr.GetResult(), left);
		}

		static void ConvertMoveRight(BinaryInstr& instr)
		{
			ConvertToMove(instr, instr.GetRHS());
		}

		static void ConvertMoveLeft(BinaryInstr& instr)
		{
			ConvertToMove(instr, instr.GetLHS());
		}

		static void ConvertMoveImm(ILContext* context, ResultInstr& instr, const Number& num)
		{
			ConvertToMove(instr, context->MakeConstant(Cast(instr, instr.GetResult(), num)));
		}

		static void ConvertMoveZero(ILContext* context, ResultInstr& instr)
		{
			ConvertMoveImm(context, instr, Number(0));
		}

		void AlgebraicSimplifier::Visit(size_t index, BasicBlock& node, EmptyCFGContext& ctx)
		{
			for (auto& instr : node)
			{
				switch (instr.GetOpCode())
				{
				case OpCode_Multiply:
				{
					auto& in = *cast<BinaryInstr>(&instr);
					if (IsZero(in.GetLHS()) || IsZero(in.GetRHS()))
					{
						ConvertMoveZero(context, in); changed = true;
					}

					if (IsOne(in.GetLHS()))
					{
						ConvertMoveRight(in); changed = true;
					}

					if (IsOne(in.GetRHS()))
					{
						ConvertMoveLeft(in); changed = true;
					}
				}
				break;
				case OpCode_Divide:
				{
					auto& in = *cast<BinaryInstr>(&instr);
					if (IsZero(in.GetLHS()))
					{
						ConvertMoveZero(context, in); changed = true;
					}
					if (IsZero(in.GetRHS()))
					{
						// TODO: add warning or error.
						//instr.opcode = OpCode_Move;
						//instr.GetRHS() = {};
						changed = true;
					}

					if (IsOne(in.GetRHS()))
					{
						ConvertMoveLeft(in); changed = true;
					}

					if (in.GetLHS() == in.GetRHS())
					{
						ConvertMoveImm(context, in, Number(1)); changed = true;
					}
				}
				break;
				case OpCode_Subtract:
				{
					auto& in = *cast<BinaryInstr>(&instr);
					if (IsZero(in.GetLHS()))
					{
						ConvertMoveRight(in); changed = true;
					}
					if (IsZero(in.GetRHS()))
					{
						ConvertMoveLeft(in); changed = true;
					}
					if (in.GetLHS() == in.GetRHS())
					{
						ConvertMoveZero(context, in); changed = true;
					}
				}
				break;
				case OpCode_Add:
				{
					auto& in = *cast<BinaryInstr>(&instr);
					if (IsZero(in.GetLHS()))
					{
						ConvertMoveRight(in); changed = true;
					}
					if (IsZero(in.GetRHS()))
					{
						ConvertMoveLeft(in); changed = true;
					}
				}
				break;
				case OpCode_Modulus:
				{
					auto& in = *cast<BinaryInstr>(&instr);
					if (IsZero(in.GetLHS()))
					{
						ConvertMoveZero(context, in); changed = true;
					}
				}
				break;
				case OpCode_BitwiseAnd:
				{
					auto& in = *cast<BinaryInstr>(&instr);
					if (IsZero(in.GetRHS()))
					{
						ConvertMoveZero(context, in); changed = true;
					}
				}
				break;
				case OpCode_BitwiseOr:
				{
					auto& in = *cast<BinaryInstr>(&instr);
					if (IsZero(in.GetRHS()))
					{
						ConvertMoveLeft(in); changed = true;
					}
				}
				break;
				case OpCode_BitwiseXor:
				{
					auto& in = *cast<BinaryInstr>(&instr);
					if (IsZero(in.GetRHS()))
					{
						ConvertMoveLeft(in); changed = true;
					}

					if (in.GetLHS() == in.GetRHS())
					{
						ConvertMoveZero(context, in); changed = true;
					}
				}
				break;
				case OpCode_AndAnd:
				{
					auto& in = *cast<BinaryInstr>(&instr);
					auto immR = dyn_cast<Constant>(in.GetRHS());
					if (!immR) break;
					if (immR->imm().ToBool())
					{
						ConvertMoveLeft(in); changed = true;
					}
					else
					{
						changed = true;

						bool condition = false;
						if (instr.GetNext())
						{
							auto jumpInstr = dyn_cast<JumpInstr>(instr.GetNext());
							if (jumpInstr && jumpInstr->GetOpCode() == OpCode_Jump)
							{
								auto& jump = *jumpInstr;
								bool isTrueBranch = jump.GetOpCode() == OpCode_JumpNotZero;
								bool willJump = (isTrueBranch && condition) || (!isTrueBranch && !condition);
								auto& target = jump.GetLabel()->label.value;

								if (willJump)
								{
									auto& successors = node.GetSuccessors();
									for (size_t i = 0; i < successors.size(); i++)
									{
										auto succs = successors[i];
										if (succs != target)
										{
											cfg.Unlink(index, succs);
											i--;

											auto& succsNode = cfg.GetNode(succs);
											if (succsNode.IsPredecessorsEmpty())
											{
												cfg.RemoveNode(succs);
											}
										}
									}

									node.SetType(ControlFlowType_Normal);
									node.InstructionsTrimEnd(&instr);

									auto& targetNode = cfg.GetNode(target);
									if (targetNode.NumPredecessors() == 1 && targetNode.GetPredecessors()[0] == index)
									{
										cfg.MergeNodes(index, target);
									}
								}
								else
								{
									cfg.Unlink(index, target);
									auto& succsNode = cfg.GetNode(target);
									if (succsNode.IsPredecessorsEmpty())
									{
										cfg.RemoveNode(target);
									}

									node.SetType(ControlFlowType_Normal);
									node.InstructionsTrimEnd(&instr);
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
}