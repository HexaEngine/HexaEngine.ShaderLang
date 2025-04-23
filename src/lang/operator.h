#ifndef OPERATOR_H
#define OPERATOR_H

#include <string>

#include "tst.hpp"

namespace HXSL
{
	enum Operator : int
	{
		Operator_Unknown,
		Operator_Add,
		Operator_Subtract,
		Operator_Multiply,
		Operator_Divide,
		Operator_Modulus,
		Operator_Assign,
		Operator_PlusAssign,
		Operator_MinusAssign,
		Operator_MultiplyAssign,
		Operator_DivideAssign,
		Operator_ModulusAssign,
		Operator_BitwiseNot,
		Operator_BitwiseShiftLeft,
		Operator_BitwiseShiftRight,
		Operator_BitwiseAnd,
		Operator_BitwiseOr,
		Operator_BitwiseXor,
		Operator_BitwiseShiftLeftAssign,
		Operator_BitwiseShiftRightAssign,
		Operator_BitwiseAndAssign,
		Operator_BitwiseOrAssign,
		Operator_BitwiseXorAssign,
		Operator_AndAnd,
		Operator_OrOr,
		Operator_Ternary,
		Operator_TernaryElse,
		Operator_LessThan,
		Operator_GreaterThan,
		Operator_Equal,
		Operator_NotEqual,
		Operator_LessThanOrEqual,
		Operator_GreaterThanOrEqual,
		Operator_Increment,
		Operator_Decrement,
		Operator_MemberAccess,
		Operator_LogicalNot,
		Operator_Cast,

		Operator_Colon = Operator_TernaryElse
	};

	static std::string ToString(Operator op)
	{
		switch (op)
		{
		case Operator_Add: return "+";
		case Operator_Subtract: return "-";
		case Operator_Multiply: return "*";
		case Operator_Divide: return "/";
		case Operator_Modulus: return "%";
		case Operator_Assign: return "=";
		case Operator_PlusAssign: return "+=";
		case Operator_MinusAssign: return "-=";
		case Operator_MultiplyAssign: return "*=";
		case Operator_DivideAssign: return "/=";
		case Operator_ModulusAssign: return "%=";
		case Operator_BitwiseNot: return "~";
		case Operator_BitwiseShiftLeft: return "<<";
		case Operator_BitwiseShiftRight: return ">>";
		case Operator_BitwiseAnd: return "&";
		case Operator_BitwiseOr: return "|";
		case Operator_BitwiseXor: return "^";
		case Operator_BitwiseShiftLeftAssign: return "<<=";
		case Operator_BitwiseShiftRightAssign: return ">>=";
		case Operator_BitwiseAndAssign: return "&=";
		case Operator_BitwiseOrAssign: return "|=";
		case Operator_BitwiseXorAssign: return "^=";
		case Operator_AndAnd: return "&&";
		case Operator_OrOr: return "||";
		case Operator_Ternary: return "?";
		case Operator_TernaryElse: return ":";
		case Operator_LessThan: return "<";
		case Operator_GreaterThan: return ">";
		case Operator_Equal: return "==";
		case Operator_NotEqual: return "!=";
		case Operator_LessThanOrEqual: return "<=";
		case Operator_GreaterThanOrEqual: return ">=";
		case Operator_Increment: return "++";
		case Operator_Decrement: return "--";
		case Operator_MemberAccess: return ".";
		case Operator_LogicalNot: return "!";
		default: return "Unknown";
		}
	}

	static void BuildOperatorTST(TernarySearchTreeDictionary<int>* tst) {
		tst->Insert("+", Operator_Add);
		tst->Insert("-", Operator_Subtract);
		tst->Insert("*", Operator_Multiply);
		tst->Insert("/", Operator_Divide);
		tst->Insert("%", Operator_Modulus);
		tst->Insert("=", Operator_Assign);
		tst->Insert("+=", Operator_PlusAssign);
		tst->Insert("-=", Operator_MinusAssign);
		tst->Insert("*=", Operator_MultiplyAssign);
		tst->Insert("/=", Operator_DivideAssign);
		tst->Insert("%=", Operator_ModulusAssign);
		tst->Insert("~", Operator_BitwiseNot);
		tst->Insert("<<", Operator_BitwiseShiftLeft);
		tst->Insert(">>", Operator_BitwiseShiftRight);
		tst->Insert("&", Operator_BitwiseAnd);
		tst->Insert("|", Operator_BitwiseOr);
		tst->Insert("^", Operator_BitwiseXor);
		tst->Insert("<<=", Operator_BitwiseShiftLeftAssign);
		tst->Insert(">>=", Operator_BitwiseShiftRightAssign);
		tst->Insert("&=", Operator_BitwiseAndAssign);
		tst->Insert("|=", Operator_BitwiseOrAssign);
		tst->Insert("^=", Operator_BitwiseXorAssign);
		tst->Insert("&&", Operator_AndAnd);
		tst->Insert("||", Operator_OrOr);
		tst->Insert("?", Operator_Ternary);
		tst->Insert(":", Operator_TernaryElse);
		tst->Insert("<", Operator_LessThan);
		tst->Insert(">", Operator_GreaterThan);
		tst->Insert("==", Operator_Equal);
		tst->Insert("!=", Operator_NotEqual);
		tst->Insert("<=", Operator_LessThanOrEqual);
		tst->Insert(">=", Operator_GreaterThanOrEqual);
		tst->Insert("++", Operator_Increment);
		tst->Insert("--", Operator_Decrement);
		tst->Insert(".", Operator_MemberAccess);
		tst->Insert("!", Operator_LogicalNot);
	}

