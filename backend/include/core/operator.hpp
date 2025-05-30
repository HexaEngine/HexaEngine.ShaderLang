#ifndef CORE_OPERATOR_HPP
#define CORE_OPERATOR_HPP

#include "pch/std.hpp"
#include "utils/radix_tree.hpp"

namespace HXSL
{
	enum Operator : char
	{
		Operator_Unknown,          // Unknown

		Operator_Add,               // +
		Operator_Subtract,          // -
		Operator_Multiply,          // *
		Operator_Divide,            // /
		Operator_Modulus,           // %

		Operator_BitwiseShiftLeft,  // <<
		Operator_BitwiseShiftRight, // >>

		Operator_AndAnd,            // &&
		Operator_OrOr,              // ||

		Operator_BitwiseAnd,        // &
		Operator_BitwiseOr,         // |
		Operator_BitwiseXor,        // ^

		Operator_LessThan,          // <
		Operator_LessThanOrEqual,   // <=
		Operator_GreaterThan,       // >
		Operator_GreaterThanOrEqual,// >=
		Operator_Equal,             // ==
		Operator_NotEqual,          // !=

		Operator_Assign,            // =
		Operator_PlusAssign,        // +=
		Operator_MinusAssign,       // -=
		Operator_MultiplyAssign,    // *=
		Operator_DivideAssign,      // /=
		Operator_ModulusAssign,     // %=
		Operator_BitwiseShiftLeftAssign,  // <<=
		Operator_BitwiseShiftRightAssign, // >>=
		Operator_BitwiseAndAssign, // &=
		Operator_BitwiseOrAssign,  // |=
		Operator_BitwiseXorAssign, // ^=

		Operator_Increment,         // ++
		Operator_Decrement,         // --
		Operator_LogicalNot,        // !
		Operator_BitwiseNot,        // ~

		Operator_Ternary,           // ?
		Operator_TernaryElse,       // :

		Operator_MemberAccess,      // .

		Operator_Cast,               // Type casting

		Operator_Colon = Operator_TernaryElse
	};
}

#endif