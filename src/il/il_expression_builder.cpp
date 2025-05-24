#include "il_expression_builder.hpp"
#include "il_helper.hpp"

namespace HXSL
{
	static ILOpCode VecSwizzle(uint32_t components)
	{
		switch (components)
		{
		case 2: return OpCode_Vec2Swizzle;
		case 3: return OpCode_Vec3Swizzle;
		case 4: return OpCode_Vec4Swizzle;
		default: return OpCode_Noop;
		}
	}

	static ILOpCode VecBroadcast(uint32_t components)
	{
		switch (components)
		{
		case 2: return OpCode_BroadcastVec2;
		case 3: return OpCode_BroadcastVec3;
		case 4: return OpCode_BroadcastVec4;
		default: return OpCode_Noop;
		}
	}

	static ILInstruction VecSwizzleExpr(SwizzleDefinition* swizzle, const ILOperand& inOp, const ILOperand& outOp)
	{
		auto baseType = swizzle->GetBasePrim();
		auto targetType = swizzle->GetTargetPrim();

		if (baseType->GetClass() == PrimitiveClass_Scalar)
		{
			return ILInstruction(VecBroadcast(targetType->GetRows()), inOp, outOp, PrimToOpKind(baseType->GetKind()));
		}

		return ILInstruction(VecSwizzle(targetType->GetRows()), inOp, Number(swizzle->GetMask()), outOp);
	}

	bool ILExpressionBuilder::IsInlineable(Expression* expr, ILOperand& opOut)
	{
		Number imm;
		if (IsImmediate(expr, imm))
		{
			opOut = imm;
			return true;
		}
		else if (expr->GetType() == NodeType_MemberReferenceExpression)
		{
			auto m = static_cast<MemberReferenceExpression*>(expr);
			auto& var = FindVar(m);
			if (!var.IsReference())
			{
				opOut = var.AsOperand();
				return true;
			}
		}
		return false;
	}

	void ILExpressionBuilder::ReadVar(Expression* target, ILRegister registerOut)
	{
		MemberAccess(target, registerOut, MemberOp_Read);
	}

	void ILExpressionBuilder::WriteVar(Expression* target, ILRegister registerIn)
	{
		if (target->GetType() == NodeType_MemberReferenceExpression)
		{
			auto m = static_cast<MemberReferenceExpression*>(target);
			auto& var = FindVar(m);
			if (!var.IsReference())
			{
				const size_t n = container.size();
				if (n != 0)
				{
					auto& result = container[n - 1].operandResult;
					HXSL_ASSERT(result.IsReg() && result.reg == registerIn, "");
					result = var.AsOperand();
				}
				return;
			}
		}

		MemberAccess(target, {}, MemberOp_Write, registerIn);
	}

	bool ILExpressionBuilder::MemberAccess(Expression* expr, ILRegister outRegister, MemberOp op, ILOperand writeOp)
	{
		if (auto m = expr->As<MemberReferenceExpression>())
		{
			auto& varId = FindVar(m);
			if (op == MemberOp_Write)
			{
				AddInstr(VecStoreOp(varId, m), writeOp, varId.AsTypeOperand(), varId.AsOperand());
			}
			else if (op == MemberOp_Read)
			{
				AddInstr(VecLoadOp(varId, m), varId.AsOperand(), varId.AsTypeOperand(), outRegister);
			}
			return false;
		}

		auto temp = reg.Alloc(expr->GetInferredType());
		auto member = expr->Cast<MemberAccessExpression>();
		auto& varId = FindVar(member);
		AddInstr(VecLoadOp(varId, member, true), varId.AsOperand(), varId.AsTypeOperand(), temp);

		bool first = true;
		auto next = member->GetNextExpression().get();
		while (next)
		{
			auto type = next->GetType();
			auto ref = next->GetSymbolRef().get();
			auto decl = ref->GetDeclaration();
			reg.Free(temp);
			auto nextVar = reg.Alloc(next->GetInferredType());

			if (auto swizzle = decl->As<SwizzleDefinition>())
			{
				if (first)
				{
					container.instructions.back() = VecSwizzleExpr(swizzle, varId.AsOperand(), nextVar);
				}
				else
				{
					AddInstr(VecSwizzleExpr(swizzle, temp, nextVar));
				}

				goto end;
			}
			switch (type)
			{
			case NodeType_MemberAccessExpression:
			{
				AddInstr(OpCode_OffsetAddress, temp, MakeFieldAccess(decl), nextVar);
			}
			break;
			case NodeType_MemberReferenceExpression:
			{
				AddInstr(OpCode_OffsetAddress, temp, MakeFieldAccess(decl), nextVar);

				if (op == MemberOp_Write)
				{
					AddInstr(OpCode_Store, writeOp, temp, ILOperand());
				}
				else if (op == MemberOp_Read)
				{
					AddInstr(OpCode_Load, temp, outRegister);
				}
			}
			break;
			default:
				break;
			}
		end:
			temp = nextVar;
			next = next->GetNextExpression().get();
			first = false;
		}

		reg.Free(temp);

		return true;
	}

