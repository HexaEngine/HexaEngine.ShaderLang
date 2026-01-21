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
				auto span = stream.MakeFromLast(start);
				auto memberAccessExpression = MemberAccessExpression::Create(span, baseSymbol.make(root ? SymbolRefType_Member : SymbolRefType_Identifier), nullptr);
				Chain(memberAccessExpression);
			}
			else if (stream.TryGetDelimiter('['))
			{
				Expression* indexExpression;
				HybridExpressionParser::ParseExpression(parser, stream, indexExpression);
				stream.ExpectDelimiter(']', EXPECTED_RIGHT_BRACKET);
				auto span = stream.MakeFromLast(start);
				auto indexerAccessExpression = IndexerAccessExpression::Create(span, baseSymbol.make(root ? SymbolRefType_Member : SymbolRefType_Identifier, !wantsIdentifier), indexExpression);
				ChainOrEnd(indexerAccessExpression);
			}
			else if (stream.TryGetDelimiter('('))
			{
				std::vector<FunctionCallParameter*> parameters;
				IF_ERR_RET_FALSE(ParserHelper::ParseFunctionCallInner(parser, stream, parameters));
				auto span = stream.MakeFromLast(start);
				auto functionExpression = FunctionCallExpression::Create(span, baseSymbol.make(SymbolRefType_FunctionOverload), parameters);
				ChainOrEnd(functionExpression);
			}
			else if (stream.TryGetKeyword(Keyword_New))
			{
				std::vector<FunctionCallParameter*> parameters;
				IF_ERR_RET_FALSE(ParserHelper::ParseFunctionCallInner(parser, stream, parameters));
				auto span = stream.MakeFromLast(start);
				auto constructorExpression = ConstructorCallExpression::Create(span, baseSymbol.make(SymbolRefType_Constructor), parameters);
				ChainOrEnd(constructorExpression);
			}
			else
			{
				auto symbolExpression = MemberReferenceExpression::Create(start.Span, baseSymbol.make(root ? SymbolRefType_Member : SymbolRefType_Identifier));
				ChainEnd(symbolExpression);
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