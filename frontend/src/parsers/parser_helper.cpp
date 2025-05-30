#include "parser_helper.hpp"
#include "sub_parser_registry.hpp"
#include "hybrid_expr_parser.hpp"
#include "parser.hpp"
#include "member_path_parser.hpp"

namespace HXSL
{
#define ERR_RETURN_FALSE_INTERNAL(code) \
	do { \
		Log(code, stream->Current()); \
		return false; \
	} while (0)

	bool ParserHelper::TryParseMemberAccessPath(Parser& parser, TokenStream& stream, ast_ptr<Expression>& expressionOut, Expression** headOut)
	{
		MemberPathParser context = MemberPathParser(parser, stream);

		if (!context.TryParse())
		{
			return false;
		}

		expressionOut = context.TakeRoot();

		if (headOut)
		{
			*headOut = context.Head();
		}

		return true;
	}

	static bool ParseFunctionCallParameter(Parser& parser, TokenStream& stream, ast_ptr<FunctionCallParameter>& parameter)
	{
		auto callParameter = make_ast_ptr<FunctionCallParameter>(TextSpan(), nullptr);
		ast_ptr<Expression> expression;
		IF_ERR_RET_FALSE(HybridExpressionParser::ParseExpression(parser, stream, expression));
		callParameter->SetSpan(expression->GetSpan());
		callParameter->SetExpression(std::move(expression));
		parameter = std::move(callParameter);
		return true;
	}

	bool ParserHelper::ParseFunctionCallInner(Parser& parser, TokenStream& stream, std::vector<ast_ptr<FunctionCallParameter>>& parameters)
	{
		bool firstParam = true;
		while (!stream.TryGetDelimiter(')'))
		{
			if (!firstParam)
			{
				if (!stream.ExpectDelimiter(',', EXPECTED_COMMA))
				{
					if (!parser.TryRecoverParameterList())
					{
						break;
					}
				}
			}
			firstParam = false;
			ast_ptr<FunctionCallParameter> parameter;
			while (!ParseFunctionCallParameter(parser, stream, parameter))
			{
				parser.Log(UNEXPECTED_TOKEN, stream.Current());
				if (!parser.TryRecoverParameterList())
				{
					break;
				}
				continue;
			}
			parameters.push_back(std::move(parameter));
		}

		return true;
	}

	bool ParserHelper::ParseFunctionCallInner(const Token& start, LazySymbol& lazy, Parser& parser, TokenStream& stream, ast_ptr<FunctionCallExpression>& expression)
	{
		std::vector<ast_ptr<FunctionCallParameter>> parameters;
		IF_ERR_RET_FALSE(ParseFunctionCallInner(parser, stream, parameters));

		auto span = start.Span.merge(stream.LastToken().Span);

		ast_ptr<SymbolRef> symbol;
		if (!symbol.get())
		{
			symbol = lazy.make();
		}

		expression = make_ast_ptr<FunctionCallExpression>(span, std::move(symbol), std::move(parameters));
		return true;
	}

	bool ParserHelper::TryParseFunctionCall(Parser& parser, TokenStream& stream, ast_ptr<FunctionCallExpression>& expression)
	{
		auto start = stream.Current();
		ast_ptr<SymbolRef> symbol;
		LazySymbol lazy;

		IF_ERR_RET_FALSE(parser.TryParseSymbol(SymbolRefType_FunctionOrConstructor, lazy));
		IF_ERR_RET_FALSE(stream.TryGetDelimiter('('));
		IF_ERR_RET_FALSE(ParseFunctionCallInner(start, lazy, parser, stream, expression));

		return true;
	}

	bool ParserHelper::TryParseSymbol(Parser& parser, TokenStream& stream, ast_ptr<Expression>& expressionOut)
	{
		auto current = stream.Current();
		LazySymbol symbol;
		if (parser.TryParseSymbol(SymbolRefType_Any, symbol))
		{
			expressionOut = make_ast_ptr<MemberReferenceExpression>(current.Span, std::move(symbol.make()));
			return true;
		}
		return false;
	}

	bool ParserHelper::TryParseLiteralExpression(Parser& parser, TokenStream& stream, ast_ptr<LiteralExpression>& expressionOut)
	{
		auto current = stream.Current();

		TextSpan span;
		Number number;
		if (stream.TryGetLiteral(span))
		{
			expressionOut = make_ast_ptr<LiteralExpression>(current.Span, current);
			return true;
		}
		else if (stream.TryGetNumber(number))
		{
			expressionOut = make_ast_ptr<LiteralExpression>(current.Span, current);
			return true;
		}
		else if (stream.TryGetKeywords({ Keyword_True, Keyword_False, Keyword_Null }))
		{
			expressionOut = make_ast_ptr<LiteralExpression>(current.Span, current);
			return true;
		}

		return false;
	}