	void ILExpressionBuilder::FunctionCall(FunctionCallExpression* expr)
	{
	}

	void ILExpressionBuilder::OperatorCall(OperatorOverload* op, const ILOperand& left, const ILOperand& right, const ILOperand& result)
	{
		if ((op->GetOperatorFlags() & OperatorFlags_Intrinsic) != 0)
		{
			auto prim = op->GetParent()->As<Primitive>();
			bool vec = prim->GetClass() == PrimitiveClass_Vector;
			AddInstr(vec ? OperatorToVecOpCode(op->GetOperator(), prim->GetRows()) : OperatorToOpCode(op->GetOperator()), left, right, result, PrimToOpKind(prim->GetKind()));
		}
		else
		{
			// TODO: function call.
		}
	}

	ILRegister ILExpressionBuilder::TraverseExpression(Expression* expression, const ILOperand& outOperand)
	{
		auto outOp = outOperand;
		if (outOperand.IsReg())
		{
			outOp.reg = outOperand.reg != INVALID_REGISTER ? outOperand.reg : 0;
			reg.current = outOp.reg.id + 1;
		}

		if (expression->GetInferredType() == nullptr)
		{
			return outOp.reg;
		}

		PushFrame({ expression, outOp });

		while (!stack.empty())
		{
			PopFrame();
			auto expr = currentFrame.expression;

			auto type = expr->GetType();
			switch (type)
			{
			case NodeType_EmptyExpression:
				break;

			case NodeType_BinaryExpression:
			{
				auto binary = static_cast<BinaryExpression*>(expr);
				auto left = binary->GetLeft().get();
				auto right = binary->GetRight().get();

				ILOperand imm0, imm1;
				bool isImm0 = IsInlineable(left, imm0);
				bool isImm1 = IsInlineable(right, imm1);

				if (currentFrame.state || isImm0 && isImm1)
				{
					ILOperand leftOp = isImm0 ? imm0 : ILOperand(currentFrame.outRegister);
					ILOperand rightOp = isImm1 ? imm1 : ILOperand(currentFrame.rightRegister);
					OperatorCall(binary, leftOp, rightOp, currentFrame.outRegister);
					reg.Free(currentFrame.rightRegister);
				}
				else
				{
					currentFrame.state++;
					currentFrame.rightRegister = isImm1 ? INVALID_REGISTER : reg.Alloc(right->GetInferredType());
					PushFrame(currentFrame);

					if (!isImm1)
					{
						PushFrame({ right, currentFrame.rightRegister });
					}

					if (!isImm0)
					{
						PushFrame({ left, currentFrame.outRegister });
					}
				}
			}
			break;
			case NodeType_LiteralExpression:
			{
				auto literal = static_cast<LiteralExpression*>(expr);
				auto& token = literal->GetLiteral();
				if (token.isNumeric())
				{
					auto num = token.Numeric;
					AddInstr(ILInstruction(OpCode_Move, ILOperand(num), ILOperand(currentFrame.outRegister), NumToOpKind(num.Kind)));
				}
				else if (token.isBool())
				{
					bool value = token.Value == Keyword_True;
					AddInstr(ILInstruction(OpCode_Equal, Number(value), Number(true), ILOperand(currentFrame.outRegister), ILOpKind_U8));
				}
				else
				{
					HXSL_ASSERT(false, "Invalid token as constant expression, this should never happen.");
				}
			}
			break;

			case NodeType_PrefixExpression:
			{
				auto pref = expr->As<PrefixExpression>();
				auto operand = pref->GetOperand().get();
				auto operator_ = pref->GetOperator();

				ILOperand imm;
				bool isImm = IsInlineable(operand, imm);

				ILRegister operandReg = {};
				if (!isImm)
				{
					operandReg = reg.Alloc(operand->GetInferredType());
					MemberAccess(operand, operandReg);
				}

				ILOperand ilOperand = isImm ? imm : ILOperand(operandReg);

				auto op = static_cast<OperatorOverload*>(pref->GetOperatorSymbolRef()->GetDeclaration());
				if ((op->GetOperatorFlags() & OperatorFlags_Intrinsic) != 0)
				{
					if (operator_ == Operator_Increment)
					{
						AddInstr(OpCode_Add, ilOperand, Number(1), ILOperand(currentFrame.outRegister), PrimToOpKind(op));
						MemberAccess(operand, currentFrame.outRegister.reg, MemberOp_Write, currentFrame.outRegister);
					}
					else if (operator_ == Operator_Decrement)
					{
						AddInstr(OpCode_Subtract, ilOperand, Number(1), ILOperand(currentFrame.outRegister), PrimToOpKind(op));
						MemberAccess(operand, currentFrame.outRegister.reg, MemberOp_Write, currentFrame.outRegister);
					}
					else if (operator_ == Operator_Subtract)
					{
						AddInstr(OpCode_Negate, ilOperand, ILOperand(currentFrame.outRegister), PrimToOpKind(op));
					}
				}
				else
				{
					// TODO: function call.
				}

				reg.Free(operandReg);
			}
			break;
			case NodeType_PostfixExpression:
			{
				auto pref = expr->As<PostfixExpression>();
				auto operand = pref->GetOperand().get();
				auto operator_ = pref->GetOperator();

				MemberAccess(operand, currentFrame.outRegister.reg);

				ILOperand ilOperand = ILOperand(currentFrame.outRegister);

				auto op = static_cast<OperatorOverload*>(pref->GetOperatorSymbolRef()->GetDeclaration());
				if ((op->GetOperatorFlags() & OperatorFlags_Intrinsic) != 0)
				{
					auto temp = reg.Alloc(operand->GetInferredType());
					if (operator_ == Operator_Increment)
					{
						AddInstr(OpCode_Add, ILOperand(currentFrame.outRegister), Number(1), temp, PrimToOpKind(op));
					}
					else
					{
						AddInstr(OpCode_Subtract, ILOperand(currentFrame.outRegister), Number(1), temp, PrimToOpKind(op));
					}
					MemberAccess(operand, temp, MemberOp_Write, temp);
					reg.Free(temp);
				}
				else
				{
					// TODO: function call.
				}
			}
			break;

			case NodeType_FunctionCallExpression:
			{
				auto funcCall = static_cast<FunctionCallExpression*>(expr);
				auto overload = funcCall->GetSymbolRef()->GetDeclaration();
				auto& parameters = funcCall->GetParameters();
				bool isConstructor = overload->IsTypeOf(NodeType_ConstructorOverload);
				size_t paramOffset = 0;
				if (isConstructor)
				{
					if (currentFrame.state == 0)
					{
						if (container.instructions.size() > 0)
						{
							auto& last = container.instructions.back();
							if (last.opcode == OpCode_StackAlloc)
							{
								AddInstr(OpCode_StoreParamRef, last.operandResult, last.operandLeft, Number(0));
							}
						}
					}
					paramOffset = 1;
				}

				bool looped = false;
				while (true)
				{
					if (currentFrame.state < parameters.size())
					{
						if (currentFrame.state != 0 && !looped)
						{
							AddInstr(OpCode_StoreParam, ILRegister(0), {}, Number(paramOffset + currentFrame.state - 1));
						}

						auto param = parameters[currentFrame.state].get();
						auto operand = param->GetExpression().get();
						currentFrame.state++;
						Number imm;
						bool isImm = IsImmediate(operand, imm);
						if (isImm)
						{
							looped = true;
							AddInstr(OpCode_StoreParam, ILOperand(imm));
						}
						else
						{
							if (operand->GetType() == NodeType_MemberReferenceExpression)
							{
								looped = true;
								auto member = static_cast<MemberReferenceExpression*>(operand);
								auto& var = FindVar(member);
								AddInstr(OpCode_StoreParam, var.AsOperand(), var.AsTypeOperand(), Number(paramOffset + currentFrame.state - 1));
							}
							else
							{
								PushFrame(currentFrame);
								PushFrame({ operand, currentFrame.outRegister });
								break;
							}
						}
					}
					else
					{
						if (currentFrame.state != 0 && !looped)
						{
							AddInstr(OpCode_StoreParam, ILRegister(0), {}, Number(paramOffset + currentFrame.state - 1));
						}

						if (!funcCall->IsVoidType())
						{
							AddInstr(OpCode_Call, ILOperand(ILOperandKind_Func, RegFunc(overload)), ILOperand(ILOperandKind_Type, RegType(funcCall->GetInferredType())), ILOperand(currentFrame.outRegister));
						}
						else
						{
							AddInstr(OpCode_Call, ILOperand(ILOperandKind_Func, RegFunc(overload)));
						}
						break;
					}
				}
			}
			break;
			case NodeType_MemberReferenceExpression:
			case NodeType_MemberAccessExpression:
				MemberAccess(expr, currentFrame.outRegister.reg);
				break;
			case NodeType_CastExpression:
			{
				auto cast = static_cast<CastExpression*>(expr);
				auto operand = cast->GetOperand().get();

				Number imm;
				bool isImm = IsImmediate(operand, imm);

				if (currentFrame.state || isImm)
				{
					auto ilOperand = isImm ? ILOperand(imm) : ILOperand(currentFrame.outRegister);

					auto op = cast->GetOperatorSymbolRef()->GetDeclaration()->As<OperatorOverload>();

					if ((op->GetOperatorFlags() & OperatorFlags_Intrinsic) != 0)
					{
						AddInstr(OpCode_Cast, ilOperand, currentFrame.outRegister, PrimToOpKind(op));
					}
					else
					{
						// TODO: function call.
					}
				}
				else
				{
					currentFrame.state++;
					PushFrame(currentFrame);
					PushFrame({ operand, currentFrame.outRegister });
				}
			}
			break;
			case NodeType_AssignmentExpression:
			{
				auto assign = static_cast<AssignmentExpression*>(expr);
				auto target = assign->GetTarget().get();
				auto operand = assign->GetExpression().get();

				ILOperand imm;
				bool isImm = IsInlineable(operand, imm);

				if (currentFrame.state || isImm)
				{
					if (isImm)
					{
						MemberAccess(target, {}, MemberOp_Write, imm);
					}
					else
					{
						WriteVar(target, currentFrame.outRegister.reg);
					}
				}
				else
				{
					currentFrame.state++;
					PushCurrent();
					PushFrame({ operand, currentFrame.outRegister });
				}
			}
			break;
			case NodeType_CompoundAssignmentExpression:
			{
				auto assign = static_cast<CompoundAssignmentExpression*>(expr);
				auto target = assign->GetTarget().get();
				auto operand = assign->GetExpression().get();

				ILOperand imm;
				bool isImm = IsInlineable(operand, imm);
				if (isImm || currentFrame.state == 0)
				{
					ReadVar(target, currentFrame.outRegister.reg);
				}

				if (currentFrame.state || isImm)
				{
					ILOperand targetOp = currentFrame.outRegister;
					ILOperand rightOp = isImm ? imm : ILOperand(currentFrame.rightRegister);

					auto op = assign->GetOperatorSymbolRef()->GetDeclaration()->As<OperatorOverload>();
					OperatorCall(op, targetOp, rightOp, targetOp);

					WriteVar(target, currentFrame.outRegister.reg);
					reg.Free(currentFrame.rightRegister);
				}
				else
				{
					currentFrame.state++;
					currentFrame.rightRegister = reg.Alloc(operand->GetInferredType());
					PushCurrent();
					PushFrame({ operand, currentFrame.rightRegister });
				}
			}
			break;
			case NodeType_TernaryExpression:
			{
				auto ternary = static_cast<TernaryExpression*>(expr);

				if (currentFrame.state == 1)
				{
					currentFrame.data = MakeJumpLocation();
					AddInstr(OpCode_JumpZero, ILOperand(ILOperandKind_Label, currentFrame.data));

					currentFrame.state++;
					PushCurrent();
					PushFrame({ ternary->GetTrueBranch().get(), currentFrame.outRegister });
				}
				else if (currentFrame.state == 2)
				{
					auto endLoc = MakeJumpLocation();
					AddInstr(OpCode_Jump, ILOperand(ILOperandKind_Label, endLoc));

					SetLocation(currentFrame.data);
					currentFrame.data = endLoc;

					currentFrame.state++;
					PushCurrent();
					PushFrame({ ternary->GetFalseBranch().get(), currentFrame.outRegister });
				}
				else if (currentFrame.state == 3)
				{
					SetLocation(currentFrame.data);
				}
				else
				{
					currentFrame.state++;
					PushCurrent();
					PushFrame({ ternary->GetCondition().get(), currentFrame.outRegister });
				}
			}
			break;
			case NodeType_IndexerAccessExpression:
				break;
			case NodeType_InitializationExpression:
				break;
			default:
				break;
			}
		}

		return outOp.reg;
	}
}