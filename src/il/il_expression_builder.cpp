#include "il_expression_builder.hpp"
#include "il_helper.hpp"

namespace HXSL
{
	static ILInstruction VecSwizzleExpr(ILContext* context, SwizzleDefinition* swizzle, Operand* inOp, Operand* outOp)
	{
		auto baseType = swizzle->GetBasePrim();
		auto targetType = swizzle->GetTargetPrim();

		if (baseType->GetClass() == PrimitiveClass_Scalar)
		{
			return ILInstruction(OpCode_BroadcastVec, inOp, outOp, PrimToOpKind(baseType->GetKind()));
		}

		return ILInstruction(OpCode_VecSwizzle, inOp, context->MakeConstant(Number(swizzle->GetMask())), outOp);
	}

	bool ILExpressionBuilder::IsInlineable(Expression* expr, Operand*& opOut)
	{
		Number imm;
		if (IsImmediate(expr, imm))
		{
			opOut = context->MakeConstant(imm);
			return true;
		}
		else if (expr->GetType() == NodeType_MemberReferenceExpression)
		{
			auto m = static_cast<MemberReferenceExpression*>(expr);
			auto& var = FindVar(m);
			if (!var.IsReference())
			{
				opOut = context->MakeVariable(var);
				return true;
			}
		}
		return false;
	}

	void ILExpressionBuilder::ReadVar(Expression* target, Operand* varOut)
	{
		MemberAccess(target, varOut, MemberOp_Read);
	}

	void ILExpressionBuilder::WriteVar(Expression* target, Operand* varsIn)
	{
		if (target->GetType() == NodeType_MemberReferenceExpression)
		{
			auto m = static_cast<MemberReferenceExpression*>(target);
			auto& var = FindVar(m);
			if (!var.IsReference())
			{
				if (!container.empty())
				{
					auto& result = container.back().operandResult;
					//HXSL_ASSERT(result.IsReg() && result.reg == registerIn, "");
					result = context->MakeVariable(var);
				}
				return;
			}
		}

		MemberAccess(target, {}, MemberOp_Write, varsIn);
	}

	SymbolDef* ILExpressionBuilder::GetAddrType(SymbolDef* elementType)
	{
		SymbolHandle handle;
		SymbolDef* def;
		compilation->GetPointerManager()->TryGetOrCreatePointerType(elementType, handle, def);
		return def;
	}

