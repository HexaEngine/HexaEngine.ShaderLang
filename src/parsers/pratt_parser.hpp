#ifndef PRATT_PARSER_H
#define PRATT_PARSER_H

#include "sub_parser_registry.hpp"

namespace HXSL
{
	enum FrameFlags : int
	{
		FrameFlags_None = 0,
		FrameFlags_HadBrackets = 1,
		FrameFlags_Unary = 2,
	};

	enum FrameType : int
	{
		FrameType_ClosingBracket,
		FrameType_ParseExpression,
		FrameType_ParseOperator,
		FrameType_Prefix,
		FrameType_Cast,
		FrameType_Binary,
		FrameType_Ternary,
	};

	DEFINE_FLAGS_OPERATORS(FrameFlags, int);

	class PrattParser
	{
		struct Frame
		{
			FrameType type;
			FrameFlags flags;
			int precedence;
			Operator op;
			TextSpan begin;
			std::unique_ptr<Expression> expr;
			ASTNode* parent;

			void SetFlag(FrameFlags flag, bool value)
			{
				if (value)
				{
					flags |= flag;
				}
				else
				{
					flags &= ~flag;
				}
			}

			void SetFlag(FrameFlags flag) noexcept
			{
				SetFlag(flag, true);
			}

			void ClearFlag(FrameFlags flag) noexcept
			{
				SetFlag(flag, false);
			}

			bool GetFlag(FrameFlags flag) const noexcept
			{
				return (flags & flag) != 0;
			}

			Frame() = default;

			Frame(FrameType type, FrameFlags flags, int precedence, ASTNode* parent) : type(type), flags(flags), precedence(precedence), parent(parent), op(Operator_Unknown)
			{
			}

			Frame(FrameType type, int precedence, ASTNode* parent) : type(type), flags(FrameFlags_None), precedence(precedence), parent(parent), op(Operator_Unknown)
			{
			}

			Frame(FrameType type, FrameFlags flags, int precedence, std::unique_ptr<Expression> expr, TextSpan begin) : type(type), flags(flags), precedence(precedence), op(Operator_Unknown), expr(std::move(expr)), begin(begin)
			{
				parent = this->expr.get();
			}

			Frame(FrameType type, int precedence, std::unique_ptr<Expression> expr, TextSpan begin) : type(type), flags(FrameFlags_None), precedence(precedence), op(Operator_Unknown), expr(std::move(expr)), begin(begin)
			{
				parent = this->expr.get();
			}
		};

	public:
		static bool TryParseUnaryPrefixOperator(const Token& start, Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expression, bool& hadBrackets);

		static bool TryParseUnaryPostfixOperator(const Token& start, Operator op, Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expression);

		static bool TryParseCastOperator(const Token& start, Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expression);

		static bool ParseExpressionInner(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expression, int precedence = 0);

		static bool ParseExpressionInnerIter(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expression);

		static bool ParseSingleLeftExpression(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expression, bool& hadBrackets);

		static int ParseSingleLeftExpressionIter(Frame& frame, std::stack<Frame>& stack, std::stack<std::unique_ptr<Expression>>& resultStack, Parser& parser, TokenStream& stream);

		static bool ParseExpression(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expression);
	};
}

#endif