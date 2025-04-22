#include "parser_helper.hpp"
#include "sub_parser_registry.hpp"
#include "pratt_parser.hpp"

#include <memory>

namespace ParserHelper
{
	template <typename T>
	static void ChainExpression(std::unique_ptr<Expression>& root, IChainExpression*& chainExpr, std::unique_ptr<T> newExpr)
	{
		auto next = newExpr.get();
		if (chainExpr)
		{
			chainExpr->chain(std::move(newExpr));
		}
		else
		{
			root = std::move(newExpr);
		}
		chainExpr = next;
	}

	template <typename T>
	static void ChainExpressionEnd(std::unique_ptr<Expression>& root, IChainExpression* chainExpr, std::unique_ptr<T> newExpr)
	{
		if (chainExpr)
		{
			chainExpr->chain(std::move(newExpr));
		}
		else
		{
			root = std::move(newExpr);
		}
	}

	bool TryParseMemberAccessPath(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expressionOut)
	{
		std::unique_ptr<Expression> root = nullptr;
		IChainExpression* chainExpr = nullptr;

		while (true)
		{
			auto start = stream.Current();

			LazySymbol baseSymbol;
			IF_ERR_RET_FALSE(parser.TryParseSymbol(SymbolRefType_Variable, baseSymbol));

			if (stream.TryGetOperator(Operator_MemberAccess))
			{
				auto memberAccessExpression = std::make_unique<MemberAccessExpression>(start.Span, parent, std::move(baseSymbol.make(root ? SymbolRefType_Member : SymbolRefType_Variable)), nullptr);
				memberAccessExpression->SetSpan(stream.MakeFromLast(start));
				ChainExpression(root, chainExpr, std::move(memberAccessExpression));
			}
			else if (stream.TryGetDelimiter('['))
			{
				auto indexerAccessExpression = std::make_unique<IndexerAccessExpression>(TextSpan(), parent, std::move(baseSymbol.make(root ? SymbolRefType_Member : SymbolRefType_Variable)));
				do
				{
					std::unique_ptr<Expression> indexExpression;
					IF_ERR_RET_FALSE(ParseExpression(parser, stream, indexerAccessExpression.get(), indexExpression));
					IF_ERR_RET_FALSE(stream.ExpectDelimiter(']'));
					indexerAccessExpression->AddIndex(std::move(indexExpression));
				} while (stream.TryGetDelimiter('['));
				indexerAccessExpression->SetSpan(stream.MakeFromLast(start));

				if (stream.TryGetOperator(Operator_MemberAccess))
				{
					auto memberAccessExpression = std::make_unique<BinaryExpression>(TextSpan(), parent, Operator_MemberAccess, nullptr, nullptr);
					memberAccessExpression->SetLeft(std::move(indexerAccessExpression));
					memberAccessExpression->SetSpan(stream.MakeFromLast(start));
					ChainExpression(root, chainExpr, std::move(memberAccessExpression));
				}
				else
				{
					ChainExpressionEnd(root, chainExpr, std::move(indexerAccessExpression));
					break;
				}
			}
			else if (stream.TryGetDelimiter('('))
			{
				auto functionExpression = std::make_unique<FunctionCallExpression>(TextSpan(), parent, std::move(baseSymbol.make(root ? SymbolRefType_Function : SymbolRefType_FunctionOrConstructor)));
				std::vector<std::unique_ptr<CallParameter>> parameters;
				IF_ERR_RET_FALSE(ParseFunctionCallInner(parser, stream, functionExpression.get(), parameters));
				functionExpression->SetParameters(std::move(parameters));
				functionExpression->SetSpan(stream.MakeFromLast(start));
				ChainExpressionEnd(root, chainExpr, std::move(functionExpression));
				break;
			}
			else
			{
				auto symbolExpression = std::make_unique<SymbolRefExpression>(start.Span, parent, std::move(baseSymbol.make(root ? SymbolRefType_Member : SymbolRefType_Variable)));
				ChainExpressionEnd(root, chainExpr, std::move(symbolExpression));
				break;
			}
		}

		if (!root)
		{
			return false;
		}

		root->SetParent(parent);
		expressionOut = std::move(root);

		return true;
	}

	static bool ParseFunctionCallParameter(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<CallParameter>& parameter)
	{
		auto callParameter = std::make_unique<CallParameter>(TextSpan(), parent, nullptr);
		std::unique_ptr<Expression> expression;
		IF_ERR_RET_FALSE(ParseExpression(parser, stream, callParameter.get(), expression));
		callParameter->SetSpan(expression->GetSpan());
		callParameter->SetExpression(std::move(expression));
		parameter = std::move(callParameter);
		return true;
	}

