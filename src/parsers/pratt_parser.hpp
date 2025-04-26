#ifndef PRATT_PARSER_H
#define PRATT_PARSER_H

#include "sub_parser_registry.hpp"
namespace HXSL
{
	static bool ParseSingleLeftExpression(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expression, bool& hadBrackets);

	static bool ParseExpressionInner(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expression, int precedence = 0)
	{
		auto start = stream.Current();

		bool hadBrackets = false;
		Operator unaryOp;
		if (start.isUnaryOperator(unaryOp))
		{
			stream.Advance();
			std::unique_ptr<Expression> operand;
			IF_ERR_RET_FALSE(ParseSingleLeftExpression(parser, stream, parent, operand, hadBrackets));
			if (unaryOp == Operator_Increment || unaryOp == Operator_Decrement)
			{
				if (operand->GetType() != NodeType_MemberReferenceExpression)
				{
					parser.LogError("Prefix increment/decrement must target a variable.", operand->GetSpan());
					return false;
				}
			}
			expression = std::make_unique<PrefixExpression>(TextSpan(), parent, unaryOp, std::move(operand));
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

			Operator op;
			if (!token.isOperator(op))
			{
				if (token.isDelimiterOf({ ';' ,')', '}', ']', ',', ':' }))
				{
					return true;
				}

				if (hadBrackets && expression->GetType() == NodeType_MemberReferenceExpression)
				{
					std::unique_ptr<SymbolRef> type;
					ParserHelper::MakeConcreteSymbolRef(expression.get(), SymbolRefType_Type, type);
					std::unique_ptr<Expression> rightCast;
					IF_ERR_RET_FALSE(ParseSingleLeftExpression(parser, stream, parent, rightCast, hadBrackets));
					expression = std::make_unique<CastExpression>(TextSpan(), parent, std::move(type), std::move(rightCast));
					expression->SetSpan(stream.MakeFromLast(start));
					continue;
				}

				ERR_RETURN_FALSE(parser, "Unexpected token in expression, expected an operator or ';'.");
			}

			int prec = Operators::GetOperatorPrecedence(op);

			if (prec <= precedence)
			{
				return true;
			}

			stream.Advance();

			if (Operators::isUnaryOperator(op) && op != Operator_Subtract)
			{
				if (op != Operator_Increment && op != Operator_Decrement)
				{
					parser.LogError("Invalid postfix operator.", stream.LastToken());
					return false;
				}

				if (expression->GetType() != NodeType_MemberReferenceExpression)
				{
					parser.LogError("Postfix increment/decrement must target a variable.", expression->GetSpan());
					return false;
				}

				expression = std::make_unique<PostfixExpression>(TextSpan(), parent, op, std::move(expression));
				expression->SetSpan(stream.MakeFromLast(start));
			}
			else if (Operators::isTernaryOperator(op))
			{
				auto ternary = std::make_unique<TernaryExpression>(TextSpan(), parent, std::move(expression), nullptr, nullptr);
				std::unique_ptr<Expression> right;
				IF_ERR_RET_FALSE(ParseSingleLeftExpression(parser, stream, ternary.get(), right, hadBrackets));
				ternary->SetTrueBranch(std::move(right));
				stream.ExpectOperator(Operator_TernaryElse);
				std::unique_ptr<Expression> left;
				IF_ERR_RET_FALSE(ParseSingleLeftExpression(parser, stream, ternary.get(), left, hadBrackets));
				ternary->SetFalseBranch(std::move(left));
				ternary->SetSpan(stream.MakeFromLast(start));
				expression = std::move(ternary);
			}
			else
			{
				std::unique_ptr<Expression> right;
				IF_ERR_RET_FALSE(ParseExpressionInner(parser, stream, parent, right, prec));
				expression = std::make_unique<BinaryExpression>(TextSpan(), parent, op, std::move(expression), std::move(right));
				expression->SetSpan(stream.MakeFromLast(start));
			}
		}

		return true;
	}

	bool ParseSingleLeftExpression(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expression, bool& hadBrackets)
	{
		if (stream.TryGetDelimiter('('))
		{
			IF_ERR_RET_FALSE(ParseExpressionInner(parser, stream, parent, expression, 0));
			IF_ERR_RET_FALSE(stream.ExpectDelimiter(')'));
			hadBrackets = true;
		}
		else
		{
			IF_ERR_RET_FALSE(ExpressionParserRegistry::TryParse(parser, stream, parent, expression)); // member expressions and complex grammar.
		}
		return true;
	}

	static bool ParseExpression(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expression)
	{
		HXSL_ASSERT(parent, "Parent cannot be null.");
		auto start = stream.Current();
		stream.PushState();
		ParseExpressionInner(parser, stream, parent, expression);
		if (!expression.get())
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