	bool ParserHelper::TryParseInitializationExpression(Parser& parser, TokenStream& stream, ast_ptr<InitializationExpression>& expressionOut)
	{
		auto root = make_ast_ptr<InitializationExpression>(stream.Current().Span);
		parser.EnterScope(ScopeType_Initialization, root.get(), true);
		std::stack<InitializationExpression*> stack;
		InitializationExpression* current = root.get();

		while (true)
		{
			bool isFirst = true;
			while (parser.IterateScope(root.get()))
			{
				if (!isFirst)
				{
					stream.ExpectDelimiter(',', EXPECTED_COMMA);
				}
				isFirst = false;

				auto token = stream.Current();
				if (token.isDelimiterOf('{'))
				{
					auto node = make_ast_ptr<InitializationExpression>(token.Span);
					parser.EnterScope(ScopeType_Initialization, node.get(), true);
					stack.push(current);
					auto next = node.get();
					current->AddParameter(std::move(node));
					current = next;
					isFirst = true;
				}
				else
				{
					ast_ptr<Expression> expression;
					IF_ERR_RET_FALSE(HybridExpressionParser::ParseExpression(parser, stream, expression));
					current->AddParameter(std::move(expression));
				}
			}

			current->SetSpan(stream.MakeFromLast(current->GetSpan()));
			if (stack.empty())
			{
				break;
			}
			current = stack.top();
			stack.pop();
		}

		expressionOut = std::move(root);
		return true;
	}

	bool ParserHelper::MakeConcreteSymbolRef(Expression* expression, SymbolRefType type, ast_ptr<SymbolRef>& symbolOut)
	{
		bool isFQN = false;
		Expression* current = expression;
		TextSpan span = current->GetSpan();
		while (current)
		{
			auto& type = current->GetType();
			if (type != NodeType_MemberAccessExpression && type != NodeType_MemberReferenceExpression)
			{
				return false;
			}

			span = span.merge(current->GetSpan());

			if (auto chainGetter = dyn_cast<ChainExpression>(current))
			{
				current = chainGetter->GetNextExpression().get();
				continue;
			}

			break;
		}

		symbolOut = make_ast_ptr<SymbolRef>(span, type, isFQN);

		return true;
	}

	bool Parser::TryParseSymbol(const SymbolRefType& type, LazySymbol& symbol)
	{
		bool fqn = false;
		TextSpan span = stream->Current().Span;
		while (true)
		{
			auto start = stream->Current();

			TextSpan baseSymbol;
			IF_ERR_RET_FALSE(TryParseSymbolInternal(type, baseSymbol));

			if (stream->TryGetOperator(Operator_MemberAccess))
			{
				span = stream->MakeFromLast(span);
				fqn = true;
			}
			else
			{
				span = stream->MakeFromLast(span);
				break;
			}
		}

		symbol = LazySymbol(span, type, fqn);
		return true;
	}

	bool Parser::ParseSymbol(SymbolRefType expectedType, ast_ptr<SymbolRef>& type)
	{
		LazySymbol symbol;
		if (!TryParseSymbol(expectedType, symbol))
		{
			ERR_RETURN_FALSE_INTERNAL(EXPECTED_IDENTIFIER);
		}

		type = symbol.make();

		return true;
	}

	bool Parser::TryParseArraySizes(std::vector<size_t>& arraySizes)
	{
		if (stream->Current().isDelimiterOf('['))
		{
			ParseArraySizes(arraySizes);
			return true;
		}
		return false;
	}

	void Parser::ParseArraySizes(std::vector<size_t>& arraySizes)
	{
		while (stream->TryGetDelimiter('['))
		{
			Number num;
			stream->ExpectNumeric(num, EXPECTED_NUMBER);
			if (!num.IsIntegral())
			{
				Log(ARRAY_SIZE_MUST_BE_INT, stream->LastToken());
			}
			if (num.IsSigned() && num.IsNegative())
			{
				Log(ARRAY_SIZE_CANNOT_BE_NEG, stream->LastToken());
			}
			arraySizes.push_back(num.ToSizeT());
			stream->ExpectDelimiter(']', EXPECTED_RIGHT_BRACKET);
		}
	}
}