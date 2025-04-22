#ifndef SUB_PARSER_REGISTRY_H
#define SUB_PARSER_REGISTRY_H

#include "sub_parser.hpp"
#include <memory>

namespace HXSL
{
	class SubParserRegistry
	{
	private:
		static std::vector<std::unique_ptr<SubParser>> parsers;

	public:
		static bool TryParse(Parser& parser, TokenStream& stream, ASTNode* parent, Compilation* compilation)
		{
			parser.pushParentNode(parent);
			for (auto& subParser : parsers)
			{
				stream.PushState();
				if (subParser->TryParse(parser, stream, compilation))
				{
					parser.popParentNode();
					stream.PopState(false);
					return true;
				}
				stream.PopState();
			}
			parser.popParentNode();

			auto current = stream.Current();
			if (!stream.IsEndOfTokens() && current.Type != TokenType_Delimiter && current.Span[0] != '}')
			{
				parser.LogError("Unexpected token.");
			}

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

	public:
		static bool TryParse(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Statement>& statementOut, bool leaveOpen = false)
		{
			parser.pushParentNode(parent);
			for (auto& subParser : parsers)
			{
				stream.PushState();
				if (subParser->TryParse(parser, stream, statementOut))
				{
					stream.PopState(false);
					parser.popParentNode();
					return true;
				}
				stream.PopState();
			}
			parser.popParentNode();

			if (!leaveOpen)
			{
				auto current = stream.Current();
				if (!stream.IsEndOfTokens() && (current.Type != TokenType_Delimiter || current.Span[0] != '}'))
				{
					parser.LogError("Unrecognised token.");
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

	static bool ParseStatementBodyInner(Parser& parser, TokenStream& stream, ASTNode* parent, StatementContainer* container, bool leaveOpen = false)
	{
		if (stream.TryGetDelimiter(';'))
		{
			return true;
		}
		std::unique_ptr<Statement> outStatement;
		bool success = StatementParserRegistry::TryParse(parser, stream, parent, outStatement, leaveOpen);

		if (!success)
		{
			while (!stream.TryGetDelimiter(';') && !stream.IsEndOfTokens())
			{
				stream.Advance();
			}

			return true;
		}

		container->AddStatement(std::move(outStatement));
		return true;
	}

	static bool ParseStatementBody(TextSpan name, ScopeType type, ASTNode* parent, Parser& parser, TokenStream& stream, std::unique_ptr<BlockStatement>& statement)
	{
		Token first;
		IF_ERR_RET_FALSE(parser.EnterScope(name, type, parent, first));

		auto blockStatement = std::make_unique<BlockStatement>(TextSpan(), parent);
		while (parser.IterateScope())
		{
			ParseStatementBodyInner(parser, stream, blockStatement.get(), blockStatement.get());
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

	public:
		static bool TryParse(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expressionOut)
		{
			auto first = stream.Current();
			parser.pushParentNode(parent);
			for (auto& subParser : parsers)
			{
				stream.PushState();
				if (subParser->TryParse(parser, stream, expressionOut))
				{
					stream.PopState(false);
					parser.popParentNode();
					return true;
				}
				stream.PopState();
			}
			parser.popParentNode();

			auto current = stream.Current();
			if (!stream.IsEndOfTokens() && current.Type != TokenType_Delimiter && current.Span[0] != ';')
			{
				parser.LogError("Unrecognised token.");
				return false;
			}

			if (!expressionOut.get())
			{
				expressionOut = std::make_unique<EmptyExpression>(first.Span.merge(stream.LastToken().Span), static_cast<ASTNode*>(nullptr));
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