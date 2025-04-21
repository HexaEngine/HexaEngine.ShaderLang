#include "statement_parser.hpp"
#include "sub_parser_registry.hpp"
#include "expression_parser.hpp"
#include "parser_helper.hpp"
#include "pratt_parser.hpp"

#include <memory>
namespace HXSL
{
	bool HXSLMiscKeywordStatementParser::TryParse(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLStatement>& statementOut)
	{
		auto start = stream.Current();
		if (stream.TryGetKeyword(HXSLKeyword_Break))
		{
			IF_ERR_RET_FALSE(parser.inScope(ScopeFlags_InsideLoop | ScopeFlags_InsideSwitch, "'break' statement used outside of a loop or switch context."));
			IF_ERR_RET_FALSE(stream.ExpectDelimiter(';'));
			statementOut = std::make_unique<HXSLBreakStatement>(TextSpan(), parser.parentNode());
			return true;
		}
		else if (stream.TryGetKeyword(HXSLKeyword_Continue))
		{
			IF_ERR_RET_FALSE(parser.inScope(ScopeFlags_InsideLoop, "'continue' statement used outside of a loop context."));
			IF_ERR_RET_FALSE(stream.ExpectDelimiter(';'));
			statementOut = std::make_unique<HXSLContinueStatement>(TextSpan(), parser.parentNode());
			return true;
		}
		else if (stream.TryGetKeyword(HXSLKeyword_Discard))
		{
			IF_ERR_RET_FALSE(parser.inScope(ScopeFlags_InsideFunction));
			IF_ERR_RET_FALSE(stream.ExpectDelimiter(';'));
			statementOut = std::make_unique<HXSLDiscardStatement>(TextSpan(), parser.parentNode());
			return true;
		}
		return false;
	}

	bool HXSLBlockStatementParser::TryParse(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLStatement>& statementOut)
	{
		auto start = stream.Current();
		if (!start.isDelimiterOf('{'))
		{
			return false;
		}
		std::unique_ptr<HXSLBlockStatement> body;
		IF_ERR_RET_FALSE(ParseStatementBody(TextSpan(), parser.scopeType(), parser.parentNode(), parser, stream, body));
		statementOut = std::move(body);
		return true;
	}

	bool HXSLForStatementParser::TryParse(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLStatement>& statementOut)
	{
		auto start = stream.Current();

		std::unique_ptr<HXSLAttributeDeclaration> attribute;
		parser.TryParseAttribute(attribute);

		if (!stream.TryGetKeyword(HXSLKeyword_For))
		{
			return false;
		}

		IF_ERR_RET_FALSE(stream.ExpectDelimiter('('));

		auto forStatement = std::make_unique<HXSLForStatement>(TextSpan(), parser.parentNode());

		if (attribute)
		{
			forStatement->AddAttribute(std::move(attribute));
		}

		std::unique_ptr<HXSLStatement> init;
		IF_ERR_RET_FALSE(HXSLStatementParserRegistry::TryParse(parser, stream, forStatement.get(), init));
		IF_ERR_RET_FALSE(stream.LastToken().isDelimiterOf(';'));
		forStatement->SetInit(std::move(init));

		std::unique_ptr<HXSLExpression> condition;
		IF_ERR_RET_FALSE(ParseExpression(parser, stream, forStatement.get(), condition));
		IF_ERR_RET_FALSE(stream.ExpectDelimiter(';'));
		forStatement->SetCondition(std::move(condition));

		std::unique_ptr<HXSLExpression> iteration;
		IF_ERR_RET_FALSE(ParseExpression(parser, stream, forStatement.get(), iteration));
		IF_ERR_RET_FALSE(stream.ExpectDelimiter(')'));
		forStatement->SetIteration(std::move(iteration));

		std::unique_ptr<HXSLBlockStatement> body; // TODO: single line statements.
		IF_ERR_RET_FALSE(ParseStatementBody(TextSpan(), ScopeType_For, forStatement.get(), parser, stream, body));
		forStatement->SetBody(std::move(body));

		forStatement->SetSpan(stream.MakeFromLast(start));

		statementOut = std::move(forStatement);
		return true;
	}

