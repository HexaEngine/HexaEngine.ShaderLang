#include "il_expression_builder.hpp"
#include "il/il_helper.hpp"
#include "helper.hpp"

namespace HXSL
{
	static Instruction VecSwizzleExpr(ILContext* context, SwizzleDefinition* swizzle, Operand* inOp, const ILVarId& outOp)
	{
		auto baseType = swizzle->GetBasePrim();
		auto targetType = swizzle->GetTargetPrim();

		if (baseType->GetClass() == PrimitiveClass_Scalar)
		{
			return UnaryInstr(context->allocator, OpCode_BroadcastVec, outOp, inOp);
		}

		return BinaryInstr(context->allocator, OpCode_VecSwizzle, outOp, inOp, context->MakeConstant(Number(swizzle->GetMask())));
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

	void ILExpressionBuilder::ReadVar(Expression* target, ILVarId varOut)
	{
		bool isAddress;
		auto& var = MemberAccess(target, isAddress);
		AddLoadInstr(var, varOut);
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
					cast<ResultInstr>(&container.back())->GetResult() = var;
				}
				return;
			}
		}

		bool isAddress;
		auto& var = MemberAccess(target, isAddress);
		AddStoreInstr(var, varsIn);
	}

	ILVariable& ILExpressionBuilder::MemberAccess(Expression* expr, bool& isAddress)
	{
		isAddress = false;
		if (auto m = dyn_cast<MemberReferenceExpression>(expr))
		{
			return FindVar(m);
		}

		auto member = cast<MemberAccessExpression>(expr);
		auto& varId = FindVar(member);

		ILVariable* curVarId = &varId;

		bool first = true;
		auto next = member->GetNextExpression().get();
		while (next)
		{
			auto type = next->GetType();
			auto ref = SymbolRefHelper::GetSymbolRef(next).get();
			auto decl = ref->GetDeclaration();

			Variable* currentVar = context->MakeVariable(*curVarId);

			ILVariable* nextVar;
			if (auto swizzle = dyn_cast<SwizzleDefinition>(decl))
			{
				nextVar = &MakeTemp(next->GetInferredType());
				if (first)
				{
					container.back() = VecSwizzleExpr(context, swizzle, context->MakeVariable(varId), *nextVar);
				}
				else
				{
					AddInstr(VecSwizzleExpr(context, swizzle, currentVar, *nextVar));
				}
				isAddress = false;
				goto end;
			}

			nextVar = &MakeTemp(MakeAddrType(ref->GetBaseDeclaration()));
			switch (type)
			{
			case NodeType_MemberAccessExpression:
			{
				AddInstrO<OffsetInstr>(*nextVar, currentVar, MakeFieldAccess(decl));
				isAddress = true;
			}
			break;
			case NodeType_MemberReferenceExpression:
			{
				AddInstrO<OffsetInstr>(*nextVar, currentVar, MakeFieldAccess(decl));
				isAddress = true;
				return *nextVar;
			}
			break;
			}
		end:
			curVarId = nextVar;
			next = next->GetNextExpression().get();
			first = false;
		}

		return *curVarId;
	}

	void ILExpressionBuilder::FunctionCall(FunctionCallExpression* expr)
	{
	}

	void ILExpressionBuilder::OperatorCall(OperatorOverload* op, Operand* left, Operand* right, ILVarId result)
	{
		if ((op->GetOperatorFlags() & OperatorFlags_Intrinsic) != 0)
		{
			auto prim = cast<Primitive>(op->GetParent());
			bool vec = prim->GetClass() == PrimitiveClass_Vector;
			AddInstr<BinaryInstr>(vec ? OperatorToVecOpCode(op->GetOperator(), prim->GetRows()) : OperatorToOpCode(op->GetOperator()), result, left, right);
		}
		else
		{
			// TODO: function call.
		}
	}

	ILVarId ILExpressionBuilder::TraverseExpression(Expression* expression, ILVarId outOperand)
	{
		auto outOp = outOperand;

		outOp = outOperand != INVALID_VARIABLE ? outOperand : MakeTemp(expression->GetInferredType());

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
					Operand* leftOp = isImm0 ? imm0 : currentFrame.leftRegister;
					Operand* rightOp = isImm1 ? imm1 : currentFrame.rightRegister;
					OperatorCall(binary, leftOp, rightOp, currentFrame.outRegister);
				}
				else
				{
					currentFrame.state++;
					currentFrame.leftRegister = isImm0 ? nullptr : context->MakeVariable(MakeTemp(left->GetInferredType()));
					currentFrame.rightRegister = isImm1 ? nullptr : context->MakeVariable(MakeTemp(right->GetInferredType()));
					PushFrame(currentFrame);

					if (!isImm1)
					{
						PushFrame({ right, currentFrame.rightRegister->varId });
					}

					if (!isImm0)
					{
						PushFrame({ left, currentFrame.leftRegister->varId });
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
					AddInstrO<MoveInstr>(currentFrame.outRegister, num);
				}
				else if (token.isBool())
				{
					bool value = token.Value == Keyword_True;
					AddInstr<BinaryInstr>(OpCode_Equal, currentFrame.outRegister, Number(value), Number(true));
				}
				else
				{
					HXSL_ASSERT(false, "Invalid token as constant expression, this should never happen.");
				}
			}
			break;

			case NodeType_PrefixExpression:
			{
				auto pref = cast<PrefixExpression>(expr);
				auto operand = pref->GetOperand().get();
				auto operator_ = pref->GetOperator();

				Operand* imm;
				bool isImm = IsInlineable(operand, imm);

				Variable* operandReg = nullptr;
				if (!isImm)
				{
					operandReg = context->MakeVariable(MakeTemp(operand->GetInferredType()));
					ReadVar(operand, operandReg->varId);
				}

				Operand* ilOperand = isImm ? imm : operandReg;

				auto op = static_cast<OperatorOverload*>(pref->GetOperatorSymbolRef()->GetDeclaration());
				if ((op->GetOperatorFlags() & OperatorFlags_Intrinsic) != 0)
				{
					if (operator_ == Operator_Increment)
					{
						AddInstr<BinaryInstr>(OpCode_Add, currentFrame.outRegister, ilOperand, Number(1));
						WriteVar(operand, context->MakeVariable(currentFrame.outRegister));
					}
					else if (operator_ == Operator_Decrement)
					{
						AddInstr<BinaryInstr>(OpCode_Subtract, currentFrame.outRegister, ilOperand, Number(1));
						WriteVar(operand, context->MakeVariable(currentFrame.outRegister));
					}
					else if (operator_ == Operator_Subtract)
					{
						AddInstr<UnaryInstr>(OpCode_Negate, currentFrame.outRegister, ilOperand);
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
				auto pref = cast<PostfixExpression>(expr);
				auto operand = pref->GetOperand().get();
				auto operator_ = pref->GetOperator();

				ReadVar(operand, currentFrame.outRegister);

				auto op = static_cast<OperatorOverload*>(pref->GetOperatorSymbolRef()->GetDeclaration());
				if ((op->GetOperatorFlags() & OperatorFlags_Intrinsic) != 0)
				{
					auto& temp = MakeTemp(operand->GetInferredType());
					if (operator_ == Operator_Increment)
					{
						AddInstr<BinaryInstr>(OpCode_Add, temp, currentFrame.outRegister, Number(1));
					}
					else
					{
						AddInstr<BinaryInstr>(OpCode_Subtract, temp, currentFrame.outRegister, Number(1));
					}
					WriteVar(operand, context->MakeVariable(temp));
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
				auto overload = cast<FunctionOverload>(funcCall->GetSymbolRef()->GetDeclaration());
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
							auto last = &container.back();
							if (auto alloc = dyn_cast<StackAllocInstr>(last))
							{
								AddInstrONO<StoreParamInstr>(alloc->GetResult(), Number(0));
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
							AddInstrONO<StoreParamInstr>(currentFrame.rightRegister, Number(paramOffset + currentFrame.state - 1));
						}

						auto param = parameters[currentFrame.state].get();
						auto operand = param->GetExpression().get();
						currentFrame.state++;
						Number imm;
						bool isImm = IsImmediate(operand, imm);
						if (isImm)
						{
							looped = true;
							AddInstrONO<StoreParamInstr>(imm, Number(paramOffset + currentFrame.state - 1));
						}
						else
						{
							if (operand->GetType() == NodeType_MemberReferenceExpression)
							{
								looped = true;
								auto member = static_cast<MemberReferenceExpression*>(operand);
								auto& var = FindVar(member);
								AddInstrONO<StoreParamInstr>(var, Number(paramOffset + currentFrame.state - 1));
							}
							else
							{
								currentFrame.rightRegister = context->MakeVariable(MakeTemp(operand->GetInferredType()));
								PushFrame(currentFrame);
								PushFrame({ operand, currentFrame.rightRegister->varId });
								break;
							}
						}
					}
					else
					{
						if (currentFrame.state != 0 && !looped)
						{
							AddInstrONO<StoreParamInstr>(currentFrame.rightRegister, Number(paramOffset + currentFrame.state - 1));
						}

						if (returnType != nullptr && !expression->IsVoidType())
						{
							AddInstrO<CallInstr>(currentFrame.outRegister, RegFunc(overload));
						}
						else
						{
							AddInstrONO<CallInstr>(RegFunc(overload));
						}
						break;
					}
				}
			}
			break;
			case NodeType_MemberReferenceExpression:
			case NodeType_MemberAccessExpression:
				ReadVar(expr, currentFrame.outRegister);
				break;
			case NodeType_CastExpression:
			{
				auto castExpr = static_cast<CastExpression*>(expr);
				auto operand = castExpr->GetOperand().get();

				Operand* imm;
				bool isImm = IsInlineable(operand, imm);

				if (currentFrame.state || isImm)
				{
					auto ilOperand = isImm ? imm : currentFrame.rightRegister;

					auto op = cast<OperatorOverload>(castExpr->GetOperatorSymbolRef()->GetDeclaration());

					if ((op->GetOperatorFlags() & OperatorFlags_Intrinsic) != 0)
					{
						AddInstr<UnaryInstr>(OpCode_Cast, currentFrame.outRegister, ilOperand);
					}
					else
					{
						// TODO: function call.
					}
				}
				else
				{
					currentFrame.rightRegister = context->MakeVariable(MakeTemp(operand->GetInferredType()));
					currentFrame.state++;
					PushFrame(currentFrame);
					PushFrame({ operand, currentFrame.rightRegister->varId });
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
						WriteVar(target, context->MakeConstant(imm));
					}
					else
					{
						WriteVar(target, currentFrame.leftRegister);
					}
				}
				else
				{
					currentFrame.leftRegister = context->MakeVariable(MakeTemp(operand->GetInferredType()));
					currentFrame.state++;
					PushCurrent();
					PushFrame({ operand, currentFrame.leftRegister->varId });
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
					currentFrame.leftRegister = context->MakeVariable(MakeTemp(operand->GetInferredType()));
					ReadVar(target, currentFrame.leftRegister->varId);
				}

				if (currentFrame.state || isImm)
				{
					auto targetOp = currentFrame.outRegister;
					auto leftOp = currentFrame.leftRegister;
					auto rightOp = isImm ? imm : currentFrame.rightRegister;

					auto op = cast<OperatorOverload>(assign->GetOperatorSymbolRef()->GetDeclaration());
					OperatorCall(op, leftOp, rightOp, targetOp);

					WriteVar(target, context->MakeVariable(targetOp));
				}
				else
				{
					currentFrame.state++;
					currentFrame.rightRegister = context->MakeVariable(MakeTemp(operand->GetInferredType()));
					PushCurrent();
					PushFrame({ operand, currentFrame.rightRegister->varId });
				}
			}
			break;
			case NodeType_TernaryExpression:
			{
				auto ternary = static_cast<TernaryExpression*>(expr);

				if (currentFrame.state == 1)
				{
					currentFrame.label = MakeJumpLocation();
					AddInstrNO<JumpInstr>(OpCode_JumpZero, currentFrame.label);

					currentFrame.state++;
					PushCurrent();
					PushFrame({ ternary->GetTrueBranch().get(), currentFrame.outRegister });
				}
				else if (currentFrame.state == 2)
				{
					auto endLoc = MakeJumpLocation();
					AddInstrNO<JumpInstr>(OpCode_Jump, endLoc);

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