	bool ILExpressionBuilder::MemberAccess(Expression* expr, Operand* outVar, MemberOp op, Operand* writeOp)
	{
		if (auto m = expr->As<MemberReferenceExpression>())
		{
			auto& varId = FindVar(m);
			if (op == MemberOp_Write)
			{
				AddInstr(VecStoreOp(varId, m), writeOp, varId);
			}
			else if (op == MemberOp_Read)
			{
				AddInstr(VecLoadOp(varId, m), varId, outVar);
			}
			return false;
		}

		/*
		auto member = expr->Cast<MemberAccessExpression>();
		auto temp = reg.Alloc(GetAddrType(member->GetSymbolRef()->GetBaseDeclaration()));
		auto& varId = FindVar(member);
		AddInstr(VecLoadOp(varId, member, true), varId.AsOperand(), temp);
		*/

		auto member = expr->Cast<MemberAccessExpression>();
		auto& varId = FindVar(member);
		Operand* temp = context->MakeVariable(varId);

		bool first = true;
		auto next = member->GetNextExpression().get();
		while (next)
		{
			auto type = next->GetType();
			auto ref = next->GetSymbolRef().get();
			auto decl = ref->GetDeclaration();
			//reg.Free(temp);

			Operand* nextVar;
			if (auto swizzle = decl->As<SwizzleDefinition>())
			{
				nextVar = context->MakeVariable(reg.Alloc(next->GetInferredType()));
				if (first)
				{
					container.back() = VecSwizzleExpr(context, swizzle, context->MakeVariable(varId), nextVar);
				}
				else
				{
					AddInstr(VecSwizzleExpr(context, swizzle, temp, nextVar));
				}

				goto end;
			}

			nextVar = context->MakeVariable(reg.Alloc(GetAddrType(next->GetSymbolRef()->GetBaseDeclaration())));
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
					AddInstr(OpCode_Store, writeOp, nextVar, nullptr);
				}
				else if (op == MemberOp_Read)
				{
					AddInstr(OpCode_Load, nextVar, outVar);
				}
			}
			break;
			}
		end:
			temp = nextVar;
			next = next->GetNextExpression().get();
			first = false;
		}

		//reg.Free(temp);

		return true;
	}

	void ILExpressionBuilder::FunctionCall(FunctionCallExpression* expr)
	{
	}

	void ILExpressionBuilder::OperatorCall(OperatorOverload* op, Operand* left, Operand* right, Operand* result)
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

	Operand* ILExpressionBuilder::TraverseExpression(Expression* expression, Operand* outOperand)
	{
		auto outOp = outOperand;

		outOp = outOperand != nullptr ? outOperand : context->MakeVariable(reg.Alloc(expression->GetInferredType()));

		if (expression->GetInferredType() == nullptr)
		{
			return outOp;
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

				Operand* imm0; Operand* imm1;
				bool isImm0 = IsInlineable(left, imm0);
				bool isImm1 = IsInlineable(right, imm1);

				if (currentFrame.state || isImm0 && isImm1)
				{
					Operand* leftOp = isImm0 ? imm0 : currentFrame.outRegister;
					Operand* rightOp = isImm1 ? imm1 : currentFrame.rightRegister;
					OperatorCall(binary, leftOp, rightOp, currentFrame.outRegister);
				}
				else
				{
					currentFrame.state++;
					currentFrame.rightRegister = isImm1 ? nullptr : context->MakeVariable(reg.Alloc(right->GetInferredType()));
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
					AddInstr(OpCode_Move, num, currentFrame.outRegister, NumToOpKind(num.Kind));
				}
				else if (token.isBool())
				{
					bool value = token.Value == Keyword_True;
					AddInstr(OpCode_Equal, Number(value), Number(true), currentFrame.outRegister, ILOpKind_U8);
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

				Operand* imm;
				bool isImm = IsInlineable(operand, imm);

				Operand* operandReg = {};
				if (!isImm)
				{
					operandReg = context->MakeVariable(reg.Alloc(operand->GetInferredType()));
					MemberAccess(operand, operandReg);
				}

				Operand* ilOperand = isImm ? imm : operandReg;

				auto op = static_cast<OperatorOverload*>(pref->GetOperatorSymbolRef()->GetDeclaration());
				if ((op->GetOperatorFlags() & OperatorFlags_Intrinsic) != 0)
				{
					if (operator_ == Operator_Increment)
					{
						AddInstr(OpCode_Add, ilOperand, Number(1), currentFrame.outRegister, PrimToOpKind(op));
						MemberAccess(operand, currentFrame.outRegister, MemberOp_Write, currentFrame.outRegister);
					}
					else if (operator_ == Operator_Decrement)
					{
						AddInstr(OpCode_Subtract, ilOperand, Number(1), currentFrame.outRegister, PrimToOpKind(op));
						MemberAccess(operand, currentFrame.outRegister, MemberOp_Write, currentFrame.outRegister);
					}
					else if (operator_ == Operator_Subtract)
					{
						AddInstr(OpCode_Negate, ilOperand, currentFrame.outRegister, PrimToOpKind(op));
					}
				}
				else
				{
					// TODO: function call.
				}
			}
			break;
			case NodeType_PostfixExpression:
			{
				auto pref = expr->As<PostfixExpression>();
				auto operand = pref->GetOperand().get();
				auto operator_ = pref->GetOperator();

				MemberAccess(operand, currentFrame.outRegister);

				Operand* ilOperand = currentFrame.outRegister;

				auto op = static_cast<OperatorOverload*>(pref->GetOperatorSymbolRef()->GetDeclaration());
				if ((op->GetOperatorFlags() & OperatorFlags_Intrinsic) != 0)
				{
					auto temp = context->MakeVariable(reg.Alloc(operand->GetInferredType()));
					if (operator_ == Operator_Increment)
					{
						AddInstr(OpCode_Add, currentFrame.outRegister, Number(1), temp, PrimToOpKind(op));
					}
					else
					{
						AddInstr(OpCode_Subtract, currentFrame.outRegister, Number(1), temp, PrimToOpKind(op));
					}
					MemberAccess(operand, temp, MemberOp_Write, temp);
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
				auto overload = funcCall->GetSymbolRef()->GetDeclaration()->As<FunctionOverload>();
				auto returnType = overload->GetReturnType();
				auto& parameters = funcCall->GetParameters();
				bool isConstructor = funcCall->IsConstructorCall();
				size_t paramOffset = 0;
				if (isConstructor)
				{
					if (currentFrame.state == 0)
					{
						if (!container.empty())
						{
							auto& last = container.back();
							if (last.opcode == OpCode_StackAlloc)
							{
								AddInstr(OpCode_StoreParam, last.operandResult, Number(0));
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
							AddInstr(OpCode_StoreParam, currentFrame.rightRegister, nullptr, Number(paramOffset + currentFrame.state - 1));
						}

						auto param = parameters[currentFrame.state].get();
						auto operand = param->GetExpression().get();
						currentFrame.state++;
						Number imm;
						bool isImm = IsImmediate(operand, imm);
						if (isImm)
						{
							looped = true;
							AddInstr(OpCode_StoreParam, imm);
						}
						else
						{
							if (operand->GetType() == NodeType_MemberReferenceExpression)
							{
								looped = true;
								auto member = static_cast<MemberReferenceExpression*>(operand);
								auto& var = FindVar(member);
								AddInstr(OpCode_StoreParam, var, nullptr, Number(paramOffset + currentFrame.state - 1));
							}
							else
							{
								PushFrame(currentFrame);
								PushFrame({ operand, currentFrame.rightRegister = context->MakeVariable(reg.Alloc(operand->GetInferredType())) });
								break;
							}
						}
					}
					else
					{
						if (currentFrame.state != 0 && !looped)
						{
							AddInstr(OpCode_StoreParam, currentFrame.rightRegister, nullptr, Number(paramOffset + currentFrame.state - 1));
						}

						if (returnType != nullptr && !expression->IsVoidType())
						{
							AddInstr(OpCode_Call, RegFunc(overload), currentFrame.outRegister);
						}
						else
						{
							AddInstr(OpCode_Call, RegFunc(overload));
						}
						break;
					}
				}
			}
			break;
			case NodeType_MemberReferenceExpression:
			case NodeType_MemberAccessExpression:
				MemberAccess(expr, currentFrame.outRegister);
				break;
			case NodeType_CastExpression:
			{
				auto cast = static_cast<CastExpression*>(expr);
				auto operand = cast->GetOperand().get();

				Operand* imm;
				bool isImm = IsInlineable(operand, imm);

				if (currentFrame.state || isImm)
				{
					auto ilOperand = isImm ? imm : currentFrame.rightRegister;

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
					PushFrame({ operand, currentFrame.rightRegister = context->MakeVariable(reg.Alloc(operand->GetInferredType())) });
				}
			}
			break;
			case NodeType_AssignmentExpression:
			{
				auto assign = static_cast<AssignmentExpression*>(expr);
				auto target = assign->GetTarget().get();
				auto operand = assign->GetExpression().get();

				Operand* imm;
				bool isImm = IsInlineable(operand, imm);

				if (currentFrame.state || isImm)
				{
					if (isImm)
					{
						MemberAccess(target, {}, MemberOp_Write, imm);
					}
					else
					{
						WriteVar(target, currentFrame.outRegister);
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

				Operand* imm;
				bool isImm = IsInlineable(operand, imm);
				if (isImm || currentFrame.state == 0)
				{
					ReadVar(target, currentFrame.outRegister);
				}

				if (currentFrame.state || isImm)
				{
					auto targetOp = currentFrame.outRegister;
					auto rightOp = isImm ? imm : currentFrame.rightRegister;

					auto op = assign->GetOperatorSymbolRef()->GetDeclaration()->As<OperatorOverload>();
					OperatorCall(op, targetOp, rightOp, targetOp);

					WriteVar(target, currentFrame.outRegister);
				}
				else
				{
					currentFrame.state++;
					currentFrame.rightRegister = context->MakeVariable(reg.Alloc(operand->GetInferredType()));
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
					currentFrame.label = MakeJumpLocation();
					AddInstr(OpCode_JumpZero, currentFrame.label);

					currentFrame.state++;
					PushCurrent();
					PushFrame({ ternary->GetTrueBranch().get(), currentFrame.outRegister });
				}
				else if (currentFrame.state == 2)
				{
					auto endLoc = MakeJumpLocation();
					AddInstr(OpCode_Jump, endLoc);

					SetLocation(currentFrame.label);
					currentFrame.label = endLoc;

					currentFrame.state++;
					PushCurrent();
					PushFrame({ ternary->GetFalseBranch().get(), currentFrame.outRegister });
				}
				else if (currentFrame.state == 3)
				{
					SetLocation(currentFrame.label);
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

		return outOp;
	}
}