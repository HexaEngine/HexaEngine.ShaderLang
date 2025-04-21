#ifndef OPERATOR_H
#define OPERATOR_H

#include <string>
namespace HXSL
{
	enum HXSLOperator : int
	{
		HXSLOperator_Unknown,
		HXSLOperator_Add,
		HXSLOperator_Subtract,
		HXSLOperator_Multiply,
		HXSLOperator_Divide,
		HXSLOperator_Modulus,
		HXSLOperator_Assign,
		HXSLOperator_PlusAssign,
		HXSLOperator_MinusAssign,
		HXSLOperator_MultiplyAssign,
		HXSLOperator_DivideAssign,
		HXSLOperator_ModulusAssign,
		HXSLOperator_BitwiseNot,
		HXSLOperator_BitwiseShiftLeft,
		HXSLOperator_BitwiseShiftRight,
		HXSLOperator_BitwiseAnd,
		HXSLOperator_BitwiseOr,
		HXSLOperator_BitwiseXor,
		HXSLOperator_BitwiseShiftLeftAssign,
		HXSLOperator_BitwiseShiftRightAssign,
		HXSLOperator_BitwiseAndAssign,
		HXSLOperator_BitwiseOrAssign,
		HXSLOperator_BitwiseXorAssign,
		HXSLOperator_AndAnd,
		HXSLOperator_OrOr,
		HXSLOperator_Ternary,
		HXSLOperator_TernaryElse,
		HXSLOperator_LessThan,
		HXSLOperator_GreaterThan,
		HXSLOperator_Equal,
		HXSLOperator_NotEqual,
		HXSLOperator_LessThanOrEqual,
		HXSLOperator_GreaterThanOrEqual,
		HXSLOperator_Increment,
		HXSLOperator_Decrement,
		HXSLOperator_MemberAccess,
		HXSLOperator_LogicalNot,

		HXSLOperator_Colon = HXSLOperator_TernaryElse
	};

	static std::string ToString(HXSLOperator op)
	{
		switch (op)
		{
		case HXSLOperator_Add: return "+";
		case HXSLOperator_Subtract: return "-";
		case HXSLOperator_Multiply: return "*";
		case HXSLOperator_Divide: return "/";
		case HXSLOperator_Modulus: return "%";
		case HXSLOperator_Assign: return "=";
		case HXSLOperator_PlusAssign: return "+=";
		case HXSLOperator_MinusAssign: return "-=";
		case HXSLOperator_MultiplyAssign: return "*=";
		case HXSLOperator_DivideAssign: return "/=";
		case HXSLOperator_ModulusAssign: return "%=";
		case HXSLOperator_BitwiseNot: return "~";
		case HXSLOperator_BitwiseShiftLeft: return "<<";
		case HXSLOperator_BitwiseShiftRight: return ">>";
		case HXSLOperator_BitwiseAnd: return "&";
		case HXSLOperator_BitwiseOr: return "|";
		case HXSLOperator_BitwiseXor: return "^";
		case HXSLOperator_BitwiseShiftLeftAssign: return "<<=";
		case HXSLOperator_BitwiseShiftRightAssign: return ">>=";
		case HXSLOperator_BitwiseAndAssign: return "&=";
		case HXSLOperator_BitwiseOrAssign: return "|=";
		case HXSLOperator_BitwiseXorAssign: return "^=";
		case HXSLOperator_AndAnd: return "&&";
		case HXSLOperator_OrOr: return "||";
		case HXSLOperator_Ternary: return "?";
		case HXSLOperator_TernaryElse: return ":";
		case HXSLOperator_LessThan: return "<";
		case HXSLOperator_GreaterThan: return ">";
		case HXSLOperator_Equal: return "==";
		case HXSLOperator_NotEqual: return "!=";
		case HXSLOperator_LessThanOrEqual: return "<=";
		case HXSLOperator_GreaterThanOrEqual: return ">=";
		case HXSLOperator_Increment: return "++";
		case HXSLOperator_Decrement: return "--";
		case HXSLOperator_MemberAccess: return ".";
		case HXSLOperator_LogicalNot: return "!";
		default: return "Unknown";
		}
	}

#include "tst.hpp"

