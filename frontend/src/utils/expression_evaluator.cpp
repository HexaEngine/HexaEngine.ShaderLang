#include "utils/expression_evaluator.hpp"

namespace HXSL
{
	struct EvalOperator
	{
		Expression* Expression;
		bool Closing;
	};


	static Number FoldBinary(const Number& left, const Number& right, Operator code)
	{
		switch (code)
		{
		case Operator_Add: return left + right;
		case Operator_Subtract: return left - right;
		case Operator_Multiply: return left * right;
		case Operator_Divide: return left / right;
		case Operator_Modulus: return left % right;
		case Operator_BitwiseShiftLeft: return left << right;
		case Operator_BitwiseShiftRight: return left >> right;
		case Operator_AndAnd: return Number(left.ToBool() && right.ToBool());
		case Operator_OrOr: return Number(left.ToBool() || right.ToBool());
		case Operator_BitwiseAnd: return left & right;
		case Operator_BitwiseOr: return left | right;
		case Operator_BitwiseXor: return left ^ right;
		case Operator_LessThan: return Number(left < right);
		case Operator_LessThanOrEqual: return Number(left <= right);
		case Operator_GreaterThan: return Number(left > right);
		case Operator_GreaterThanOrEqual: return Number(left >= right);
		case Operator_Equal: return Number(left == right);
		case Operator_NotEqual: return Number(left != right);
		}
		return Number{};
	}

	static Number FoldUnary(const Number& left, Operator code)
	{
		switch (code)
		{
		case Operator_BitwiseNot: return ~left;
		case Operator_Subtract: return -left;
		case Operator_LogicalNot: return Number(!left.ToBool());
		case Operator_Increment: return left + Number(1);
		case Operator_Decrement: return left - Number(1);
		}
		return Number{};
	}

	ExpressionEvaluatorResult ExpressionEvaluator::TryEvaluate(Expression* expr, ExpressionEvaluatorContext& ctx)
	{
		std::stack<EvalOperator> operatorStack;
		std::stack<Number> operandStack;
		dense_set<SymbolDef*> unresolved;

		operatorStack.push({expr, false});
		while (!operatorStack.empty())
		{
			EvalOperator op = operatorStack.top();
			auto expr = op.Expression;
			operatorStack.pop();
			auto type = expr->GetType();

			switch (type)
			{
				case NodeType_LiteralExpression:
				{
					auto literal = cast<LiteralExpression>(expr);
					auto& token = literal->GetLiteral();
					if (token.isNumeric())
					{
						operandStack.push(token.Numeric);
					}
					else
					{
						return { false, {}, std::move(unresolved) };
					}
					break;
				}
				case NodeType_BinaryExpression:
				{
					auto binaryExpr = cast<BinaryExpression>(expr);
					if (op.Closing)
					{
						if (operandStack.size() < 2)
						{
							return { false, {}, std::move(unresolved) };
						}
						Number right = operandStack.top();
						operandStack.pop();
						Number left = operandStack.top();
						operandStack.pop();
						auto opType = binaryExpr->GetOperator();
						auto result = FoldBinary(left, right, opType);
						operandStack.push(result);
					}
					else
					{
						operatorStack.push({ binaryExpr, true });
						operatorStack.push({ binaryExpr->GetRight(), false });
						operatorStack.push({ binaryExpr->GetLeft(), false });
					}
					break;
				}
				case NodeType_PostfixExpression:
				{
					auto postfixExpr = cast<PostfixExpression>(expr);
					operatorStack.push({ postfixExpr->GetOperand(), false }); 
					// postfix expressions have no real change on the operand here.
					break;
				}
				case NodeType_PrefixExpression:
				{
					auto prefixExpr = cast<PrefixExpression>(expr);
					if (op.Closing)
					{
						auto operand = operandStack.top();
						operandStack.pop();
						auto result = FoldUnary(operand, prefixExpr->GetOperator());
						operandStack.push(result);
					}
					else
					{
						operatorStack.push({ prefixExpr, true });
						operatorStack.push({ prefixExpr->GetOperand(), false });
					}
				}
				break;
				case NodeType_MemberAccessExpression:
				{
					operatorStack.push({ cast<MemberAccessExpression>(expr)->GetNextExpression(), false });
				}
				break;
				case NodeType_MemberReferenceExpression:
				{
					auto memberRefExpr = cast<MemberReferenceExpression>(expr);
					auto def = memberRefExpr->GetSymbolRef()->GetDeclaration();
					auto it = ctx.symbols.find(def);
					if (it != ctx.symbols.end())
					{
						operandStack.push(it->second);
					}
					else
					{
						unresolved.insert(def);
						return { false, {}, std::move(unresolved) };
					}
				}
				break;
				default:
					return { false, {}, std::move(unresolved) };
			}
		}

		if (operandStack.size() != 1)
		{
			return { false, {}, std::move(unresolved) };
		}

		auto result = operandStack.top();
		return { true, result, std::move(unresolved) };
	}
}
