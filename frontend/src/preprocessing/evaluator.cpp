#include "evaluator.hpp"

namespace HXSL
{
	static bool isLeftAssociative(Operator op, bool isUnary)
	{
		return !isUnary && !Operators::isAssignment(op) && !Operators::isCompoundAssignment(op);
	}

	struct OperatorToken
	{
		Token token;
		int precedence;
		bool isUnary;

		OperatorToken(const Token& token) : token(token), precedence(0), isUnary(false) {}

		OperatorToken(const Token& token, int precedence, bool isUnary) : token(token), precedence(precedence), isUnary(isUnary) {}

		EvalItem ToEval() const
		{
			return EvalItem(token, isUnary);
		}
	};

	std::vector<EvalItem> Evaluator::toPostfix(const std::vector<Token>& infixTokens)
	{
		std::vector<EvalItem> output;
		std::stack<OperatorToken> operatorStack;

		bool wasOperator = false;
		for (const Token& token : infixTokens)
		{
			if (token.isNumeric() || token.isIdentifier() || token.isLiteral())
			{
				output.push_back(EvalItem(token));
				wasOperator = false;
			}
			else if (token.isDelimiter())
			{
				if (token.Value == '(')
				{
					operatorStack.push(token);
				}
				else if (token.Value == ')')
				{
					bool foundMatchingParen = false;

					while (!operatorStack.empty())
					{
						OperatorToken topToken = operatorStack.top();
						operatorStack.pop();

						if (topToken.token.isDelimiterOf('('))
						{
							foundMatchingParen = true;
							break;
						}
						else
						{
							output.push_back(topToken.ToEval());
						}
					}

					if (!foundMatchingParen)
					{
						// TODO: error logging.
						continue;
					}
				}
			}
			else if (token.isOperator())
			{
				Operator op = token.asOperator();

				if (!Operators::isPreprocessorAllowedOperator(op))
				{
					// TODO: error logging.
					continue;
				}

				auto prec = Operators::GetOperatorPrecedence(op);

				bool isUnary = false;

				if (Operators::isUnaryOperator(op) && (op != Operator_Subtract || (op == Operator_Subtract && wasOperator)))
				{
					isUnary = true;
					wasOperator = false;
					prec = Operators::UNARY_PRECEDENCE;
				}

				if (wasOperator)
				{
					// TODO: error logging.
					continue;
				}

				wasOperator = true;

				while (!operatorStack.empty())
				{
					const OperatorToken& topT = operatorStack.top();

					if (topT.token.isDelimiterOf('('))
					{
						break;
					}

					Operator topOp = topT.token.asOperator();

					if (topT.precedence > prec || (topT.precedence == prec && isLeftAssociative(op, topT.isUnary)))
					{
						output.push_back(topT.ToEval());
						operatorStack.pop();
					}
					else
					{
						break;
					}
				}

				operatorStack.push(OperatorToken(token, prec, isUnary));
			}
			else
			{
				// TODO: error logging.
				continue;
			}
		}

		while (!operatorStack.empty())
		{
			OperatorToken topT = operatorStack.top();
			operatorStack.pop();

			if (topT.token.isDelimiterOf('('))
			{
				// TODO: error logging.
				continue;
			}

			output.push_back(topT.ToEval());
		}

		return output;
	}

	static Number performOperation(Operator op, const Number& a, const Number& b)
	{
		switch (op) {
		case Operator_Add: return a + b;
		case Operator_Subtract: return a - b;
		case Operator_Multiply: return a * b;
		case Operator_Divide:
			if (b.IsZero())
			{
				return UNKNOWN_NUMBER;
			}
			return a / b;
		case Operator_Modulus:
			if (b.IsZero())
			{
				return UNKNOWN_NUMBER;
			}
			return a % b;
		case Operator_BitwiseShiftLeft: return a << b;
		case Operator_BitwiseShiftRight: return a >> b;
		case Operator_AndAnd: return Number(a.ToBool() && b.ToBool());
		case Operator_OrOr: return Number(a.ToBool() || b.ToBool());
		case Operator_BitwiseAnd: return a & b;
		case Operator_BitwiseOr: return a | b;
		case Operator_BitwiseXor: return a ^ b;
		case Operator_LessThan: return Number(a < b);
		case Operator_LessThanOrEqual: return Number(a <= b);
		case Operator_GreaterThan: return Number(a > b);
		case Operator_GreaterThanOrEqual: return Number(a >= b);
		case Operator_Equal: return Number(a == b);
		case Operator_NotEqual: return Number(a != b);
		default: HXSL_ASSERT(false, "Unsupported binary operator in preprocessor expression"); return {};
		}
	}

	static Number performUnaryOperation(Operator op, const Number& a)
	{
		switch (op) {
		case Operator_LogicalNot: return Number(!a.ToBool());
		case Operator_BitwiseNot: return ~a;
		case Operator_Subtract: return -a;
		default: HXSL_ASSERT(false, "Unsupported unary operator in preprocessor expression"); return {};
		}
	}

	Number Evaluator::evaluatePostfix(const std::vector<EvalItem>& postfixTokens)
	{
		std::stack<Number> operandStack;

		for (const EvalItem& item : postfixTokens)
		{
			auto& token = item.token;
			if (token.isNumeric())
			{
				operandStack.push(token.Numeric);
			}
			else if (token.isOperator())
			{
				Operator op = token.asOperator();

				if (item.isUnary)
				{
					if (operandStack.empty())
					{
						// TODO: error logging
						break;
					}

					Number a = operandStack.top(); operandStack.pop();
					Number result = performUnaryOperation(op, a);
					operandStack.push(result);
				}
				else if (Operators::isBinaryOperator(op))
				{
					if (operandStack.size() < 2)
					{
						// TODO: error logging
						break;
					}

					Number b = operandStack.top(); operandStack.pop();
					Number a = operandStack.top(); operandStack.pop();

					Number result = performOperation(op, a, b);
					operandStack.push(result);
				}
				else
				{
					// TODO: error logging
					break;
				}
			}
			else if (token.isIdentifier() || token.isLiteral())
			{
				operandStack.push(Number(0));
			}
		}

		if (operandStack.size() != 1)
		{
			// TODO: error logging
			return UNKNOWN_NUMBER;
		}

		return operandStack.top();
	}

	Number Evaluator::evaluate(const std::vector<Token>& infixTokens)
	{
		auto postfixTokens = toPostfix(infixTokens);
		return evaluatePostfix(postfixTokens);
	}
}