	static bool ParseCaseInner(HXSLParser& parser, TokenStream& stream, ASTNode* parent, HXSLStatementContainer* container)
	{
		static const std::unordered_set<char> unexpectedDelimiters = { '}', ')', '.', ':' };
		static const std::unordered_set<HXSLNodeType> breakoutTypes = { HXSLNodeType_BreakStatement, HXSLNodeType_ReturnStatement, HXSLNodeType_ContinueStatement, HXSLNodeType_DiscardStatement };

		while (true)
		{
			IF_ERR_RET_FALSE(stream.ExpectNoDelimiters(unexpectedDelimiters));
			IF_ERR_RET_FALSE(ParseStatementBodyInner(parser, stream, parent, container, true));
			auto& statements = container->GetStatements();
			auto& last = statements[statements.size() - 1];
			if (last->IsAnyTypeOf(breakoutTypes)) break;
		}

		return true;
	}

	bool HXSLSwitchStatementParser::TryParse(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLStatement>& statementOut)
	{
		auto start = stream.Current();

		std::unique_ptr<HXSLAttributeDeclaration> attribute;
		parser.TryParseAttribute(attribute);

		if (!stream.TryGetKeyword(HXSLKeyword_Switch))
		{
			return false;
		}

		IF_ERR_RET_FALSE(stream.ExpectDelimiter('('));
		auto switchStatement = std::make_unique<HXSLSwitchStatement>(TextSpan(), parser.parentNode());
		if (attribute)
		{
			switchStatement->AddAttribute(std::move(attribute));
		}
		std::unique_ptr<HXSLExpression> expression;
		IF_ERR_RET_FALSE(ParseExpression(parser, stream, switchStatement.get(), expression));
		IF_ERR_RET_FALSE(stream.ExpectDelimiter(')'));
		switchStatement->SetExpression(std::move(expression));

		IF_ERR_RET_FALSE(parser.EnterScope(TextSpan(), ScopeType_Switch, switchStatement.get()));

		static const std::unordered_set<HXSLKeyword> keywords = { HXSLKeyword_Case, HXSLKeyword_Default };

		while (parser.IterateScope())
		{
			HXSLKeyword keyword;
			auto caseStart = stream.Current();
			IF_ERR_RET_FALSE(stream.ExpectKeywords(keywords, keyword));

			if (keyword == HXSLKeyword_Case)
			{
				auto caseStatement = std::make_unique<HXSLCaseStatement>(TextSpan(), switchStatement.get(), nullptr);
				std::unique_ptr<HXSLExpression> caseExpression;
				IF_ERR_RET_FALSE(ParseExpression(parser, stream, caseStatement.get(), caseExpression));
				caseStatement->SetExpression(std::move(caseExpression));
				IF_ERR_RET_FALSE(stream.ExpectOperator(HXSLOperator_Colon));
				IF_ERR_RET_FALSE(ParseCaseInner(parser, stream, caseStatement.get(), caseStatement.get()));
				caseStatement->SetSpan(stream.MakeFromLast(caseStart));
				switchStatement->AddCase(std::move(caseStatement));
			}
			else
			{
				if (switchStatement->GetDefaultCase())
				{
					ERR_RETURN_FALSE(parser, "Cannot declare two default cases in a switch-case.");
				}

				IF_ERR_RET_FALSE(stream.ExpectOperator(HXSLOperator_Colon));
				auto defaultCaseStatement = std::make_unique<HXSLDefaultCaseStatement>(TextSpan(), switchStatement.get());
				IF_ERR_RET_FALSE(ParseCaseInner(parser, stream, defaultCaseStatement.get(), defaultCaseStatement.get()));
				defaultCaseStatement->SetSpan(stream.MakeFromLast(caseStart));
				switchStatement->SetDefaultCase(std::move(defaultCaseStatement));
			}
		}

		switchStatement->SetSpan(stream.MakeFromLast(start));
		statementOut = std::move(switchStatement);
		return true;
	}

	bool HXSLIfStatementParser::TryParse(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLStatement>& statementOut)
	{
		auto start = stream.Current();

		std::unique_ptr<HXSLAttributeDeclaration> attribute;
		parser.TryParseAttribute(attribute);

		if (stream.TryGetKeyword(HXSLKeyword_If))
		{
			IF_ERR_RET_FALSE(stream.ExpectDelimiter('('));
			auto ifStatement = std::make_unique<HXSLIfStatement>(TextSpan(), parser.parentNode(), nullptr, nullptr);
			if (attribute)
			{
				ifStatement->AddAttribute(std::move(attribute));
			}
			std::unique_ptr<HXSLExpression> expression;
			IF_ERR_RET_FALSE(ParseExpression(parser, stream, ifStatement.get(), expression));
			IF_ERR_RET_FALSE(stream.ExpectDelimiter(')'));
			ifStatement->SetExpression(std::move(expression));

			std::unique_ptr<HXSLBlockStatement> statement; // TODO: single line statements.
			IF_ERR_RET_FALSE(ParseStatementBody(TextSpan(), ScopeType_If, ifStatement.get(), parser, stream, statement));
			ifStatement->SetBody(std::move(statement));
			ifStatement->SetSpan(start.Span.merge(stream.LastToken().Span));
			statementOut = std::move(ifStatement);
			return true;
		}
		return false;
	}

