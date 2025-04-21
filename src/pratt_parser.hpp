#ifndef PRATT_PARSER_H
#define PRATT_PARSER_H

#include "sub_parser_registry.hpp"
namespace HXSL
{
	static bool ParseSingleLeftExpression(HXSLParser& parser, TokenStream& stream, HXSLNode* parent, std::unique_ptr<HXSLExpression>& expression, bool& hadBrackets);

	static bool ParseExpressionInner(HXSLParser& parser, TokenStream& stream, HXSLNode* parent, std::unique_ptr<HXSLExpression>& expression, int precedence = 0)
	{
		auto start = stream.Current();

		bool hadBrackets = false;
		HXSLOperator unaryOp;
		if (start.isUnaryOperator(unaryOp))
		{
			stream.Advance();
			std::unique_ptr<HXSLExpression> operand;
			IF_ERR_RET_FALSE(ParseSingleLeftExpression(parser, stream, parent, operand, hadBrackets));
			if (unaryOp == HXSLOperator_Increment || unaryOp == HXSLOperator_Decrement)
			{
				if (operand->GetType() != HXSLNodeType_SymbolRefExpression)
				{
					parser.LogError("Prefix increment/decrement must target a variable.", operand->GetSpan());
					return false;
				}
			}
			expression = std::make_unique<HXSLPrefixExpression>(TextSpan(), parent, unaryOp, std::move(operand));
			expression->SetSpan(stream.MakeFromLast(start));
			hadBrackets = false;
		}
		else
		{
			IF_ERR_RET_FALSE(ParseSingleLeftExpression(parser, stream, parent, expression, hadBrackets));
		}

		while (true)
		{
			auto token = stream.Current();

			HXSLOperator op;
			if (!token.isOperator(op))
			{
				if (token.isDelimiterOf({ ';' ,')', '}', ']', ',', ':' }))
				{
					return true;
				}

				if (hadBrackets && expression->GetType() == HXSLNodeType_SymbolRefExpression)
				{
					std::unique_ptr<HXSLExpression> rightCast;
					IF_ERR_RET_FALSE(ParseSingleLeftExpression(parser, stream, parent, rightCast, hadBrackets));
					expression = std::make_unique<HXSLCastExpression>(TextSpan(), parent, std::move(expression), std::move(rightCast));
					expression->SetSpan(stream.MakeFromLast(start));
					continue;
				}

				ERR_RETURN_FALSE(parser, "Unexpected token in expression, expected an operator.");
			}

			int prec = Operators::GetOperatorPrecedence(op);

			if (prec <= precedence)
			{
				return true;
			}

			stream.Advance();

			if (Operators::isUnaryOperator(op) && op != HXSLOperator_Subtract)
			{
				if (op != HXSLOperator_Increment && op != HXSLOperator_Decrement)
				{
					parser.LogError("Invalid postfix operator.", stream.LastToken());
					return false;
				}

				if (expression->GetType() != HXSLNodeType_SymbolRefExpression)
				{
					parser.LogError("Postfix increment/decrement must target a variable.", expression->GetSpan());
					return false;
				}

				expression = std::make_unique<HXSLPostfixExpression>(TextSpan(), parent, op, std::move(expression));
				expression->SetSpan(stream.MakeFromLast(start));
			}
			else if (Operators::isTernaryOperator(op))
			{
				auto ternary = std::make_unique<HXSLTernaryExpression>(TextSpan(), parent, std::move(expression), nullptr, nullptr);
				std::unique_ptr<HXSLExpression> right;
				IF_ERR_RET_FALSE(ParseSingleLeftExpression(parser, stream, ternary.get(), right, hadBrackets));
				ternary->SetRight(std::move(right));
				stream.ExpectOperator(HXSLOperator_TernaryElse);
				std::unique_ptr<HXSLExpression> left;
				IF_ERR_RET_FALSE(ParseSingleLeftExpression(parser, stream, ternary.get(), left, hadBrackets));
				ternary->SetRight(std::move(left));
				ternary->SetSpan(stream.MakeFromLast(start));
				expression = std::move(ternary);
			}
			else
			{
				std::unique_ptr<HXSLExpression> right;
				IF_ERR_RET_FALSE(ParseExpressionInner(parser, stream, parent, right, prec));
				expression = std::make_unique<HXSLBinaryExpression>(TextSpan(), parent, op, std::move(expression), std::move(right));
				expression->SetSpan(stream.MakeFromLast(start));
			}
		}

		return true;
	}

	bool ParseSingleLeftExpression(HXSLParser& parser, TokenStream& stream, HXSLNode* parent, std::unique_ptr<HXSLExpression>& expression, bool& hadBrackets)
	{
		if (stream.TryGetDelimiter('('))
		{
			IF_ERR_RET_FALSE(ParseExpressionInner(parser, stream, parent, expression, 0));
			IF_ERR_RET_FALSE(stream.ExpectDelimiter(')'));
			hadBrackets = true;
		}
		else
		{
			IF_ERR_RET_FALSE(HXSLExpressionParserRegistry::TryParse(parser, stream, parent, expression)); // member expressions and complex grammar.
		}
		return true;
	}

	static bool ParseExpression(HXSLParser& parser, TokenStream& stream, HXSLNode* parent, std::unique_ptr<HXSLExpression>& expression)
	{
		HXSL_ASSERT(parent, "Parent cannot be null.");
		auto start = stream.Current();
		stream.PushState();
		if (!ParseExpressionInner(parser, stream, parent, expression))
		{
			stream.PopState();
			return false;
		}
		stream.PopState(false);
		expression->SetParent(parent);

		return true;
	}
}

#endif