	namespace Operators
	{
		static bool IsValidOverloadOperator(Operator op)
		{
			switch (op) {
			case Operator_Increment:
			case Operator_Decrement:
			case Operator_LogicalNot:
			case Operator_BitwiseNot:
			case Operator_Add:
			case Operator_Subtract:
			case Operator_Multiply:
			case Operator_Divide:
			case Operator_Modulus:
			case Operator_BitwiseShiftLeft:
			case Operator_BitwiseShiftRight:
			case Operator_BitwiseAnd:
			case Operator_BitwiseOr:
			case Operator_BitwiseXor:
			case Operator_LessThan:
			case Operator_GreaterThan:
			case Operator_Equal:
			case Operator_NotEqual:
			case Operator_LessThanOrEqual:
			case Operator_GreaterThanOrEqual:
			case Operator_AndAnd:
			case Operator_OrOr:
				return true;
			default:
				return false;
			}
		}

		static size_t GetNumOverloadParams(Operator op)
		{
			switch (op) {
			case Operator_Increment:
			case Operator_Decrement:
			case Operator_LogicalNot:
			case Operator_BitwiseNot:
				return 1;

			case Operator_Add:
			case Operator_Subtract:
			case Operator_Multiply:
			case Operator_Divide:
			case Operator_Modulus:
			case Operator_BitwiseShiftLeft:
			case Operator_BitwiseShiftRight:
			case Operator_BitwiseAnd:
			case Operator_BitwiseOr:
			case Operator_BitwiseXor:
			case Operator_LessThan:
			case Operator_GreaterThan:
			case Operator_Equal:
			case Operator_NotEqual:
			case Operator_LessThanOrEqual:
			case Operator_GreaterThanOrEqual:
			case Operator_AndAnd:
			case Operator_OrOr:
				return 2;
			default:
				return 0;
			}
		}

		static int GetOperatorPrecedence(Operator op) {
			switch (op)
			{
			case Operator_BitwiseShiftLeftAssign:
			case Operator_BitwiseShiftRightAssign:
			case Operator_BitwiseAndAssign:
			case Operator_BitwiseOrAssign:
			case Operator_BitwiseXorAssign:
			case Operator_Assign:
			case Operator_PlusAssign:
			case Operator_MinusAssign:
			case Operator_MultiplyAssign:
			case Operator_DivideAssign:
			case Operator_ModulusAssign:
				return 0;

			case Operator_Ternary:
				return 5;

			case Operator_LessThan:
			case Operator_GreaterThan:
			case Operator_Equal:
			case Operator_NotEqual:
			case Operator_LessThanOrEqual:
			case Operator_GreaterThanOrEqual:
				return 10;

			case Operator_OrOr:
				return 15;

			case Operator_AndAnd:
				return 20;

			case Operator_BitwiseShiftLeft:
			case Operator_BitwiseShiftRight:
			case Operator_BitwiseAnd:
			case Operator_BitwiseOr:
			case Operator_BitwiseXor:
				return 25;

			case Operator_Add:
			case Operator_Subtract:
				return 30;

			case Operator_Multiply:
			case Operator_Divide:
			case Operator_Modulus:
				return 35;

			case Operator_LogicalNot:
			case Operator_BitwiseNot:
			case Operator_Increment:
			case Operator_Decrement:
				return 40;

			case Operator_MemberAccess:
				return 45;

			default:
				return -1;
			}
		}

		static bool isUnaryOperator(Operator op)
		{
			switch (op)
			{
			case Operator_Subtract:
			case Operator_LogicalNot:
			case Operator_BitwiseNot:
			case Operator_Increment:
			case Operator_Decrement:
				return true;
			default:
				return false;
			}
		}

		static bool isTernaryOperator(Operator op)
		{
			return op == Operator_Ternary;
		}

		static bool isMemberAccessOperator(Operator op)
		{
			return op == Operator_MemberAccess;
		}

		static bool isCompoundAssignment(Operator op)
		{
			switch (op)
			{
			case Operator_PlusAssign:
			case Operator_MinusAssign:
			case Operator_MultiplyAssign:
			case Operator_DivideAssign:
			case Operator_ModulusAssign:
			case Operator_BitwiseShiftLeftAssign:
			case Operator_BitwiseShiftRightAssign:
			case Operator_BitwiseAndAssign:
			case Operator_BitwiseOrAssign:
			case Operator_BitwiseXorAssign:
				return true;
			default:
				return false;
			}
		}
	}
}

#endif