	static void BuildOperatorTST(TernarySearchTreeDictionary<int>* tst) {
		tst->Insert("+", HXSLOperator_Add);
		tst->Insert("-", HXSLOperator_Subtract);
		tst->Insert("*", HXSLOperator_Multiply);
		tst->Insert("/", HXSLOperator_Divide);
		tst->Insert("%", HXSLOperator_Modulus);
		tst->Insert("=", HXSLOperator_Assign);
		tst->Insert("+=", HXSLOperator_PlusAssign);
		tst->Insert("-=", HXSLOperator_MinusAssign);
		tst->Insert("*=", HXSLOperator_MultiplyAssign);
		tst->Insert("/=", HXSLOperator_DivideAssign);
		tst->Insert("%=", HXSLOperator_ModulusAssign);
		tst->Insert("~", HXSLOperator_BitwiseNot);
		tst->Insert("<<", HXSLOperator_BitwiseShiftLeft);
		tst->Insert(">>", HXSLOperator_BitwiseShiftRight);
		tst->Insert("&", HXSLOperator_BitwiseAnd);
		tst->Insert("|", HXSLOperator_BitwiseOr);
		tst->Insert("^", HXSLOperator_BitwiseXor);
		tst->Insert("<<=", HXSLOperator_BitwiseShiftLeftAssign);
		tst->Insert(">>=", HXSLOperator_BitwiseShiftRightAssign);
		tst->Insert("&=", HXSLOperator_BitwiseAndAssign);
		tst->Insert("|=", HXSLOperator_BitwiseOrAssign);
		tst->Insert("^=", HXSLOperator_BitwiseXorAssign);
		tst->Insert("&&", HXSLOperator_AndAnd);
		tst->Insert("||", HXSLOperator_OrOr);
		tst->Insert("?", HXSLOperator_Ternary);
		tst->Insert(":", HXSLOperator_TernaryElse);
		tst->Insert("<", HXSLOperator_LessThan);
		tst->Insert(">", HXSLOperator_GreaterThan);
		tst->Insert("==", HXSLOperator_Equal);
		tst->Insert("!=", HXSLOperator_NotEqual);
		tst->Insert("<=", HXSLOperator_LessThanOrEqual);
		tst->Insert(">=", HXSLOperator_GreaterThanOrEqual);
		tst->Insert("++", HXSLOperator_Increment);
		tst->Insert("--", HXSLOperator_Decrement);
		tst->Insert(".", HXSLOperator_MemberAccess);
		tst->Insert("!", HXSLOperator_LogicalNot);
	}

	namespace Operators
	{
		static int GetOperatorPrecedence(HXSLOperator op) {
			switch (op)
			{
			case HXSLOperator_BitwiseShiftLeftAssign:
			case HXSLOperator_BitwiseShiftRightAssign:
			case HXSLOperator_BitwiseAndAssign:
			case HXSLOperator_BitwiseOrAssign:
			case HXSLOperator_BitwiseXorAssign:
			case HXSLOperator_Assign:
			case HXSLOperator_PlusAssign:
			case HXSLOperator_MinusAssign:
			case HXSLOperator_MultiplyAssign:
			case HXSLOperator_DivideAssign:
			case HXSLOperator_ModulusAssign:
				return 0;

			case HXSLOperator_Ternary:
				return 5;

			case HXSLOperator_LessThan:
			case HXSLOperator_GreaterThan:
			case HXSLOperator_Equal:
			case HXSLOperator_NotEqual:
			case HXSLOperator_LessThanOrEqual:
			case HXSLOperator_GreaterThanOrEqual:
				return 10;

			case HXSLOperator_OrOr:
				return 15;

			case HXSLOperator_AndAnd:
				return 20;

			case HXSLOperator_BitwiseShiftLeft:
			case HXSLOperator_BitwiseShiftRight:
			case HXSLOperator_BitwiseAnd:
			case HXSLOperator_BitwiseOr:
			case HXSLOperator_BitwiseXor:
				return 25;

			case HXSLOperator_Add:
			case HXSLOperator_Subtract:
				return 30;

			case HXSLOperator_Multiply:
			case HXSLOperator_Divide:
			case HXSLOperator_Modulus:
				return 35;

			case HXSLOperator_LogicalNot:
			case HXSLOperator_BitwiseNot:
			case HXSLOperator_Increment:
			case HXSLOperator_Decrement:
				return 40;

			case HXSLOperator_MemberAccess:
				return 45;

			default:
				return -1;
			}
		}

		static bool isUnaryOperator(HXSLOperator op)
		{
			switch (op)
			{
			case HXSLOperator_Subtract:
			case HXSLOperator_LogicalNot:
			case HXSLOperator_BitwiseNot:
			case HXSLOperator_Increment:
			case HXSLOperator_Decrement:
				return true;
			default:
				return false;
			}
		}

		static bool isTernaryOperator(HXSLOperator op)
		{
			return op == HXSLOperator_Ternary;
		}

		static bool isMemberAccessOperator(HXSLOperator op)
		{
			return op == HXSLOperator_MemberAccess;
		}

		static bool isCompoundAssignment(HXSLOperator op)
		{
			switch (op)
			{
			case HXSLOperator_PlusAssign:
			case HXSLOperator_MinusAssign:
			case HXSLOperator_MultiplyAssign:
			case HXSLOperator_DivideAssign:
			case HXSLOperator_ModulusAssign:
			case HXSLOperator_BitwiseShiftLeftAssign:
			case HXSLOperator_BitwiseShiftRightAssign:
			case HXSLOperator_BitwiseAndAssign:
			case HXSLOperator_BitwiseOrAssign:
			case HXSLOperator_BitwiseXorAssign:
				return true;
			default:
				return false;
			}
		}
	}
}

#endif