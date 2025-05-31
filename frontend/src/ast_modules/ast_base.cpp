#include "ast_base.hpp"
#include "declarations.hpp"
#include "statements.hpp"
#include "primitive.hpp"
#include "array.hpp"
#include "compilation_unit.hpp"

namespace HXSL
{
	ASTNode::child_range ASTNode::GetChildrenIt()
	{
		switch (type)
		{
		case NodeType_Unknown:
			break;
		case NodeType_CompilationUnit:
			break;
		case NodeType_LowerCompilationUnit:
			break;
		case NodeType_Namespace:
			break;
		case NodeType_Enum:
			break;
		case NodeType_Primitive:
			break;
		case NodeType_Struct:
			break;
		case NodeType_Class:
			break;
		case NodeType_Array:
			break;
		case NodeType_Pointer:
			break;
		case NodeType_Field:
			break;
		case NodeType_IntrinsicFunction:
			break;
		case NodeType_FunctionOverload:
			break;
		case NodeType_OperatorOverload:
			break;
		case NodeType_ConstructorOverload:
			break;
		case NodeType_Parameter:
			break;
		case NodeType_ThisDef:
			break;
		case NodeType_SwizzleDefinition:
			break;
		case NodeType_AttributeDeclaration:
			break;
		case NodeType_BlockStatement:
			break;
		case NodeType_DeclarationStatement:
			break;
		case NodeType_AssignmentStatement:
			break;
		case NodeType_CompoundAssignmentStatement:
			break;
		case NodeType_ExpressionStatement:
			break;
		case NodeType_ReturnStatement:
			break;
		case NodeType_IfStatement:
			break;
		case NodeType_ElseStatement:
			break;
		case NodeType_ElseIfStatement:
			break;
		case NodeType_WhileStatement:
			break;
		case NodeType_DoWhileStatement:
			break;
		case NodeType_ForStatement:
			break;
		case NodeType_BreakStatement:
			break;
		case NodeType_ContinueStatement:
			break;
		case NodeType_DiscardStatement:
			break;
		case NodeType_SwitchStatement:
			break;
		case NodeType_CaseStatement:
			break;
		case NodeType_DefaultCaseStatement:
			break;
		case NodeType_EmptyExpression:
			break;
		case NodeType_BinaryExpression:
			break;
		case NodeType_LiteralExpression:
			break;
		case NodeType_MemberReferenceExpression:
			break;
		case NodeType_FunctionCallExpression:
			break;
		case NodeType_FunctionCallParameter:
			break;
		case NodeType_MemberAccessExpression:
			break;
		case NodeType_IndexerAccessExpression:
			break;
		case NodeType_CastExpression:
			break;
		case NodeType_TernaryExpression:
			break;
		case NodeType_UnaryExpression:
			break;
		case NodeType_PrefixExpression:
			break;
		case NodeType_PostfixExpression:
			break;
		case NodeType_AssignmentExpression:
			break;
		case NodeType_CompoundAssignmentExpression:
			break;
		case NodeType_InitializationExpression:
			break;
		default:
			break;
		}

		return { nullptr, nullptr };
	}
}