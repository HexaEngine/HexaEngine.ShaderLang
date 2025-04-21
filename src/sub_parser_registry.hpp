#ifndef SUB_PARSER_REGISTRY_H
#define SUB_PARSER_REGISTRY_H

#include "sub_parser.hpp"
#include <memory>

namespace HXSL
{
	class HXSLSubParserRegistry
	{
	private:
		static std::vector<std::unique_ptr<HXSLSubParser>> parsers;

	public:
		static bool TryParse(HXSLParser& parser, TokenStream& stream, HXSLNode* parent, HXSLCompilation* compilation)
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
		static typename std::enable_if<std::is_base_of<HXSLSubParser, SubParserType>::value>::type
			Register()
		{
			parsers.push_back(std::make_unique<SubParserType>());
		}
	};

	class HXSLStatementParserRegistry
	{
	private:
		static std::vector<std::unique_ptr<HXSLStatementParser>> parsers;

	public:
		static bool TryParse(HXSLParser& parser, TokenStream& stream, HXSLNode* parent, std::unique_ptr<HXSLStatement>& statementOut, bool leaveOpen = false)
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
		static typename std::enable_if<std::is_base_of<HXSLStatementParser, SubParserType>::value>::type
			Register()
		{
			parsers.push_back(std::make_unique<SubParserType>());
		}
	};

	static bool ParseStatementBodyInner(HXSLParser& parser, TokenStream& stream, HXSLNode* parent, HXSLStatementContainer* container, bool leaveOpen = false)
	{
		if (stream.TryGetDelimiter(';'))
		{
			return true;
		}
		std::unique_ptr<HXSLStatement> outStatement;
		bool success = HXSLStatementParserRegistry::TryParse(parser, stream, parent, outStatement, leaveOpen);

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

	static bool ParseStatementBody(TextSpan name, ScopeType type, HXSLNode* parent, HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLBlockStatement>& statement)
	{
		Token first;
		IF_ERR_RET_FALSE(parser.EnterScope(name, type, parent, first));

		auto blockStatement = std::make_unique<HXSLBlockStatement>(TextSpan(), parent);
		while (parser.IterateScope())
		{
			ParseStatementBodyInner(parser, stream, blockStatement.get(), blockStatement.get());
		}

		auto span = first.Span.merge(stream.LastToken().Span);
		blockStatement->SetSpan(span);
		statement = std::move(blockStatement);
		return true;
	}

	class HXSLExpressionParserRegistry
	{
	private:
		static std::vector<std::unique_ptr<HXSLExpressionParser>> parsers;

	public:
		static bool TryParse(HXSLParser& parser, TokenStream& stream, HXSLNode* parent, std::unique_ptr<HXSLExpression>& expressionOut)
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
				expressionOut = std::make_unique<HXSLEmptyExpression>(first.Span.merge(stream.LastToken().Span), static_cast<HXSLNode*>(nullptr));
				return true;
			}

			return false;
		}

		template <typename SubParserType>
		static typename std::enable_if<std::is_base_of<HXSLExpressionParser, SubParserType>::value>::type
			Register()
		{
			parsers.push_back(std::make_unique<SubParserType>());
		}
	};
}

#endif