	bool ParseFunctionCallInner(Parser& parser, TokenStream& stream, ASTNode* parent, std::vector<std::unique_ptr<CallParameter>>& parameters)
	{
		bool firstParam = true;
		while (!stream.TryGetDelimiter(')'))
		{
			if (!firstParam)
			{
				IF_ERR_RET_FALSE(stream.ExpectDelimiter(','));
			}
			firstParam = false;
			std::unique_ptr<CallParameter> parameter;
			IF_ERR_RET_FALSE(ParseFunctionCallParameter(parser, stream, parent, parameter));
			parameters.push_back(std::move(parameter));
		}

		return true;
	}

	bool ParseFunctionCallInner(const Token& start, LazySymbol& lazy, Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<FunctionCallExpression>& expression)
	{
		std::vector<std::unique_ptr<CallParameter>> parameters;
		IF_ERR_RET_FALSE(ParseFunctionCallInner(parser, stream, parent, parameters));

		auto span = start.Span.merge(stream.LastToken().Span);

		std::unique_ptr<SymbolRef> symbol;
		if (!symbol.get())
		{
			symbol = lazy.make();
		}

		expression = std::make_unique<FunctionCallExpression>(span, parent, std::move(symbol), std::move(parameters));
		return true;
	}

	bool TryParseFunctionCall(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<FunctionCallExpression>& expression)
	{
		auto start = stream.Current();
		std::unique_ptr<SymbolRef> symbol;
		LazySymbol lazy;

		IF_ERR_RET_FALSE(parser.TryParseSymbol(SymbolRefType_FunctionOrConstructor, lazy));
		IF_ERR_RET_FALSE(stream.TryGetDelimiter('('));
		IF_ERR_RET_FALSE(ParseFunctionCallInner(start, lazy, parser, stream, parent, expression));

		return true;
	}

	bool TryParseSymbol(Parser& parser, TokenStream& stream, std::unique_ptr<Expression>& expressionOut)
	{
		auto current = stream.Current();
		LazySymbol symbol;
		if (parser.TryParseSymbol(SymbolRefType_Any, symbol))
		{
			expressionOut = std::make_unique<SymbolRefExpression>(current.Span, static_cast<ASTNode*>(nullptr), std::move(symbol.make()));
			return true;
		}
		return false;
	}

	bool TryParseLiteralExpression(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<LiteralExpression>& expressionOut)
	{
		auto current = stream.Current();

		TextSpan span;
		Number number;
		if (stream.TryGetLiteral(span))
		{
			expressionOut = std::make_unique<LiteralExpression>(current.Span, parent, current);
			return true;
		}
		else if (stream.TryGetNumber(number))
		{
			expressionOut = std::make_unique<LiteralExpression>(current.Span, parent, current);
			return true;
		}
		else if (stream.TryGetKeywords({ Keyword_True, Keyword_False, Keyword_Null }))
		{
			expressionOut = std::make_unique<LiteralExpression>(current.Span, parent, current);
			return true;
		}

		return false;
	}

	bool TryParseInitializationExpression(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<InitializationExpression>& expressionOut)
	{
		auto root = std::make_unique<InitializationExpression>(stream.Current().Span, parent);
		IF_ERR_RET_FALSE(parser.EnterScope(TextSpan(), ScopeType_Initialization, root.get()));
		std::stack<InitializationExpression*> stack;
		InitializationExpression* current = root.get();

		while (true)
		{
			bool isFirst = true;
			while (parser.IterateScope())
			{
				if (!isFirst)
				{
					IF_ERR_RET_FALSE(stream.ExpectDelimiter(','));
				}
				isFirst = false;

				auto token = stream.Current();
				if (token.isDelimiterOf('{'))
				{
					auto node = std::make_unique<InitializationExpression>(token.Span, current);
					IF_ERR_RET_FALSE(parser.EnterScope(TextSpan(), ScopeType_Initialization, node.get()));
					stack.push(current);
					auto next = node.get();
					current->AddParameter(std::move(node));
					current = next;
					isFirst = true;
				}
				else
				{
					std::unique_ptr<Expression> expression;
					IF_ERR_RET_FALSE(ParseExpression(parser, stream, current, expression));
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
}