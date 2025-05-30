#include "member_path_parser.hpp"
#include "parser_helper.hpp"
#include "sub_parser_registry.hpp"
#include "hybrid_expr_parser.hpp"
#include "parser.hpp"

namespace HXSL
{
	bool MemberPathParser::TryParse()
	{
		bool first = true;
		while (parsing && !stream.IsEndOfTokens() && !stream.HasCriticalErrors())
		{
			auto start = stream.Current();

			LazySymbol baseSymbol = {};
			if (wantsIdentifier)
			{
				TextSpan span;
				if (!(parser.TryParseSymbolInternal(SymbolRefType_Identifier, span)))
				{
					if (first)
					{
						return false;
					}

					parser.Log(UNEXPECTED_TOKEN, start);
					stream.TryAdvance();
					continue;
				}
				first = false;
				baseSymbol = LazySymbol(span, SymbolRefType_Identifier, false);
			}

			if (stream.TryGetOperator(Operator_MemberAccess))
			{
				auto memberAccessExpression = make_ast_ptr<MemberAccessExpression>(start.Span, std::move(baseSymbol.make(root ? SymbolRefType_Member : SymbolRefType_Identifier)), nullptr);
				memberAccessExpression->SetSpan(stream.MakeFromLast(start));
				Chain(std::move(memberAccessExpression));
			}
			else if (stream.TryGetDelimiter('['))
			{
				auto indexerAccessExpression = make_ast_ptr<IndexerAccessExpression>(TextSpan(), std::move(baseSymbol.make(root ? SymbolRefType_Member : SymbolRefType_Identifier, !wantsIdentifier)));
				ast_ptr<Expression> indexExpression;
				HybridExpressionParser::ParseExpression(parser, stream, indexExpression);
				stream.ExpectDelimiter(']', EXPECTED_RIGHT_BRACKET);
				indexerAccessExpression->SetIndexExpression(std::move(indexExpression));
				indexerAccessExpression->SetSpan(stream.MakeFromLast(start));
				ChainOrEnd(std::move(indexerAccessExpression));
			}
			else if (stream.TryGetDelimiter('('))
			{
				auto functionExpression = make_ast_ptr<FunctionCallExpression>(TextSpan(), std::move(baseSymbol.make(root ? SymbolRefType_FunctionOverload : SymbolRefType_FunctionOrConstructor)));
				std::vector<ast_ptr<FunctionCallParameter>> parameters;
				IF_ERR_RET_FALSE(ParserHelper::ParseFunctionCallInner(parser, stream, parameters));
				functionExpression->SetParameters(std::move(parameters));
				functionExpression->SetSpan(stream.MakeFromLast(start));
				ChainOrEnd(std::move(functionExpression));
			}
			else
			{
				auto symbolExpression = make_ast_ptr<MemberReferenceExpression>(start.Span, std::move(baseSymbol.make(root ? SymbolRefType_Member : SymbolRefType_Identifier)));
				ChainEnd(std::move(symbolExpression));
				break;
			}
		}

		if (!root)
		{
			return false;
		}

		return true;
	}
}