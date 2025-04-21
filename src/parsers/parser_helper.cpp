#include "parser_helper.hpp"
#include "sub_parser_registry.hpp"
#include "pratt_parser.hpp"

#include <memory>

namespace ParserHelper
{
	template <typename T>
	static void ChainExpression(std::unique_ptr<HXSLExpression>& root, IHXSLChainExpression*& chainExpr, std::unique_ptr<T> newExpr)
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
	static void ChainExpressionEnd(std::unique_ptr<HXSLExpression>& root, IHXSLChainExpression* chainExpr, std::unique_ptr<T> newExpr)
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

	bool TryParseMemberAccessPath(HXSLParser& parser, TokenStream& stream, HXSLNode* parent, std::unique_ptr<HXSLExpression>& expressionOut)
	{
		std::unique_ptr<HXSLExpression> root = nullptr;
		IHXSLChainExpression* chainExpr = nullptr;

		while (true)
		{
			auto start = stream.Current();

			LazySymbol baseSymbol;
			IF_ERR_RET_FALSE(parser.TryParseSymbol(HXSLSymbolRefType_Variable, baseSymbol));

			if (stream.TryGetOperator(HXSLOperator_MemberAccess))
			{
				auto memberAccessExpression = std::make_unique<HXSLMemberAccessExpression>(start.Span, parent, std::move(baseSymbol.make(root ? HXSLSymbolRefType_Member : HXSLSymbolRefType_Variable)), nullptr);
				memberAccessExpression->SetSpan(stream.MakeFromLast(start));
				ChainExpression(root, chainExpr, std::move(memberAccessExpression));
			}
			else if (stream.TryGetDelimiter('['))
			{
				auto indexerAccessExpression = std::make_unique<HXSLIndexerAccessExpression>(TextSpan(), parent, std::move(baseSymbol.make(root ? HXSLSymbolRefType_Member : HXSLSymbolRefType_Variable)));
				do
				{
					std::unique_ptr<HXSLExpression> indexExpression;
					IF_ERR_RET_FALSE(ParseExpression(parser, stream, indexerAccessExpression.get(), indexExpression));
					IF_ERR_RET_FALSE(stream.ExpectDelimiter(']'));
					indexerAccessExpression->AddIndex(std::move(indexExpression));
				} while (stream.TryGetDelimiter('['));
				indexerAccessExpression->SetSpan(stream.MakeFromLast(start));

				if (stream.TryGetOperator(HXSLOperator_MemberAccess))
				{
					auto memberAccessExpression = std::make_unique<HXSLBinaryExpression>(TextSpan(), parent, HXSLOperator_MemberAccess, nullptr, nullptr);
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
				auto functionExpression = std::make_unique<HXSLFunctionCallExpression>(TextSpan(), parent, std::move(baseSymbol.make(root ? HXSLSymbolRefType_Function : HXSLSymbolRefType_FunctionOrConstructor)));
				std::vector<std::unique_ptr<HXSLCallParameter>> parameters;
				IF_ERR_RET_FALSE(ParseFunctionCallInner(parser, stream, functionExpression.get(), parameters));
				functionExpression->SetParameters(std::move(parameters));
				functionExpression->SetSpan(stream.MakeFromLast(start));
				ChainExpressionEnd(root, chainExpr, std::move(functionExpression));
				break;
			}
			else
			{
				auto symbolExpression = std::make_unique<HXSLSymbolRefExpression>(start.Span, parent, std::move(baseSymbol.make(root ? HXSLSymbolRefType_Member : HXSLSymbolRefType_Variable)));
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

	static bool ParseFunctionCallParameter(HXSLParser& parser, TokenStream& stream, HXSLNode* parent, std::unique_ptr<HXSLCallParameter>& parameter)
	{
		auto callParameter = std::make_unique<HXSLCallParameter>(TextSpan(), parent, nullptr);
		std::unique_ptr<HXSLExpression> expression;
		IF_ERR_RET_FALSE(ParseExpression(parser, stream, callParameter.get(), expression));
		callParameter->SetSpan(expression->GetSpan());
		callParameter->SetExpression(std::move(expression));
		parameter = std::move(callParameter);
		return true;
	}

	bool ParseFunctionCallInner(HXSLParser& parser, TokenStream& stream, HXSLNode* parent, std::vector<std::unique_ptr<HXSLCallParameter>>& parameters)
	{
		bool firstParam = true;
		while (!stream.TryGetDelimiter(')'))
		{
			if (!firstParam)
			{
				IF_ERR_RET_FALSE(stream.ExpectDelimiter(','));
			}
			firstParam = false;
			std::unique_ptr<HXSLCallParameter> parameter;
			IF_ERR_RET_FALSE(ParseFunctionCallParameter(parser, stream, parent, parameter));
			parameters.push_back(std::move(parameter));
		}

		return true;
	}

	bool ParseFunctionCallInner(const Token& start, LazySymbol& lazy, HXSLParser& parser, TokenStream& stream, HXSLNode* parent, std::unique_ptr<HXSLFunctionCallExpression>& expression)
	{
		std::vector<std::unique_ptr<HXSLCallParameter>> parameters;
		IF_ERR_RET_FALSE(ParseFunctionCallInner(parser, stream, parent, parameters));

		auto span = start.Span.merge(stream.LastToken().Span);

		std::unique_ptr<HXSLSymbolRef> symbol;
		if (!symbol.get())
		{
			symbol = lazy.make();
		}

		expression = std::make_unique<HXSLFunctionCallExpression>(span, parent, std::move(symbol), std::move(parameters));
		return true;
	}

	bool TryParseFunctionCall(HXSLParser& parser, TokenStream& stream, HXSLNode* parent, std::unique_ptr<HXSLFunctionCallExpression>& expression)
	{
		auto start = stream.Current();
		std::unique_ptr<HXSLSymbolRef> symbol;
		LazySymbol lazy;

		IF_ERR_RET_FALSE(parser.TryParseSymbol(HXSLSymbolRefType_FunctionOrConstructor, lazy));
		IF_ERR_RET_FALSE(stream.TryGetDelimiter('('));
		IF_ERR_RET_FALSE(ParseFunctionCallInner(start, lazy, parser, stream, parent, expression));

		return true;
	}

	bool TryParseSymbol(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLExpression>& expressionOut)
	{
		auto current = stream.Current();
		LazySymbol symbol;
		if (parser.TryParseSymbol(HXSLSymbolRefType_Any, symbol))
		{
			expressionOut = std::make_unique<HXSLSymbolRefExpression>(current.Span, static_cast<HXSLNode*>(nullptr), std::move(symbol.make()));
			return true;
		}
		return false;
	}

	bool TryParseLiteralExpression(HXSLParser& parser, TokenStream& stream, HXSLNode* parent, std::unique_ptr<HXSLLiteralExpression>& expressionOut)
	{
		auto current = stream.Current();

		TextSpan span;
		HXSLNumber number;
		if (stream.TryGetLiteral(span))
		{
			expressionOut = std::make_unique<HXSLLiteralExpression>(current.Span, parent, current);
			return true;
		}
		else if (stream.TryGetNumber(number))
		{
			expressionOut = std::make_unique<HXSLLiteralExpression>(current.Span, parent, current);
			return true;
		}
		else if (stream.TryGetKeywords({ HXSLKeyword_True, HXSLKeyword_False, HXSLKeyword_Null }))
		{
			expressionOut = std::make_unique<HXSLLiteralExpression>(current.Span, parent, current);
			return true;
		}

		return false;
	}

	bool TryParseInitializationExpression(HXSLParser& parser, TokenStream& stream, HXSLNode* parent, std::unique_ptr<HXSLInitializationExpression>& expressionOut)
	{
		auto root = std::make_unique<HXSLInitializationExpression>(stream.Current().Span, parent);
		IF_ERR_RET_FALSE(parser.EnterScope(TextSpan(), ScopeType_Initialization, root.get()));
		std::stack<HXSLInitializationExpression*> stack;
		HXSLInitializationExpression* current = root.get();

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
					auto node = std::make_unique<HXSLInitializationExpression>(token.Span, current);
					IF_ERR_RET_FALSE(parser.EnterScope(TextSpan(), ScopeType_Initialization, node.get()));
					stack.push(current);
					auto next = node.get();
					current->AddParameter(std::move(node));
					current = next;
					isFirst = true;
				}
				else
				{
					std::unique_ptr<HXSLExpression> expression;
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