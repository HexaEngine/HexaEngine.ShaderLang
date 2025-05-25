#include "il_builder.hpp"
#include "il_helper.hpp"

namespace HXSL
{
	SymbolDef* ILBuilder::GetAddrType(SymbolDef* elementType)
	{
		SymbolHandle handle;
		SymbolDef* def;
		if (!compilation->GetPointerManager()->TryGetOrCreatePointerType(elementType, handle, def))
		{
			return elementType;
		}
		return def;
	}

	bool ILBuilder::TraverseStatement(Statement* statement)
	{
		auto type = statement->GetType();

		switch (type)
		{
		case NodeType_BlockStatement:
			PushFrame(ILFrame(statement->As<BlockStatement>()));
			break;
		case NodeType_ExpressionStatement:
		{
			MappingStart();
			TraverseExpression(statement->As<ExpressionStatement>()->GetExpression().get());
			MappingEnd(statement->GetSpan());
		}
		break;
		case NodeType_DeclarationStatement:
		{
			MappingStart();
			auto decl = statement->As<DeclarationStatement>();
			if (decl->GetDeclaredType() == nullptr) break;
			auto init = decl->GetInitializer().get();

			auto flags = GetVarTypeFlags(decl->GetDeclaredType());
			ILTypeId typeId;
			ILVariable varId;
			if ((flags & ILVariableFlags_LargeObject) != 0)
			{
				auto baseTypeId = RegType(decl->GetDeclaredType());
				typeId = RegType(GetAddrType(decl->GetDeclaredType()));
				varId = RegVar(typeId, decl);
				AddInstr(OpCode_StackAlloc, ILOperand(ILOperandKind_Type, baseTypeId), varId.AsOperand());
			}
			else
			{
				typeId = RegType(decl->GetDeclaredType());
				varId = RegVar(typeId, decl);
			}

			if (init)
			{
				Number imm;
				if (IsImmediate(init, imm))
				{
					AddInstr(VecStoreOp(varId, decl), imm, varId.AsOperand());
				}
				else
				{
					auto src = TraverseExpression(init);
					if (auto funcCall = init->As<FunctionCallExpression>())
					{
						if (funcCall->IsConstructorCall())
						{
							goto initEnd;
						}
					}

					if (varId.IsReference())
					{
						AddInstr(VecStoreOp(varId, decl), src, varId.AsOperand());
					}
					else
					{
						auto n = container.size();
						if (n != 0)
						{
							container[n - 1].operandResult = varId.AsOperand();
						}
					}
				}
			}
			else if (!varId.IsReference())
			{
				AddInstr(OpCode_Zero, ILOperand(ILOperandKind_Type, typeId), varId.AsOperand());
			}
		initEnd:
			MappingEnd(statement->GetSpan());
		}
		break;
		case NodeType_AssignmentStatement:
		case NodeType_CompoundAssignmentStatement:
		{
			MappingStart();
			auto assign = statement->As<AssignmentStatement>();
			auto expr = assign->GetAssignmentExpression().get();
			TraverseExpression(expr);
			MappingEnd(statement->GetSpan());
		}
		break;
		case NodeType_IfStatement:
		{
			auto ifStatement = statement->As<IfStatement>();
			auto condition = ifStatement->GetCondition().get();

			auto endIfId = MakeJumpLocation();

			MappingStart();
			TraverseExpression(condition);
			MappingEnd(condition->GetSpan());

			AddInstr(OpCode_JumpZero, ILOperand(ILOperandKind_Label, endIfId));

			PushFrame(ILFrame(ifStatement, endIfId));
			PushFrame(ILFrame(ifStatement->GetBody().get(), 0));
			return true;
		}
		break;
		case NodeType_WhileStatement:
		{
			auto whileStatement = statement->As<WhileStatement>();
			auto condition = whileStatement->GetCondition().get();

			auto whileBeginId = MakeJumpLocationFromCurrent();
			auto whileEndId = MakeJumpLocation();

			MappingStart();
			TraverseExpression(condition);
			MappingEnd(condition->GetSpan());

			AddInstr(OpCode_JumpZero, ILOperand(ILOperandKind_Label, whileEndId));

			PushFrame(ILFrame(whileStatement, whileBeginId, whileEndId));
			PushFrame(ILFrame(whileStatement->GetBody().get(), 0));
			PushLoop(ILLoopFrame(whileBeginId, whileEndId));
			return true;
		}
		break;
		case NodeType_DoWhileStatement:
		{
			auto doWhileStatement = statement->As<DoWhileStatement>();

			auto doWhileBeginId = MakeJumpLocationFromCurrent();
			auto doWhileCondId = MakeJumpLocation();
			auto doWhileEndId = MakeJumpLocation();

			PushFrame(ILFrame(doWhileStatement, doWhileBeginId, doWhileCondId, doWhileEndId));
			PushFrame(ILFrame(doWhileStatement->GetBody().get(), 0));
			PushLoop(ILLoopFrame(doWhileCondId, doWhileEndId));
			return true;
		}
		break;
		case NodeType_ForStatement:
		{
			auto forStatement = statement->As<ForStatement>();
			auto condition = forStatement->GetCondition().get();

			TraverseStatement(forStatement->GetInit().get());

			auto forBeginId = MakeJumpLocationFromCurrent();
			auto forIncId = MakeJumpLocation();
			auto forEndId = MakeJumpLocation();

			MappingStart();
			TraverseExpression(condition);
			MappingEnd(condition->GetSpan());

			AddInstr(OpCode_JumpZero, ILOperand(ILOperandKind_Label, forEndId));

			PushFrame(ILFrame(forStatement, forBeginId, forIncId, forEndId));
			PushFrame(ILFrame(forStatement->GetBody().get(), 0));
			PushLoop(ILLoopFrame(forIncId, forEndId));
			return true;
		}
		break;
		case NodeType_BreakStatement:
			MappingStart();
			AddInstr(OpCode_Jump, ILOperand(ILOperandKind_Label, currentLoop.breakLocation));
			MappingEnd(statement->GetSpan());
			break;
		case NodeType_ContinueStatement:
			MappingStart();
			AddInstr(OpCode_Jump, ILOperand(ILOperandKind_Label, currentLoop.continueLocation));
			MappingEnd(statement->GetSpan());
			break;
		case NodeType_DiscardStatement:
			MappingStart();
			AddInstr(OpCode_Discard);
			MappingEnd(statement->GetSpan());
			break;
		case NodeType_ReturnStatement:
		{
			MappingStart();
			didReturn = true;

			auto returnStatement = statement->As<ReturnStatement>();

			auto expr = returnStatement->GetReturnValueExpression().get();
			if (!expr)
			{
				AddInstr(OpCode_Return);
				break;
			}

			Number imm;
			bool isImm = IsImmediate(expr, imm);
			if (isImm)
			{
				AddInstr(OpCode_Return, ILOperand(imm));
			}
			else
			{
				if (expr->GetType() == NodeType_MemberReferenceExpression)
				{
					auto member = expr->As<MemberReferenceExpression>();
					AddInstr(OpCode_Return, FindVar(member).AsOperand());
				}
				else
				{
					auto src = TraverseExpression(expr);
					AddInstr(OpCode_Return, src);
				}
			}

			MappingEnd(statement->GetSpan());
		}
		break;
		case NodeType_SwitchStatement:
			break;
		case NodeType_CaseStatement:
			break;
		case NodeType_DefaultCaseStatement:
			break;
		}

		return false;
	}