	bool HXSLElseStatementParser::TryParse(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLStatement>& statementOut)
	{
		auto start = stream.Current();
		if (stream.TryGetKeyword(HXSLKeyword_Else))
		{
			if (stream.TryGetKeyword(HXSLKeyword_If))
			{
				IF_ERR_RET_FALSE(stream.ExpectDelimiter('('));
				auto elseIfStatement = std::make_unique<HXSLElseIfStatement>(TextSpan(), parser.parentNode(), nullptr, nullptr);
				std::unique_ptr<HXSLExpression> expression;
				IF_ERR_RET_FALSE(ParseExpression(parser, stream, elseIfStatement.get(), expression));
				IF_ERR_RET_FALSE(stream.ExpectDelimiter(')'));
				elseIfStatement->SetExpression(std::move(expression));

				std::unique_ptr<HXSLBlockStatement> statement; // TODO: single line statements.
				IF_ERR_RET_FALSE(ParseStatementBody(TextSpan(), ScopeType_ElseIf, elseIfStatement.get(), parser, stream, statement));
				elseIfStatement->SetBody(std::move(statement));
				elseIfStatement->SetSpan(start.Span.merge(stream.LastToken().Span));
				statementOut = std::move(elseIfStatement);
			}
			else
			{
				auto elseStatement = std::make_unique<HXSLElseStatement>(TextSpan(), parser.parentNode(), nullptr);
				std::unique_ptr<HXSLBlockStatement> statement;
				IF_ERR_RET_FALSE(ParseStatementBody(TextSpan(), ScopeType_Else, elseStatement.get(), parser, stream, statement));
				elseStatement->SetBody(std::move(statement));
				elseStatement->SetSpan(start.Span.merge(stream.LastToken().Span));
				statementOut = std::move(elseStatement);
			}

			return true;
		}
		return false;
	}

	bool HXSLWhileStatementParser::TryParse(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLStatement>& statementOut)
	{
		auto start = stream.Current();
		if (!stream.TryGetKeyword(HXSLKeyword_While))
		{
			return false;
		}

		IF_ERR_RET_FALSE(stream.ExpectDelimiter('('));
		auto whileStatement = std::make_unique<HXSLWhileStatement>(TextSpan(), parser.parentNode(), nullptr, nullptr);
		std::unique_ptr<HXSLExpression> expression;
		IF_ERR_RET_FALSE(ParseExpression(parser, stream, whileStatement.get(), expression));
		IF_ERR_RET_FALSE(stream.ExpectDelimiter(')'));
		whileStatement->SetExpression(std::move(expression));

		std::unique_ptr<HXSLBlockStatement> statement; // TODO: single line statements.
		IF_ERR_RET_FALSE(ParseStatementBody(TextSpan(), ScopeType_While, whileStatement.get(), parser, stream, statement));
		whileStatement->SetBody(std::move(statement));
		whileStatement->SetSpan(start.Span.merge(stream.LastToken().Span));
		statementOut = std::move(whileStatement);
		return true;
	}

	bool HXSLReturnStatementParser::TryParse(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLStatement>& statementOut)
	{
		auto start = stream.Current();
		if (!stream.TryGetKeyword(HXSLKeyword_Return))
		{
			return false;
		}

		auto returnStatement = std::make_unique<HXSLReturnStatement>(TextSpan(), parser.parentNode(), nullptr);
		std::unique_ptr<HXSLExpression> expression;
		IF_ERR_RET_FALSE(ParseExpression(parser, stream, returnStatement.get(), expression));
		IF_ERR_RET_FALSE(stream.ExpectDelimiter(';'));
		returnStatement->SetReturnValueExpression(std::move(expression));
		returnStatement->SetSpan(start.Span.merge(stream.LastToken().Span));
		statementOut = std::move(returnStatement);
		return true;
	}

