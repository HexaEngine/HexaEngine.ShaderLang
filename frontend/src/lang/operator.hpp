#ifndef OPERATOR_HPP
#define OPERATOR_HPP

#include "pch/std.hpp"
#include "utils/radix_tree.hpp"
#include "core/operator.hpp"

namespace HXSL
{
	constexpr int Operator_BinaryRangeStart = Operator_Add;
	constexpr int Operator_BinaryRangeEnd = Operator_NotEqual;

	constexpr int Operator_UnaryRangeStart = Operator_Increment;
	constexpr int Operator_UnaryRangeEnd = Operator_BitwiseNot;

	constexpr int Operator_AssignmentRangeStart = Operator_Assign;
	constexpr int Operator_AssignmentRangeEnd = Operator_BitwiseXorAssign;

	constexpr int Operator_CompoundAssignmentRangeStart = Operator_PlusAssign;
	constexpr int Operator_CompoundAssignmentRangeEnd = Operator_BitwiseXorAssign;

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

	static char ToLookupChar(Operator op)
	{
		switch (op) {
		case Operator_Increment: return '\x1';
		case Operator_Decrement: return '\x2';
		case Operator_LogicalNot: return '\x3';
		case Operator_BitwiseNot: return '\x4';
		case Operator_Add: return '\x5';
		case Operator_Subtract:  return '\x6';
		case Operator_Multiply: return '\x7';
		case Operator_Divide: return '\x8';
		case Operator_Modulus: return '\x9';
		case Operator_BitwiseShiftLeft: return '\xA';
		case Operator_BitwiseShiftRight: return '\xB';
		case Operator_BitwiseAnd: return '\xC';
		case Operator_BitwiseOr: return '\xD';
		case Operator_BitwiseXor: return '\xF';
		case Operator_LessThan: return '\x10';
		case Operator_GreaterThan: return '\x11';
		case Operator_Equal: return '\x12';
		case Operator_NotEqual: return '\x13';
		case Operator_LessThanOrEqual: return '\x14';
		case Operator_GreaterThanOrEqual: return '\x15';
		case Operator_AndAnd: return '\x16';
		case Operator_OrOr: return '\x17';
		case Operator_Cast: return '\x18';
		default:
			return '\x1F';
		}
	}

	static void BuildOperatorRadix(RadixTree<int>& t)
	{
		t.Insert("+", Operator_Add);
		t.Insert("-", Operator_Subtract);
		t.Insert("*", Operator_Multiply);
		t.Insert("/", Operator_Divide);
		t.Insert("%", Operator_Modulus);
		t.Insert("=", Operator_Assign);
		t.Insert("+=", Operator_PlusAssign);
		t.Insert("-=", Operator_MinusAssign);
		t.Insert("*=", Operator_MultiplyAssign);
		t.Insert("/=", Operator_DivideAssign);
		t.Insert("%=", Operator_ModulusAssign);
		t.Insert("~", Operator_BitwiseNot);
		t.Insert("<<", Operator_BitwiseShiftLeft);
		t.Insert(">>", Operator_BitwiseShiftRight);
		t.Insert("&", Operator_BitwiseAnd);
		t.Insert("|", Operator_BitwiseOr);
		t.Insert("^", Operator_BitwiseXor);
		t.Insert("<<=", Operator_BitwiseShiftLeftAssign);
		t.Insert(">>=", Operator_BitwiseShiftRightAssign);
		t.Insert("&=", Operator_BitwiseAndAssign);
		t.Insert("|=", Operator_BitwiseOrAssign);
		t.Insert("^=", Operator_BitwiseXorAssign);
		t.Insert("&&", Operator_AndAnd);
		t.Insert("||", Operator_OrOr);
		t.Insert("?", Operator_Ternary);
		t.Insert(":", Operator_TernaryElse);
		t.Insert("<", Operator_LessThan);
		t.Insert(">", Operator_GreaterThan);
		t.Insert("==", Operator_Equal);
		t.Insert("!=", Operator_NotEqual);
		t.Insert("<=", Operator_LessThanOrEqual);
		t.Insert(">=", Operator_GreaterThanOrEqual);
		t.Insert("++", Operator_Increment);
		t.Insert("--", Operator_Decrement);
		t.Insert(".", Operator_MemberAccess);
		t.Insert("!", Operator_LogicalNot);
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

		static bool IsLogicOperator(Operator op)
		{
			switch (op)
			{
			case Operator_LessThan:
			case Operator_GreaterThan:
			case Operator_Equal:
			case Operator_NotEqual:
			case Operator_LessThanOrEqual:
			case Operator_GreaterThanOrEqual:
			case Operator_OrOr:
			case Operator_AndAnd:
			case Operator_LogicalNot:
				return true;
			default:
				return false;
			}
		}

		constexpr int UNARY_PRECEDENCE = 8;

		static int GetOperatorPrecedence(Operator op)
		{
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
				return 1;

			case Operator_Ternary:
				return 1;

			case Operator_OrOr:
				return 2;

			case Operator_AndAnd:
				return 3;

			case Operator_LessThan:
			case Operator_GreaterThan:
			case Operator_Equal:
			case Operator_NotEqual:
			case Operator_LessThanOrEqual:
			case Operator_GreaterThanOrEqual:
				return 4;

			case Operator_BitwiseShiftLeft:
			case Operator_BitwiseShiftRight:
			case Operator_BitwiseAnd:
			case Operator_BitwiseOr:
			case Operator_BitwiseXor:
				return 5;

			case Operator_Add:
			case Operator_Subtract:
				return 6;

			case Operator_Multiply:
			case Operator_Divide:
			case Operator_Modulus:
				return 7;

			case Operator_LogicalNot:
			case Operator_BitwiseNot:
			case Operator_Increment:
			case Operator_Decrement:
				return UNARY_PRECEDENCE;

			case Operator_MemberAccess:
				return 9;

			default:
				return -1;
			}
		}

		static bool isPreprocessorAllowedOperator(const Operator& op)
		{
			return op >= Operator_BinaryRangeStart && op <= Operator_BinaryRangeEnd || op == Operator_LogicalNot || op == Operator_BitwiseNot;
		}

		static bool isBinaryOperator(Operator op)
		{
			return op >= Operator_BinaryRangeStart && op <= Operator_BinaryRangeEnd;
		}

		static bool isUnaryOperator(Operator op)
		{
			return op >= Operator_UnaryRangeStart && op <= Operator_UnaryRangeEnd || op == Operator_Subtract;
		}

		static bool isTernaryOperator(Operator op)
		{
			return op == Operator_Ternary;
		}

		static bool isMemberAccessOperator(Operator op)
		{
			return op == Operator_MemberAccess;
		}

		static bool isAssignment(Operator op)
		{
			return op >= Operator_AssignmentRangeStart && op <= Operator_AssignmentRangeEnd;
		}

		static bool isCompoundAssignment(Operator op)
		{
			return op >= Operator_CompoundAssignmentRangeStart && op <= Operator_CompoundAssignmentRangeEnd;
		}

		static Operator compoundToBinary(Operator op)
		{
			if (isCompoundAssignment(op))
			{
				return static_cast<Operator>(op - Operator_AssignmentRangeStart);
			}
			return op;
		}
	}
}

#endif