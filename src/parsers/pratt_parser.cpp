#include "pratt_parser.hpp"
#include "parser_helper.hpp"

namespace HXSL
{
#define IF_ERR_RET_FALSE_ASSERT(expr) \
	if (!expr) { \
	HXSL_ASSERT(false, "");	\
	 return false; \
	}
	const std::unordered_set<char> expressionDelimiters = { ';' ,')', '}', ']', ',', ':' };

	using ExpressionPtr = std::unique_ptr<Expression>;

	bool PrattParser::TryParseUnaryPrefixOperator(const Token& start, Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expression, bool& hadBrackets)
	{
		Operator unaryOp;
		if (stream.TryGetUnaryOperator(unaryOp))
		{
			std::unique_ptr<Expression> operand;
			IF_ERR_RET_FALSE(ParseSingleLeftExpression(parser, stream, parent, operand, hadBrackets));
			if (unaryOp == Operator_Increment || unaryOp == Operator_Decrement)
			{
				auto type = operand->GetType();
				if (type != NodeType_MemberReferenceExpression && type != NodeType_MemberAccessExpression)
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

		return true;
	}

	bool PrattParser::TryParseUnaryPostfixOperator(const Token& start, Operator op, Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expression)
	{
		if (op != Operator_Increment && op != Operator_Decrement)
		{
			parser.LogError("Invalid postfix operator.", stream.LastToken());
			return false;
		}

		auto type = expression->GetType();
		if (type != NodeType_MemberReferenceExpression && type != NodeType_MemberAccessExpression)
		{
			parser.LogError("Postfix increment/decrement must target a variable.", expression->GetSpan());
			return false;
		}

		expression = std::make_unique<PostfixExpression>(TextSpan(), parent, op, std::move(expression));
		expression->SetSpan(stream.MakeFromLast(start));
		return true;
	}

	bool PrattParser::TryParseCastOperator(const Token& start, Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expression)
	{
		std::unique_ptr<SymbolRef> type;
		ParserHelper::MakeConcreteSymbolRef(expression.get(), SymbolRefType_Type, type);
		std::unique_ptr<Expression> rightCast;
		bool hadBrackets;
		IF_ERR_RET_FALSE(ParseSingleLeftExpression(parser, stream, parent, rightCast, hadBrackets));
		expression = std::make_unique<CastExpression>(TextSpan(), parent, std::move(type), std::move(rightCast));
		expression->SetSpan(stream.MakeFromLast(start));
		return true;
	}

	bool PrattParser::ParseExpressionInner(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expression, int precedence)
	{
		auto start = stream.Current();

		bool hadBrackets = false;
		if (!TryParseUnaryPrefixOperator(start, parser, stream, parent, expression, hadBrackets))
		{
			return false;
		}

		while (true)
		{
			auto token = stream.Current();

			Operator op;
			if (!token.isOperator(op))
			{
				if (token.isDelimiterOf(expressionDelimiters))
				{
					return true;
				}

				auto type = expression->GetType();
				if (hadBrackets && (type == NodeType_MemberReferenceExpression || type == NodeType_MemberAccessExpression))
				{
					IF_ERR_RET_FALSE(TryParseCastOperator(start, parser, stream, parent, expression));
					continue;
				}

				ERR_RETURN_FALSE(parser, "Unexpected token in expression, expected an operator or ';'.");
			}

			int prec = Operators::GetOperatorPrecedence(op);

			if (prec <= precedence)
			{
				return true;
			}

			stream.TryAdvance();

			if (Operators::isUnaryOperator(op) && op != Operator_Subtract)
			{
				IF_ERR_RET_FALSE(TryParseUnaryPostfixOperator(start, op, parser, stream, parent, expression));
			}
			else if (Operators::isTernaryOperator(op))
			{
				auto ternary = std::make_unique<TernaryExpression>(TextSpan(), parent, std::move(expression), nullptr, nullptr);
				std::unique_ptr<Expression> right;
				IF_ERR_RET_FALSE(ParseExpressionInner(parser, stream, ternary.get(), right, hadBrackets));
				ternary->SetTrueBranch(std::move(right));
				stream.ExpectOperator(Operator_TernaryElse);
				std::unique_ptr<Expression> left;
				IF_ERR_RET_FALSE(ParseExpressionInner(parser, stream, ternary.get(), left, hadBrackets));
				ternary->SetFalseBranch(std::move(left));
				ternary->SetSpan(stream.MakeFromLast(start));
				expression = std::move(ternary);
			}
			else if (Operators::isAssignment(op))
			{
				auto chainExpr = dynamic_cast<ChainExpression*>(expression.get());
				if (chainExpr == nullptr)
				{
					parser.LogError("Expected member access expression on assignment expression.", expression->GetSpan());
					return false;
				}

				auto assignment = std::make_unique<AssignmentExpression>(TextSpan(), parent, std::move(expression), nullptr);
				std::unique_ptr<Expression> right;
				IF_ERR_RET_FALSE(ParseExpressionInner(parser, stream, assignment.get(), right, hadBrackets));

				if (Operators::isCompoundAssignment(op))
				{
					auto binaryOp = Operators::compoundToBinary(op);
					auto binary = std::make_unique<BinaryExpression>(TextSpan(), assignment.get(), binaryOp, nullptr, nullptr);
					binary->SetLeft(std::unique_ptr<Expression>(static_cast<Expression*>(chainExpr->Clone(parent).release())));
					binary->GetLeft()->SetParent(binary.get());
					binary->SetRight(std::move(right));
					binary->GetRight()->SetParent(binary.get());
					right = std::move(binary);
				}

				assignment->SetExpression(std::move(right));
				assignment->SetSpan(stream.MakeFromLast(start));
				expression = std::move(assignment);
			}
			else
			{
				auto binary = std::make_unique<BinaryExpression>(TextSpan(), parent, op, std::move(expression), nullptr);
				std::unique_ptr<Expression> right;
				if (!ParseExpressionInner(parser, stream, binary.get(), right, prec))
				{
					return false;
				}
				binary->SetRight(std::move(right));
				binary->SetSpan(stream.MakeFromLast(start));
				expression = std::move(binary);
			}
		}

		return true;
	}

	bool PrattParser::ParseSingleLeftExpression(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expression, bool& hadBrackets)
	{
		if (stream.TryGetDelimiter('('))
		{
			IF_ERR_RET_FALSE(ParseExpressionInner(parser, stream, parent, expression, 0));
			stream.ExpectDelimiter(')');
			hadBrackets = true;
		}
		else
		{
			IF_ERR_RET_FALSE(ExpressionParserRegistry::TryParse(parser, stream, parent, expression)); // member expressions and complex grammar.
		}
		return true;
	}

#define HANDLE_RESULT(expr) \
	auto result = expr; \
	if (result == 1) continue; else if (result == -1) return false;

	bool PrattParser::ParseExpressionInnerIter(Parser& parser, TokenStream& stream, ASTNode* parent1, std::unique_ptr<Expression>& expressionOut)
	{
		std::stack<Frame> stack;
		std::stack<std::unique_ptr<Expression>> resultStack;
		stack.push(Frame(FrameType_ParseOperator, FrameFlags_None, 0, parent1));
		stack.push(Frame(FrameType_ParseExpression, FrameFlags_None, 0, parent1));

		bool end = false;
		while (!stack.empty())
		{
			Frame frame = std::move(stack.top());
			stack.pop();

			switch (frame.type)
			{
			case FrameType_ParseExpression:
			{
				auto start = stream.Current();

				if (start.isDelimiterOf(expressionDelimiters))
				{
					continue;
				}

				Operator unaryOp;
				if (stream.TryGetUnaryOperator(unaryOp))
				{
					auto expr = std::make_unique<PrefixExpression>(TextSpan(), frame.parent, unaryOp, nullptr);
					stack.push(Frame(FrameType_Prefix, frame.precedence, std::move(expr), start.Span));

					auto result = ParseSingleLeftExpressionIter(frame, stack, resultStack, parser, stream);
					if (result == -1) return false; else continue;
				}
				else
				{
					HANDLE_RESULT(ParseSingleLeftExpressionIter(frame, stack, resultStack, parser, stream));
				}
				break;
			}

			case FrameType_ParseOperator:
			{
			loop:
				auto token = stream.Current();
				auto& peekOperand = resultStack.top();
				auto& start = peekOperand->GetSpan();

				Operator op;
				if (!token.isOperator(op))
				{
					if (token.isDelimiterOf(expressionDelimiters))
					{
						continue;
					}

					if (frame.GetFlag(FrameFlags_HadBrackets) && peekOperand->GetType() == NodeType_MemberReferenceExpression)
					{
						std::unique_ptr<SymbolRef> type;
						ParserHelper::MakeConcreteSymbolRef(peekOperand.get(), SymbolRefType_Type, type);
						resultStack.pop();

						auto expr = std::make_unique<CastExpression>(TextSpan(), frame.parent, std::move(type), nullptr);
						stack.push(Frame(FrameType_Cast, frame.precedence, std::move(expr), start));

						HANDLE_RESULT(ParseSingleLeftExpressionIter(frame, stack, resultStack, parser, stream));
					}

					ERR_RETURN_FALSE(parser, "Unexpected token in expression, expected an operator or ';'.");
				}

				int prec = Operators::GetOperatorPrecedence(op);

				if (prec <= frame.precedence)
				{
					continue;
				}

				stream.TryAdvance();

				if (Operators::isUnaryOperator(op) && op != Operator_Subtract)
				{
					ExpressionPtr expression = std::move(resultStack.top());
					resultStack.pop();
					auto& span = expression->GetSpan();

					if (op != Operator_Increment && op != Operator_Decrement)
					{
						parser.LogError("Invalid postfix operator.", stream.LastToken());
						return false;
					}

					if (expression->GetType() != NodeType_MemberReferenceExpression)
					{
						parser.LogError("Postfix increment/decrement must target a variable.", span);
						return false;
					}

					auto expr = std::make_unique<PostfixExpression>(TextSpan(), frame.parent, op, std::move(expression));
					expr->SetSpan(stream.MakeFromLast(span));
					resultStack.push(std::move(expr));
					goto loop;
				}
				else if (Operators::isTernaryOperator(op))
				{
					ExpressionPtr expression = std::move(resultStack.top());
					resultStack.pop();

					auto expr = std::make_unique<TernaryExpression>(TextSpan(), frame.parent, std::move(expression), nullptr, nullptr);
					stack.push(Frame(FrameType_Ternary, prec, std::move(expr), start));

					HANDLE_RESULT(ParseSingleLeftExpressionIter(stack.top(), stack, resultStack, parser, stream));
				}
				else if (Operators::isAssignment(op))
				{
					// TODO: Add later.
				}
				else
				{
					ExpressionPtr expression = std::move(resultStack.top());
					resultStack.pop();

					auto expr = std::make_unique<BinaryExpression>(TextSpan(), frame.expr.get(), op, std::move(expression), nullptr);
					stack.push(Frame(FrameType_Binary, prec, std::move(expr), start));

					HANDLE_RESULT(ParseSingleLeftExpressionIter(stack.top(), stack, resultStack, parser, stream));
				}
			}
			break;
			case FrameType_Prefix:
			{
				auto expr = static_cast<PrefixExpression*>(frame.expr.get());
				ExpressionPtr operand = std::move(resultStack.top());
				resultStack.pop();

				auto unaryOp = expr->GetOperator();
				if (unaryOp == Operator_Increment || unaryOp == Operator_Decrement)
				{
					auto& type = operand->GetType();
					if (type != NodeType_MemberReferenceExpression && type != NodeType_MemberAccessExpression)
					{
						parser.LogError("Prefix increment/decrement must target a variable.", operand->GetSpan());
						return false;
					}
				}
				expr->SetOperand(std::move(operand));
				expr->SetSpan(stream.MakeFromLast(frame.begin));
				stack.top().ClearFlag(FrameFlags_HadBrackets);
				resultStack.push(std::move(frame.expr));
				stack.push(Frame(FrameType_ParseOperator, FrameFlags_None, 0, parent1));
			}
			break;
			case FrameType_Cast:
			{
				auto expr = static_cast<CastExpression*>(frame.expr.get());
				ExpressionPtr operand = std::move(resultStack.top());
				resultStack.pop();

				expr->SetOperand(std::move(operand));
				expr->SetSpan(stream.MakeFromLast(frame.begin));
				resultStack.push(std::move(frame.expr));
				stack.push(Frame(FrameType_ParseOperator, FrameFlags_None, 0, parent1));
			}
			break;
			case FrameType_Binary:
			{
				auto expr = static_cast<BinaryExpression*>(frame.expr.get());
				ExpressionPtr operand = std::move(resultStack.top());
				resultStack.pop();

				expr->SetRight(std::move(operand));
				expr->SetSpan(stream.MakeFromLast(frame.begin));
				resultStack.push(std::move(frame.expr));
				stack.push(Frame(FrameType_ParseOperator, FrameFlags_None, frame.precedence, parent1));
			}
			break;
			case FrameType_Ternary:
			{
				auto expr = static_cast<TernaryExpression*>(frame.expr.get());
				ExpressionPtr operand = std::move(resultStack.top());
				resultStack.pop();

				if (!expr->GetTrueBranch())
				{
					expr->SetTrueBranch(std::move(operand));
					stack.push(std::move(frame));
					stream.ExpectOperator(Operator_TernaryElse);
					HANDLE_RESULT(ParseSingleLeftExpressionIter(stack.top(), stack, resultStack, parser, stream));
				}
				else
				{
					expr->SetFalseBranch(std::move(operand));
					resultStack.push(std::move(frame.expr));
					stack.push(Frame(FrameType_ParseOperator, FrameFlags_None, 0, parent1));
				}
			}
			break;
			case FrameType_ClosingBracket:
			{
				stream.ExpectDelimiter(')');
				stack.push(Frame(FrameType_ParseExpression, FrameFlags_None, 0, parent1));
			}
			break;
			default:
				break;
			}
		}

		expressionOut = std::move(resultStack.top());
		resultStack.pop();

		return true;
	}

	int PrattParser::ParseSingleLeftExpressionIter(Frame& frame, std::stack<Frame>& stack, std::stack<std::unique_ptr<Expression>>& resultStack, Parser& parser, TokenStream& stream)
	{
		auto parent = frame.parent;
		HXSL_ASSERT(parent, "Parent cannot be null.");
		if (stream.TryGetDelimiter('('))
		{
			frame.SetFlag(FrameFlags_HadBrackets);
			stack.push(Frame(FrameType_ClosingBracket, FrameFlags_None, 0, nullptr));
			stack.push(Frame(FrameType_ParseExpression, FrameFlags_None, 0, parent));
			return 1;
		}
		else
		{
			std::unique_ptr<Expression> expr;
			if (!ExpressionParserRegistry::TryParse(parser, stream, parent, expr)) // member expressions and complex grammar.
			{
				return -1;
			}
			resultStack.push(std::move(expr));
		}
		return 0;
	}

	bool PrattParser::ParseExpression(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expression)
	{
		HXSL_ASSERT(parent, "Parent cannot be null.");
		auto start = stream.Current();
		stream.PushState();
		if (!ParseExpressionInner(parser, stream, parent, expression))
		{
		}
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