	bool HXSLDeclarationStatementParser::TryParse(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLStatement>& statementOut)
	{
		auto start = stream.Current();
		LazySymbol symbol;
		TextSpan identifer;
		if (!parser.TryParseSymbol(HXSLSymbolRefType_AnyType, symbol) || !stream.TryGetIdentifier(identifer))
		{
			return false;
		}

		auto token = stream.Current();
		if (!token.isOperatorOf(HXSLOperator_Assign) && !token.isDelimiterOf(';'))
		{
			return false;
		}

		if (token.isOperatorOf(HXSLOperator_Assign))
		{
			stream.Advance();
		}

		auto declarationStatement = std::make_unique<HXSLDeclarationStatement>(TextSpan(), parser.parentNode(), std::move(symbol.make()), identifer, nullptr);
		if (stream.Current().isDelimiterOf('{'))
		{
			std::unique_ptr<HXSLInitializationExpression> initExpression;
			IF_ERR_RET_FALSE(ParserHelper::TryParseInitializationExpression(parser, stream, declarationStatement.get(), initExpression));
			IF_ERR_RET_FALSE(stream.ExpectDelimiter(';'));
			declarationStatement->SetInitializer(std::move(initExpression));
		}
		else if (!stream.TryGetDelimiter(';'))
		{
			std::unique_ptr<HXSLExpression> expression;
			IF_ERR_RET_FALSE(ParseExpression(parser, stream, declarationStatement.get(), expression));
			IF_ERR_RET_FALSE(stream.ExpectDelimiter(';'));
			declarationStatement->SetInitializer(std::move(expression));
		}

		declarationStatement->SetSpan(start.Span.merge(stream.LastToken().Span));
		statementOut = std::move(declarationStatement);
		return true;
	}

	bool HXSLAssignmentStatementParser::TryParse(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLStatement>& statementOut)
	{
		auto start = stream.Current();
		std::unique_ptr<HXSLExpression> target;
		if (!ParserHelper::TryParseMemberAccessPath(parser, stream, nullptr, target))
		{
			return false;
		}

		HXSLOperator op;
		if (!stream.Current().isOperator(op))
		{
			return false;
		}

		std::unique_ptr<HXSLAssignmentStatement> assignmentStatement;
		if (op == HXSLOperator_Assign)
		{
			stream.TryAdvance();
			assignmentStatement = std::make_unique<HXSLAssignmentStatement>(TextSpan(), parser.parentNode(), std::move(target), nullptr);
		}
		else if (Operators::isCompoundAssignment(op))
		{
			stream.TryAdvance();
			assignmentStatement = std::make_unique<HXSLCompoundAssignmentStatement>(TextSpan(), parser.parentNode(), op, std::move(target), nullptr);
		}
		else
		{
			return false;
		}

		std::unique_ptr<HXSLExpression> expression;

		if (stream.Current().isDelimiterOf('{'))
		{
			std::unique_ptr<HXSLInitializationExpression> initExpression;
			IF_ERR_RET_FALSE(ParserHelper::TryParseInitializationExpression(parser, stream, assignmentStatement.get(), initExpression));
			expression = std::move(initExpression);
		}
		else
		{
			IF_ERR_RET_FALSE(ParseExpression(parser, stream, assignmentStatement.get(), expression));
		}

		IF_ERR_RET_FALSE(stream.ExpectDelimiter(';'));
		assignmentStatement->SetExpression(std::move(expression));
		assignmentStatement->SetSpan(start.Span.merge(stream.LastToken().Span));
		statementOut = std::move(assignmentStatement);

		return true;
	}

	bool HXSLFunctionCallStatementParser::TryParse(HXSLParser& parser, TokenStream& stream, std::unique_ptr<HXSLStatement>& statementOut)
	{
		auto start = stream.Current();
		LazySymbol symbol;
		if (!parser.TryParseSymbol(HXSLSymbolRefType_Function, symbol) || !stream.TryGetDelimiter('('))
		{
			return false;
		}

		auto functionCallStatement = std::make_unique<HXSLFunctionCallStatement>(TextSpan(), parser.parentNode(), nullptr);
		std::unique_ptr<HXSLFunctionCallExpression> expression;
		IF_ERR_RET_FALSE(ParserHelper::ParseFunctionCallInner(start, symbol, parser, stream, nullptr, expression));
		IF_ERR_RET_FALSE(stream.ExpectDelimiter(';'));
		functionCallStatement->SetExpression(std::move(expression));
		functionCallStatement->SetSpan(start.Span.merge(stream.LastToken().Span));
		statementOut = std::move(functionCallStatement);
		return true;
	}
}