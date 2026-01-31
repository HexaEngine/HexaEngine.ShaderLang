#include "ast_base.hpp"
#include "declarations.hpp"
#include "statements.hpp"
#include "primitive.hpp"
#include "array.hpp"
#include "compilation_unit.hpp"

namespace HXSL
{
	void ASTNode::ForEachChild2(ASTChildCallback cb, void* userdata)
	{
		switch (type)
		{
		case NodeType_CompilationUnit: cast<CompilationUnit>(this)->ForEachChild(cb, userdata); break;
		case NodeType_Namespace: cast<Namespace>(this)->ForEachChild(cb, userdata); break;
		case NodeType_UsingDecl: cast<UsingDecl>(this)->ForEachChild(cb, userdata); break;
		case NodeType_Enum: break; //cast<Enum>(this)->ForEachChild(cb, userdata); break;
		case NodeType_Primitive: cast<Primitive>(this)->ForEachChild(cb, userdata); break;
		case NodeType_Struct: cast<Struct>(this)->ForEachChild(cb, userdata); break;
		case NodeType_Class: cast<Class>(this)->ForEachChild(cb, userdata); break;
		case NodeType_Array: cast<ArrayDecl>(this)->ForEachChild(cb, userdata); break;
		case NodeType_Pointer: cast<Pointer>(this)->ForEachChild(cb, userdata); break;
		case NodeType_Field: cast<Field>(this)->ForEachChild(cb, userdata); break;
		case NodeType_IntrinsicFunction: break; //cast<IntrinsicFunction>(this)->ForEachChild(cb, userdata); break;
		case NodeType_FunctionOverload: cast<FunctionOverload>(this)->ForEachChild(cb, userdata); break;
		case NodeType_OperatorOverload: cast<OperatorOverload>(this)->ForEachChild(cb, userdata); break;
		case NodeType_ConstructorOverload: cast<ConstructorOverload>(this)->ForEachChild(cb, userdata); break;
		case NodeType_Parameter: cast<Parameter>(this)->ForEachChild(cb, userdata); break;
		case NodeType_ThisDef: cast<ThisDef>(this)->ForEachChild(cb, userdata); break;
		case NodeType_SwizzleDefinition: cast<SwizzleDefinition>(this)->ForEachChild(cb, userdata); break;
		case NodeType_AttributeDeclaration: cast<AttributeDecl>(this)->ForEachChild(cb, userdata); break;
		case NodeType_BlockStatement: cast<BlockStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_DeclarationStatement: cast<DeclarationStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_AssignmentStatement: cast<AssignmentStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_CompoundAssignmentStatement: cast<CompoundAssignmentStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_ExpressionStatement: cast<ExpressionStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_ReturnStatement: cast<ReturnStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_IfStatement: cast<IfStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_ElseStatement: cast<ElseStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_ElseIfStatement: cast<ElseIfStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_WhileStatement: cast<WhileStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_DoWhileStatement: cast<DoWhileStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_ForStatement: cast<ForStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_BreakStatement: cast<BreakStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_ContinueStatement: cast<ContinueStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_DiscardStatement: cast<DiscardStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_SwitchStatement: cast<SwitchStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_CaseStatement: cast<CaseStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_DefaultCaseStatement: cast<DefaultCaseStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_EmptyExpression: cast<EmptyExpression>(this)->ForEachChild(cb, userdata); break;
		case NodeType_BinaryExpression: cast<BinaryExpression>(this)->ForEachChild(cb, userdata); break;
		case NodeType_LiteralExpression: cast<LiteralExpression>(this)->ForEachChild(cb, userdata); break;
		case NodeType_MemberReferenceExpression: cast<MemberReferenceExpression>(this)->ForEachChild(cb, userdata); break;
		case NodeType_FunctionCallExpression: cast<FunctionCallExpression>(this)->ForEachChild(cb, userdata); break;
		case NodeType_ConstructorCallExpression: cast<ConstructorCallExpression>(this)->ForEachChild(cb, userdata); break;
		case NodeType_FunctionCallParameter: cast<FunctionCallParameter>(this)->ForEachChild(cb, userdata); break;
		case NodeType_MemberAccessExpression: cast<MemberAccessExpression>(this)->ForEachChild(cb, userdata); break;
		case NodeType_IndexerAccessExpression: cast<IndexerAccessExpression>(this)->ForEachChild(cb, userdata); break;
		case NodeType_CastExpression: cast<CastExpression>(this)->ForEachChild(cb, userdata); break;
		case NodeType_TernaryExpression: cast<TernaryExpression>(this)->ForEachChild(cb, userdata); break;
		case NodeType_PrefixExpression: cast<PrefixExpression>(this)->ForEachChild(cb, userdata); break;
		case NodeType_PostfixExpression: cast<PostfixExpression>(this)->ForEachChild(cb, userdata); break;
		case NodeType_AssignmentExpression: cast<AssignmentExpression>(this)->ForEachChild(cb, userdata); break;
		case NodeType_CompoundAssignmentExpression: cast<CompoundAssignmentExpression>(this)->ForEachChild(cb, userdata); break;
		case NodeType_InitializationExpression: cast<InitializationExpression>(this)->ForEachChild(cb, userdata); break;
		default:
			HXSL_ASSERT(false, "Unhandled ASTNode type in ForEachChild.");
			break;
		}
	}