	void ILBuilder::TraverseBlock(ILBlockFrame& frame)
	{
		auto& statements = frame.block->GetStatements();
		for (; frame.index < statements.size(); frame.index++)
		{
			auto statement = statements[frame.index].get();
			if (TraverseStatement(statement))
			{
				break;
			}
		}
	}

	void ILBuilder::Build(FunctionOverload* func)
	{
		stack.push(ILFrame(func->GetBody().get(), 0));

		auto canonicalParent = func->GetCanonicalParent();
		auto type = canonicalParent->GetType();

		size_t parameterBase = 0;
		/*
		if (type == NodeType_Struct || type == NodeType_Class)
		{
			ast_ptr<ThisDef> thisDef = make_ast_ptr<ThisDef>(make_ast_ptr<SymbolRef>("", SymbolRefType_Type, false));
			auto typeId = RegType(canonicalParent->As<SymbolDef>());
			auto& varId = RegVar(typeId, param.get());
			AddInstr(OpCode_LoadParam, Number(parameterBase), ILOperand(ILOperandKind_Type, typeId), varId.AsOperand());
			parameterBase++;
		}*/

		auto& parameters = func->GetParameters();
		for (size_t i = 0; i < parameters.size(); i++)
		{
			auto& param = parameters[i];
			auto typeId = RegType(param->GetDeclaredType());
			auto& varId = RegVar(typeId, param.get());
			AddInstr(OpCode_LoadParam, Number(parameterBase + i), varId.AsOperand());
		}

		while (!stack.empty())
		{
			PopFrame();

			switch (currentFrame.type)
			{
			case ILFrameType_Block:
				TraverseBlock(currentFrame.block);
				break;
			case ILFrameType_IfStatement:
			{
				auto& frame = currentFrame.ifFrame;
				auto ifStatement = frame.statement->As<IfStatement>();

				auto& elseIfStatements = ifStatement->GetElseIfStatements();

				if (frame.state < elseIfStatements.size())
				{
					auto endLoc = frame.endLocation == INVALID_JUMP_LOCATION ? frame.endLocation = MakeJumpLocation() : frame.endLocation;
					AddInstr(OpCode_Jump, ILOperand(ILOperandKind_Label, endLoc));
					SetLocation(frame.nextLocation);

					auto elseIfStatement = elseIfStatements[frame.state].get();

					auto condition = elseIfStatement->GetCondition().get();

					MappingStart();
					TraverseExpression(condition);
					MappingEnd(condition->GetSpan());

					frame.nextLocation = MakeJumpLocation();
					AddInstr(OpCode_JumpZero, ILOperand(ILOperandKind_Label, frame.nextLocation));

					frame.state++;
					PushFrame(currentFrame);
					PushFrame(ILFrame(elseIfStatement->GetBody().get()));
					break;
				}
				else
				{
					auto elseStatement = ifStatement->GetElseStatement().get();
					if (elseStatement)
					{
						auto endLoc = frame.endLocation == INVALID_JUMP_LOCATION ? frame.endLocation = MakeJumpLocation() : frame.endLocation;
						AddInstr(OpCode_Jump, ILOperand(ILOperandKind_Label, endLoc));
						SetLocation(frame.nextLocation);

						PushFrame(ILFrame(endLoc));
						PushFrame(ILFrame(elseStatement->GetBody().get()));
					}
					else
					{
						if (frame.endLocation != INVALID_JUMP_LOCATION)
						{
							SetLocation(frame.endLocation);
						}
						else if (frame.nextLocation != INVALID_JUMP_LOCATION && frame.state == 0)
						{
							SetLocation(frame.nextLocation);
						}
					}
				}
			}
			break;
			case ILFrameType_WhileStatement:
			{
				auto& frame = currentFrame.whileFrame;
				AddInstr(OpCode_Jump, ILOperand(ILOperandKind_Label, frame.startLocation));
				SetLocation(frame.endLocation);
				PopLoop();
			}
			break;
			case ILFrameType_DoWhileStatement:
			{
				auto& frame = currentFrame.doWhileFrame;
				SetLocation(frame.condLocation);
				MappingStart();
				TraverseExpression(frame.statement->GetCondition().get());
				MappingEnd(frame.statement->GetCondition()->GetSpan());
				AddInstr(OpCode_JumpNotZero, ILOperand(ILOperandKind_Label, frame.startLocation));
				SetLocation(frame.endLocation);
				PopLoop();
			}
			break;
			case ILFrameType_ForStatement:
			{
				auto& frame = currentFrame.forFrame;
				SetLocation(frame.incrLocation);
				TraverseExpression(frame.statement->GetIteration().get());
				AddInstr(OpCode_Jump, ILOperand(ILOperandKind_Label, frame.startLocation));
				SetLocation(frame.endLocation);
				PopLoop();
			}
			break;
			case ILFrameType_SetLocation:
				SetLocation(currentFrame.location);
				break;
			default:
				break;
			}
		}

		if (!didReturn)
		{
			AddInstr(OpCode_Return);
		}
	}
}