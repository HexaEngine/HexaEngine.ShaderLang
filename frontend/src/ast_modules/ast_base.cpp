#include "ast_base.hpp"
#include "declarations.hpp"
#include "statements.hpp"
#include "primitive.hpp"
#include "array.hpp"
#include "compilation_unit.hpp"

namespace HXSL
{
	/*
	ASTNode::child_range ASTNode::GetChildrenIt()
	{
		switch (type)
		{
		case NodeType_Unknown: break;
		case NodeType_CompilationUnit: return cast<CompilationUnit>(this)->GetChildrenIt();
		case NodeType_Namespace: return cast<Namespace>(this)->GetChildrenIt();
		case NodeType_Enum: break; //return cast<Enum>(this)->GetChildrenIt();
		case NodeType_Primitive: return cast<Primitive>(this)->GetChildrenIt();
		case NodeType_Struct: return cast<Struct>(this)->GetChildrenIt();
		case NodeType_Class: return cast<Class>(this)->GetChildrenIt();
		case NodeType_Array: return cast<ArrayDecl>(this)->GetChildrenIt();
		case NodeType_Pointer: return cast<Pointer>(this)->GetChildrenIt();
		case NodeType_Field: return cast<Field>(this)->GetChildrenIt();
		case NodeType_IntrinsicFunction: break; //return cast<IntrinsicFunction>(this)->GetChildrenIt();
		case NodeType_FunctionOverload: return cast<FunctionOverload>(this)->GetChildrenIt();
		case NodeType_OperatorOverload: return cast<OperatorOverload>(this)->GetChildrenIt();
		case NodeType_ConstructorOverload: return cast<ConstructorOverload>(this)->GetChildrenIt();
		case NodeType_Parameter: return cast<Parameter>(this)->GetChildrenIt();
		case NodeType_ThisDef: return cast<ThisDef>(this)->GetChildrenIt();
		case NodeType_SwizzleDefinition: return cast<SwizzleDefinition>(this)->GetChildrenIt();
		case NodeType_AttributeDeclaration: return cast<AttributeDecl>(this)->GetChildrenIt();
		case NodeType_BlockStatement: return cast<BlockStatement>(this)->GetChildrenIt();
		case NodeType_DeclarationStatement: return cast<DeclarationStatement>(this)->GetChildrenIt();
		case NodeType_AssignmentStatement: return cast<AssignmentStatement>(this)->GetChildrenIt();
		case NodeType_CompoundAssignmentStatement: return cast<CompoundAssignmentStatement>(this)->GetChildrenIt();
		case NodeType_ExpressionStatement: return cast<ExpressionStatement>(this)->GetChildrenIt();
		case NodeType_ReturnStatement: return cast<ReturnStatement>(this)->GetChildrenIt();
		case NodeType_IfStatement: return cast<IfStatement>(this)->GetChildrenIt();
		case NodeType_ElseStatement: return cast<ElseStatement>(this)->GetChildrenIt();
		case NodeType_ElseIfStatement: return cast<ElseIfStatement>(this)->GetChildrenIt();
		case NodeType_WhileStatement: return cast<WhileStatement>(this)->GetChildrenIt();
		case NodeType_DoWhileStatement: return cast<DoWhileStatement>(this)->GetChildrenIt();
		case NodeType_ForStatement: return cast<ForStatement>(this)->GetChildrenIt();
		case NodeType_BreakStatement: return cast<BreakStatement>(this)->GetChildrenIt();
		case NodeType_ContinueStatement: return cast<ContinueStatement>(this)->GetChildrenIt();
		case NodeType_DiscardStatement: return cast<DiscardStatement>(this)->GetChildrenIt();
		case NodeType_SwitchStatement: return cast<SwitchStatement>(this)->GetChildrenIt();
		case NodeType_CaseStatement: return cast<CaseStatement>(this)->GetChildrenIt();
		case NodeType_DefaultCaseStatement: return cast<DefaultCaseStatement>(this)->GetChildrenIt();
		case NodeType_EmptyExpression: return cast<EmptyExpression>(this)->GetChildrenIt();
		case NodeType_BinaryExpression: return cast<EmptyExpression>(this)->GetChildrenIt();
		case NodeType_LiteralExpression: return cast<LiteralExpression>(this)->GetChildrenIt();
		case NodeType_MemberReferenceExpression: return cast<MemberReferenceExpression>(this)->GetChildrenIt();
		case NodeType_FunctionCallExpression: return cast<FunctionCallExpression>(this)->GetChildrenIt();
		case NodeType_FunctionCallParameter: return cast<FunctionCallParameter>(this)->GetChildrenIt();
		case NodeType_MemberAccessExpression: return cast<MemberAccessExpression>(this)->GetChildrenIt();
		case NodeType_IndexerAccessExpression: return cast<IndexerAccessExpression>(this)->GetChildrenIt();
		case NodeType_CastExpression: return cast<CastExpression>(this)->GetChildrenIt();
		case NodeType_TernaryExpression: return cast<TernaryExpression>(this)->GetChildrenIt();
		case NodeType_PrefixExpression: return cast<PrefixExpression>(this)->GetChildrenIt();
		case NodeType_PostfixExpression: return cast<PostfixExpression>(this)->GetChildrenIt();
		case NodeType_AssignmentExpression: return cast<AssignmentExpression>(this)->GetChildrenIt();
		case NodeType_CompoundAssignmentExpression: return cast<CompoundAssignmentExpression>(this)->GetChildrenIt();
		case NodeType_InitializationExpression: return cast<InitializationExpression>(this)->GetChildrenIt();
		default:
			break;
		}

		return { nullptr, nullptr };
	}*/

	std::string ASTNode::DebugName() const
	{
		switch (type)
		{
		case NodeType_FunctionOverload: return cast<FunctionOverload>(this)->DebugName();
		case NodeType_Struct: return cast<Struct>(this)->DebugName();
		case NodeType_Class: return cast<Class>(this)->DebugName();
		case NodeType_BlockStatement: return cast<BlockStatement>(this)->DebugName();
		case NodeType_ElseStatement: return cast<ElseStatement>(this)->DebugName();
		case NodeType_ElseIfStatement: return cast<ElseIfStatement>(this)->DebugName();
		case NodeType_IfStatement: return cast<IfStatement>(this)->DebugName();
		case NodeType_CaseStatement: return cast<CaseStatement>(this)->DebugName();
		case NodeType_SwitchStatement: return cast<SwitchStatement>(this)->DebugName();
		case NodeType_ForStatement: return cast<ForStatement>(this)->DebugName();
		case NodeType_WhileStatement: return cast<WhileStatement>(this)->DebugName();
		case NodeType_DoWhileStatement: return cast<DoWhileStatement>(this)->DebugName();
		default:
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] Span: " + span.str();
			return oss.str();
		}
		break;
		}
	}
}