	void ASTNode::ForEachChild2(ASTConstChildCallback cb, void* userdata) const
	{
		switch (type)
		{
		case NodeType_CompilationUnit: cast<CompilationUnit>(this)->ForEachChild(cb, userdata); break;
		case NodeType_Namespace: cast<Namespace>(this)->ForEachChild(cb, userdata); break;
		case NodeType_UsingDecl: cast<UsingDecl>(this)->ForEachChild(cb, userdata); break;
		case NodeType_Enum: break; //cast<Enum>(this)->ForEachChild(cb, userdata); break;
		case NodeType_Primitive: cast<Primitive>(this)->ForEachChild(cb, userdata); break;
		case NodeType_Struct: cast<Struct>(this)->ForEachChild(cb, userdata); break;
		case NodeType_Class: cast<Class>(this)->ForEachChild(cb, userdata); break;
		case NodeType_Array: cast<ArrayDecl>(this)->ForEachChild(cb, userdata); break;
		case NodeType_Pointer: cast<Pointer>(this)->ForEachChild(cb, userdata); break;
		case NodeType_Field: cast<Field>(this)->ForEachChild(cb, userdata); break;
		case NodeType_IntrinsicFunction: break; //cast<IntrinsicFunction>(this)->ForEachChild(cb, userdata); break;
		case NodeType_FunctionOverload: cast<FunctionOverload>(this)->ForEachChild(cb, userdata); break;
		case NodeType_OperatorOverload: cast<OperatorOverload>(this)->ForEachChild(cb, userdata); break;
		case NodeType_ConstructorOverload: cast<ConstructorOverload>(this)->ForEachChild(cb, userdata); break;
		case NodeType_Parameter: cast<Parameter>(this)->ForEachChild(cb, userdata); break;
		case NodeType_ThisDef: cast<ThisDef>(this)->ForEachChild(cb, userdata); break;
		case NodeType_SwizzleDefinition: cast<SwizzleDefinition>(this)->ForEachChild(cb, userdata); break;
		case NodeType_AttributeDeclaration: cast<AttributeDecl>(this)->ForEachChild(cb, userdata); break;
		case NodeType_BlockStatement: cast<BlockStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_DeclarationStatement: cast<DeclarationStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_AssignmentStatement: cast<AssignmentStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_CompoundAssignmentStatement: cast<CompoundAssignmentStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_ExpressionStatement: cast<ExpressionStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_ReturnStatement: cast<ReturnStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_IfStatement: cast<IfStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_ElseStatement: cast<ElseStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_ElseIfStatement: cast<ElseIfStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_WhileStatement: cast<WhileStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_DoWhileStatement: cast<DoWhileStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_ForStatement: cast<ForStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_BreakStatement: cast<BreakStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_ContinueStatement: cast<ContinueStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_DiscardStatement: cast<DiscardStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_SwitchStatement: cast<SwitchStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_CaseStatement: cast<CaseStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_DefaultCaseStatement: cast<DefaultCaseStatement>(this)->ForEachChild(cb, userdata); break;
		case NodeType_EmptyExpression: cast<EmptyExpression>(this)->ForEachChild(cb, userdata); break;
		case NodeType_BinaryExpression: cast<BinaryExpression>(this)->ForEachChild(cb, userdata); break;
		case NodeType_LiteralExpression: cast<LiteralExpression>(this)->ForEachChild(cb, userdata); break;
		case NodeType_MemberReferenceExpression: cast<MemberReferenceExpression>(this)->ForEachChild(cb, userdata); break;
		case NodeType_FunctionCallExpression: cast<FunctionCallExpression>(this)->ForEachChild(cb, userdata); break;
		case NodeType_ConstructorCallExpression: cast<ConstructorCallExpression>(this)->ForEachChild(cb, userdata); break;
		case NodeType_FunctionCallParameter: cast<FunctionCallParameter>(this)->ForEachChild(cb, userdata); break;
		case NodeType_MemberAccessExpression: cast<MemberAccessExpression>(this)->ForEachChild(cb, userdata); break;
		case NodeType_IndexerAccessExpression: cast<IndexerAccessExpression>(this)->ForEachChild(cb, userdata); break;
		case NodeType_CastExpression: cast<CastExpression>(this)->ForEachChild(cb, userdata); break;
		case NodeType_TernaryExpression: cast<TernaryExpression>(this)->ForEachChild(cb, userdata); break;
		case NodeType_PrefixExpression: cast<PrefixExpression>(this)->ForEachChild(cb, userdata); break;
		case NodeType_PostfixExpression: cast<PostfixExpression>(this)->ForEachChild(cb, userdata); break;
		case NodeType_AssignmentExpression: cast<AssignmentExpression>(this)->ForEachChild(cb, userdata); break;
		case NodeType_CompoundAssignmentExpression: cast<CompoundAssignmentExpression>(this)->ForEachChild(cb, userdata); break;
		case NodeType_InitializationExpression: cast<InitializationExpression>(this)->ForEachChild(cb, userdata); break;
		default:
			HXSL_ASSERT(false, "Unhandled ASTNode type in ForEachChild.");
			break;
		}
	}

	std::string ASTNode::DebugName() const
	{
		if (type == NodeType_Unknown)
		{
			HXSL_ASSERT(false, "ASTNode with Unknown type has no DebugName.");
			return "[Unknown ASTNode]";
		}
		switch (type)
		{
		case NodeType_CompilationUnit: return cast<CompilationUnit>(this)->DebugName();
		case NodeType_Namespace: return cast<Namespace>(this)->DebugName();
		case NodeType_FunctionOverload: return cast<FunctionOverload>(this)->DebugName();
		case NodeType_ConstructorOverload: return cast<ConstructorOverload>(this)->DebugName();
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