#include "il_builder.hpp"
#include "il/il_helper.hpp"
#include "helper.hpp"

namespace HXSL
{
	using namespace Backend;

	bool ILBuilder::TraverseStatement(ASTNode* statement)
	{
		auto type = statement->GetType();

		switch (type)
		{
		case NodeType_BlockStatement:
			PushFrame(ILFrame(cast<BlockStatement>(statement)));
			break;
		case NodeType_ExpressionStatement:
		{
			MappingStart();
			TraverseExpression(cast<ExpressionStatement>(statement)->GetExpression());
			MappingEnd(statement->GetSpan());
		}
		break;
		case NodeType_DeclarationStatement:
		{
			MappingStart();
			auto decl = cast<DeclarationStatement>(statement);
			if (decl->GetDeclaredType() == nullptr) break;
			auto init = decl->GetInitializer();

			auto flags = RegType(decl->GetDeclaredType())->GetVarTypeFlags();
			ILType typeId;
			ILVariable varId;
			if ((flags & ILVariableFlags_LargeObject) != 0)
			{
				auto baseTypeId = RegType(decl->GetDeclaredType());
				typeId = RegType(MakeAddrType(decl->GetDeclaredType()));
				varId = RegVar(typeId, decl);
				AddInstrO<StackAllocInstr>(varId, baseTypeId);
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
					AddStoreInstr(varId, imm);
				}
				else
				{
					auto src = TraverseExpression(init);
					if (auto funcCall = dyn_cast<FunctionCallExpression>(init))
					{
						if (funcCall->IsConstructorCall())
						{
							goto initEnd;
						}
					}

					if (varId.IsReference())
					{
						AddStoreInstr(varId, src);
					}
					else
					{
						if (!container.empty())
						{
							cast<ResultInstr>(&container.back())->GetResult() = varId;
						}
					}
				}
			}
			else if (!varId.IsReference())
			{
				AddInstr<UnaryInstr>(OpCode_Zero, varId, typeId);
			}
		initEnd:
			MappingEnd(statement->GetSpan());
		}
		break;
		case NodeType_AssignmentStatement:
		case NodeType_CompoundAssignmentStatement:
		{
			MappingStart();
			auto assign = cast<AssignmentStatement>(statement);
			auto expr = assign->GetAssignmentExpression();
			TraverseExpression(expr);
			MappingEnd(statement->GetSpan());
		}
		break;
		case NodeType_IfStatement:
		{
			auto ifStatement = cast<IfStatement>(statement);
			auto condition = ifStatement->GetCondition();

			auto endIfId = MakeJumpLocation();

			MappingStart();
			TraverseExpression(condition);
			MappingEnd(condition->GetSpan());

			AddInstrNO<JumpInstr>(OpCode_JumpZero, endIfId);

			PushFrame(ILFrame(ifStatement, endIfId));
			PushFrame(ILFrame(ifStatement->GetBody(), 0));
			return true;
		}
		break;
		case NodeType_WhileStatement:
		{
			auto whileStatement = cast<WhileStatement>(statement);
			auto condition = whileStatement->GetCondition();

			auto whileBeginId = MakeJumpLocationFromCurrent();
			auto whileEndId = MakeJumpLocation();

			MappingStart();
			TraverseExpression(condition);
			MappingEnd(condition->GetSpan());

			AddInstrNO<JumpInstr>(OpCode_JumpZero, whileEndId);

			PushFrame(ILFrame(whileStatement, whileBeginId, whileEndId));
			PushFrame(ILFrame(whileStatement->GetBody(), 0));
			PushLoop(ILLoopFrame(whileBeginId, whileEndId));
			return true;
		}
		break;
		case NodeType_DoWhileStatement:
		{
			auto doWhileStatement = cast<DoWhileStatement>(statement);

			auto doWhileBeginId = MakeJumpLocationFromCurrent();
			auto doWhileCondId = MakeJumpLocation();
			auto doWhileEndId = MakeJumpLocation();

			PushFrame(ILFrame(doWhileStatement, doWhileBeginId, doWhileCondId, doWhileEndId));
			PushFrame(ILFrame(doWhileStatement->GetBody(), 0));
			PushLoop(ILLoopFrame(doWhileCondId, doWhileEndId));
			return true;
		}
		break;
		case NodeType_ForStatement:
		{
			auto forStatement = cast<ForStatement>(statement);
			auto condition = forStatement->GetCondition();

			TraverseStatement(forStatement->GetInit());

			auto forBeginId = MakeJumpLocationFromCurrent();
			auto forIncId = MakeJumpLocation();
			auto forEndId = MakeJumpLocation();

			MappingStart();
			TraverseExpression(condition);
			MappingEnd(condition->GetSpan());

			AddInstrNO<JumpInstr>(OpCode_JumpZero, forEndId);

			PushFrame(ILFrame(forStatement, forBeginId, forIncId, forEndId));
			PushFrame(ILFrame(forStatement->GetBody(), 0));
			PushLoop(ILLoopFrame(forIncId, forEndId));
			return true;
		}
		break;
		case NodeType_BreakStatement:
			MappingStart();
			AddInstrNO<JumpInstr>(OpCode_Jump, currentLoop.breakLocation);
			MappingEnd(statement->GetSpan());
			break;
		case NodeType_ContinueStatement:
			MappingStart();
			AddInstrNO<JumpInstr>(OpCode_Jump, currentLoop.continueLocation);
			MappingEnd(statement->GetSpan());
			break;
		case NodeType_DiscardStatement:
			MappingStart();
			AddBasicInstr(OpCode_Discard);
			MappingEnd(statement->GetSpan());
			break;
		case NodeType_ReturnStatement:
		{
			MappingStart();
			didReturn = true;

			auto returnStatement = cast<ReturnStatement>(statement);

			auto expr = returnStatement->GetReturnValueExpression();
			if (!expr)
			{
				AddInstrONO<ReturnInstr>();
				break;
			}

			Number imm;
			bool isImm = IsImmediate(expr, imm);
			if (isImm)
			{
				AddInstrONO<ReturnInstr>(imm);
			}
			else
			{
				if (auto member = dyn_cast<MemberReferenceExpression>(expr))
				{
					AddInstrONO<ReturnInstr>(FindVar(member));
				}
				else
				{
					auto src = TraverseExpression(expr);
					AddInstrONO<ReturnInstr>(src);
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
		auto statements = frame.block->GetStatements();
		for (; frame.index < statements.size(); frame.index++)
		{
			auto statement = statements[frame.index];
			if (TraverseStatement(statement))
			{
				break;
			}
		}
	}

	void ILBuilder::Build(FunctionOverload* func)
	{
		stack.push(ILFrame(func->GetBody(), 0));

		auto parent = func->GetParent();
		auto type = parent->GetType();

		size_t parameterBase = 0;
		/*
		if (type == NodeType_Struct || type == NodeType_Class)
		{
			ast_ptr<ThisDef> thisDef = make_ast_ptr<ThisDef>(make_ast_ptr<SymbolRef>("", SymbolRefType_Type, false));
			auto typeId = RegType(canonicalParent->As<SymbolDef>());
			auto& varId = RegVar(typeId, param);
			AddInstr(OpCode_LoadParam, Number(parameterBase), ILOperand(ILOperandKind_Type, typeId), varId.AsOperand());
			parameterBase++;
		}*/

		auto parameters = func->GetParameters();
		for (size_t i = 0; i < parameters.size(); i++)
		{
			auto& param = parameters[i];
			auto typeId = RegType(param->GetDeclaredType());
			auto& varId = RegVar(typeId, param);
			AddInstrO<LoadParamInstr>(varId, Number(parameterBase + i));
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
				auto ifStatement = frame.statement;

				auto elseIfStatements = ifStatement->GetElseIfStatements();

				if (frame.state < elseIfStatements.size())
				{
					auto endLoc = frame.endLocation == INVALID_JUMP_LOCATION ? frame.endLocation = MakeJumpLocation() : frame.endLocation;
					AddInstrNO<JumpInstr>(OpCode_Jump, endLoc);
					SetLocation(frame.nextLocation);

					auto elseIfStatement = elseIfStatements[frame.state];

					auto condition = elseIfStatement->GetCondition();

					MappingStart();
					TraverseExpression(condition);
					MappingEnd(condition->GetSpan());

					frame.nextLocation = MakeJumpLocation();
					AddInstrNO<JumpInstr>(OpCode_JumpZero, frame.nextLocation);

					frame.state++;
					PushFrame(currentFrame);
					PushFrame(ILFrame(elseIfStatement->GetBody()));
					break;
				}
				else
				{
					auto elseStatement = ifStatement->GetElseStatement();
					if (elseStatement)
					{
						auto endLoc = frame.endLocation == INVALID_JUMP_LOCATION ? frame.endLocation = MakeJumpLocation() : frame.endLocation;
						AddInstrNO<JumpInstr>(OpCode_Jump, endLoc);
						SetLocation(frame.nextLocation);

						PushFrame(ILFrame(endLoc));
						PushFrame(ILFrame(elseStatement->GetBody()));
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
				AddInstrNO<JumpInstr>(OpCode_Jump, frame.startLocation);
				SetLocation(frame.endLocation);
				PopLoop();
			}
			break;
			case ILFrameType_DoWhileStatement:
			{
				auto& frame = currentFrame.doWhileFrame;
				SetLocation(frame.condLocation);
				MappingStart();
				TraverseExpression(frame.statement->GetCondition());
				MappingEnd(frame.statement->GetCondition()->GetSpan());
				AddInstrNO<JumpInstr>(OpCode_JumpNotZero, frame.startLocation);
				SetLocation(frame.endLocation);
				PopLoop();
			}
			break;
			case ILFrameType_ForStatement:
			{
				auto& frame = currentFrame.forFrame;
				SetLocation(frame.incrLocation);
				TraverseExpression(frame.statement->GetIteration());
				AddInstrNO<JumpInstr>(OpCode_Jump, frame.startLocation);
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
			AddInstrONO<ReturnInstr>();
		}
	}
}