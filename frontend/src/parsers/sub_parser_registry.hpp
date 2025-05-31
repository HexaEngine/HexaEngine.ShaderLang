#ifndef SUB_PARSER_REGISTRY_HPP
#define SUB_PARSER_REGISTRY_HPP

#include "sub_parser.hpp"

namespace HXSL
{
	class SubParserRegistry
	{
	private:
		static std::vector<std::unique_ptr<SubParser>> parsers;
		static std::once_flag initFlag;

	public:
		static void EnsureCreated();

		static bool TryParse(Parser& parser, TokenStream& stream)
		{
			do
			{
				for (auto& subParser : parsers)
				{
					stream.PushState();
					if (subParser->TryParse(parser, stream))
					{
						stream.PopState(false);
						return true;
					}
					stream.PopState();
				}

				auto current = stream.Current();
				if (stream.IsEndOfTokens() || (current.Type == TokenType_Delimiter && current.Value == '}'))
				{
					return false;
				}
			} while (parser.AttemptErrorRecovery());

			return false;
		}

		template <typename SubParserType>
		static typename std::enable_if<std::is_base_of<SubParser, SubParserType>::value>::type
			Register()
		{
			parsers.push_back(std::make_unique<SubParserType>());
		}
	};

	class StatementParserRegistry
	{
	private:
		static std::vector<std::unique_ptr<StatementParser>> parsers;
		static std::once_flag initFlag;

	public:
		static void EnsureCreated();

		static bool TryParse(Parser& parser, TokenStream& stream, ast_ptr<ASTNode>& statementOut, bool leaveOpen = false)
		{
			for (auto& subParser : parsers)
			{
				stream.PushState();
				if (subParser->TryParse(parser, stream, statementOut))
				{
					stream.PopState(false);
					return true;
				}
				stream.PopState();
			}

			if (!leaveOpen)
			{
				auto current = stream.Current();
				if (!stream.IsEndOfTokens() && (current.Type != TokenType_Delimiter || current.Value != '}'))
				{
					parser.Log(UNEXPECTED_TOKEN, stream.Current());
					return false;
				}
			}

			return false;
		}

		template <typename SubParserType>
		static typename std::enable_if<std::is_base_of<StatementParser, SubParserType>::value>::type
			Register()
		{
			parsers.push_back(std::make_unique<SubParserType>());
		}
	};

	static bool ParseStatementBodyInner(Parser& parser, TokenStream& stream, StatementContainer* container, bool leaveOpen = false)
	{
		if (stream.TryGetDelimiter(';'))
		{
			return true;
		}
		ast_ptr<ASTNode> outStatement;
		bool success = StatementParserRegistry::TryParse(parser, stream, outStatement, leaveOpen);

		if (!success)
		{
			while (!stream.TryGetDelimiter(';') && !stream.IsEndOfTokens())
			{
				auto token = stream.Current();
				if (token.isKeyword() || token.isIdentifier())
				{
					break;
				}
				stream.Advance();
			}

			return false;
		}

		container->AddStatement(std::move(outStatement));
		return true;
	}

	static bool ParseStatementBody(ScopeType type, Parser& parser, TokenStream& stream, ast_ptr<BlockStatement>& statement)
	{
		// TODO: single line statements.
		auto blockStatement = make_ast_ptr<BlockStatement>(TextSpan());
		Token first;
		parser.EnterScope(type, blockStatement.get(), first, true);

		while (parser.IterateScope(blockStatement.get()))
		{
			parser.ParseInnerBegin();

			if (ParseStatementBodyInner(parser, stream, blockStatement.get()))
			{
				HXSL_ASSERT(parser.modifierList.Empty(), "Modifier list was not empty, forgot to accept/reject it?.");
				HXSL_ASSERT(!parser.attribute.HasResource(), "Attribute list was not empty, forgot to accept/reject it?.");
			}
			else
			{
				if (!parser.TryRecoverScope(blockStatement.get(), true))
				{
					break;
				}
			}
		}

		auto span = first.Span.merge(stream.LastToken().Span);
		blockStatement->SetSpan(span);
		statement = std::move(blockStatement);
		return true;
	}

	class ExpressionParserRegistry
	{
	private:
		static std::vector<std::unique_ptr<ExpressionParser>> parsers;
		static std::once_flag initFlag;

	public:
		static void EnsureCreated();

		static bool TryParse(Parser& parser, TokenStream& stream, ast_ptr<Expression>& expressionOut)
		{
			auto first = stream.Current();
			for (auto& subParser : parsers)
			{
				stream.PushState();
				if (subParser->TryParse(parser, stream, expressionOut))
				{
					stream.PopState(false);
					return true;
				}
				stream.PopState();
			}

			auto current = stream.Current();
			if (!stream.IsEndOfTokens() && current.Type != TokenType_Delimiter && current.Value != ';')
			{
				parser.Log(UNEXPECTED_TOKEN, stream.Current());
				return false;
			}

			if (!expressionOut.get())
			{
				expressionOut = make_ast_ptr<EmptyExpression>(first.Span.merge(stream.LastToken().Span));
				return true;
			}

			return false;
		}

		template <typename SubParserType>
		static typename std::enable_if<std::is_base_of<ExpressionParser, SubParserType>::value>::type
			Register()
		{
			parsers.push_back(std::make_unique<SubParserType>());
		}
	};
}

#endif