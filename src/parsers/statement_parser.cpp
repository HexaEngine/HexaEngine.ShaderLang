#include "statement_parser.hpp"
#include "sub_parser_registry.hpp"
#include "expression_parser.hpp"
#include "parser_helper.hpp"
#include "pratt_parser.hpp"

#include <memory>
#include <optional>
namespace HXSL
{
	bool MiscKeywordStatementParser::TryParse(Parser& parser, TokenStream& stream, std::unique_ptr<Statement>& statementOut)
	{
		auto start = stream.Current();
		if (stream.TryGetKeyword(Keyword_Break))
		{
			parser.RejectAttribute("is not allowed in this context.");
			IF_ERR_RET_FALSE(parser.inScope(ScopeFlags_InsideLoop | ScopeFlags_InsideSwitch, "'break' statement used outside of a loop or switch context."));
			IF_ERR_RET_FALSE(stream.ExpectDelimiter(';'));
			statementOut = std::make_unique<BreakStatement>(TextSpan(), parser.parentNode());
			return true;
		}
		else if (stream.TryGetKeyword(Keyword_Continue))
		{
			parser.RejectAttribute("is not allowed in this context.");
			IF_ERR_RET_FALSE(parser.inScope(ScopeFlags_InsideLoop, "'continue' statement used outside of a loop context."));
			IF_ERR_RET_FALSE(stream.ExpectDelimiter(';'));
			statementOut = std::make_unique<ContinueStatement>(TextSpan(), parser.parentNode());
			return true;
		}
		else if (stream.TryGetKeyword(Keyword_Discard))
		{
			parser.RejectAttribute("is not allowed in this context.");
			IF_ERR_RET_FALSE(parser.inScope(ScopeFlags_InsideFunction));
			IF_ERR_RET_FALSE(stream.ExpectDelimiter(';'));
			statementOut = std::make_unique<DiscardStatement>(TextSpan(), parser.parentNode());
			return true;
		}
		return false;
	}

	bool BlockStatementParser::TryParse(Parser& parser, TokenStream& stream, std::unique_ptr<Statement>& statementOut)
	{
		auto start = stream.Current();
		if (!start.isDelimiterOf('{'))
		{
			return false;
		}
		parser.RejectAttribute("is not allowed in this context.");
		std::unique_ptr<BlockStatement> body;
		IF_ERR_RET_FALSE(ParseStatementBody(TextSpan(), parser.scopeType(), parser.parentNode(), parser, stream, body));
		statementOut = std::move(body);
		return true;
	}

	bool ForStatementParser::TryParse(Parser& parser, TokenStream& stream, std::unique_ptr<Statement>& statementOut)
	{
		auto start = stream.Current();

		if (!stream.TryGetKeyword(Keyword_For))
		{
			return false;
		}

		TakeHandle<AttributeDeclaration>* attribute = nullptr;
		parser.AcceptAttribute(&attribute, "");

		IF_ERR_RET_FALSE(stream.ExpectDelimiter('('));

		auto forStatement = std::make_unique<ForStatement>(TextSpan(), parser.parentNode());

		if (attribute && attribute->HasResource())
		{
			forStatement->AddAttribute(std::move(attribute->Take()));
		}

		std::unique_ptr<Statement> init;
		IF_ERR_RET_FALSE(StatementParserRegistry::TryParse(parser, stream, forStatement.get(), init));
		stream.LastToken().isDelimiterOf(';');
		forStatement->SetInit(std::move(init));

		std::unique_ptr<Expression> condition;
		IF_ERR_RET_FALSE(ParseExpression(parser, stream, forStatement.get(), condition));
		stream.ExpectDelimiter(';');
		forStatement->SetCondition(std::move(condition));

		std::unique_ptr<Expression> iteration;
		IF_ERR_RET_FALSE(ParseExpression(parser, stream, forStatement.get(), iteration));
		IF_ERR_RET_FALSE(stream.ExpectDelimiter(')'));
		forStatement->SetIteration(std::move(iteration));

		std::unique_ptr<BlockStatement> body; // TODO: single line statements.
		IF_ERR_RET_FALSE(ParseStatementBody(TextSpan(), ScopeType_For, forStatement.get(), parser, stream, body));
		forStatement->SetBody(std::move(body));

		forStatement->SetSpan(stream.MakeFromLast(start));

		statementOut = std::move(forStatement);
		return true;
	}

	static bool ParseCaseInner(Parser& parser, TokenStream& stream, ASTNode* parent, StatementContainer* container)
	{
		static const std::unordered_set<char> unexpectedDelimiters = { '}', ')', '.', ':' };
		static const std::unordered_set<NodeType> breakoutTypes = { NodeType_BreakStatement, NodeType_ReturnStatement, NodeType_ContinueStatement, NodeType_DiscardStatement };

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

	bool SwitchStatementParser::TryParse(Parser& parser, TokenStream& stream, std::unique_ptr<Statement>& statementOut)
	{
		auto start = stream.Current();

		if (!stream.TryGetKeyword(Keyword_Switch))
		{
			return false;
		}

		TakeHandle<AttributeDeclaration>* attribute;
		parser.AcceptAttribute(&attribute, "");

		IF_ERR_RET_FALSE(stream.ExpectDelimiter('('));
		auto switchStatement = std::make_unique<SwitchStatement>(TextSpan(), parser.parentNode());
		if (attribute->HasResource())
		{
			switchStatement->AddAttribute(std::move(attribute->Take()));
		}
		std::unique_ptr<Expression> expression;
		IF_ERR_RET_FALSE(ParseExpression(parser, stream, switchStatement.get(), expression));
		IF_ERR_RET_FALSE(stream.ExpectDelimiter(')'));
		switchStatement->SetExpression(std::move(expression));

		parser.EnterScope(TextSpan(), ScopeType_Switch, switchStatement.get(), true);

		static const std::unordered_set<Keyword> keywords = { Keyword_Case, Keyword_Default };

		while (parser.IterateScope())
		{
			Keyword keyword;
			auto caseStart = stream.Current();
			IF_ERR_RET_FALSE(stream.ExpectKeywords(keywords, keyword));

			if (keyword == Keyword_Case)
			{
				auto caseStatement = std::make_unique<CaseStatement>(TextSpan(), switchStatement.get(), nullptr);
				std::unique_ptr<Expression> caseExpression;
				IF_ERR_RET_FALSE(ParseExpression(parser, stream, caseStatement.get(), caseExpression));
				caseStatement->SetExpression(std::move(caseExpression));
				IF_ERR_RET_FALSE(stream.ExpectOperator(Operator_Colon));
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

				IF_ERR_RET_FALSE(stream.ExpectOperator(Operator_Colon));
				auto defaultCaseStatement = std::make_unique<DefaultCaseStatement>(TextSpan(), switchStatement.get());
				IF_ERR_RET_FALSE(ParseCaseInner(parser, stream, defaultCaseStatement.get(), defaultCaseStatement.get()));
				defaultCaseStatement->SetSpan(stream.MakeFromLast(caseStart));
				switchStatement->SetDefaultCase(std::move(defaultCaseStatement));
			}
		}

		switchStatement->SetSpan(stream.MakeFromLast(start));
		statementOut = std::move(switchStatement);
		return true;
	}

	bool IfStatementParser::TryParse(Parser& parser, TokenStream& stream, std::unique_ptr<Statement>& statementOut)
	{
		auto start = stream.Current();

		if (!stream.TryGetKeyword(Keyword_If))
		{
			return false;
		}

		TakeHandle<AttributeDeclaration>* attribute;
		parser.AcceptAttribute(&attribute, "");

		IF_ERR_RET_FALSE(stream.ExpectDelimiter('('));
		auto ifStatement = std::make_unique<IfStatement>(TextSpan(), parser.parentNode(), nullptr, nullptr);
		if (attribute->HasResource())
		{
			ifStatement->AddAttribute(std::move(attribute->Take()));
		}
		std::unique_ptr<Expression> expression;
		IF_ERR_RET_FALSE(ParseExpression(parser, stream, ifStatement.get(), expression));
		IF_ERR_RET_FALSE(stream.ExpectDelimiter(')'));
		ifStatement->SetExpression(std::move(expression));

		std::unique_ptr<BlockStatement> statement; // TODO: single line statements.
		IF_ERR_RET_FALSE(ParseStatementBody(TextSpan(), ScopeType_If, ifStatement.get(), parser, stream, statement));
		ifStatement->SetBody(std::move(statement));
		ifStatement->SetSpan(start.Span.merge(stream.LastToken().Span));
		statementOut = std::move(ifStatement);
		return true;
	}

	bool ElseStatementParser::TryParse(Parser& parser, TokenStream& stream, std::unique_ptr<Statement>& statementOut)
	{
		auto start = stream.Current();
		if (!stream.TryGetKeyword(Keyword_Else))
		{
			return false;
		}

		if (stream.TryGetKeyword(Keyword_If))
		{
			parser.RejectAttribute("is not allowed in this context.");
			IF_ERR_RET_FALSE(stream.ExpectDelimiter('('));
			auto elseIfStatement = std::make_unique<ElseIfStatement>(TextSpan(), parser.parentNode(), nullptr, nullptr);
			std::unique_ptr<Expression> expression;
			IF_ERR_RET_FALSE(ParseExpression(parser, stream, elseIfStatement.get(), expression));
			IF_ERR_RET_FALSE(stream.ExpectDelimiter(')'));
			elseIfStatement->SetExpression(std::move(expression));

			std::unique_ptr<BlockStatement> statement; // TODO: single line statements.
			IF_ERR_RET_FALSE(ParseStatementBody(TextSpan(), ScopeType_ElseIf, elseIfStatement.get(), parser, stream, statement));
			elseIfStatement->SetBody(std::move(statement));
			elseIfStatement->SetSpan(start.Span.merge(stream.LastToken().Span));
			statementOut = std::move(elseIfStatement);
		}
		else
		{
			parser.RejectAttribute("is not allowed in this context.");
			auto elseStatement = std::make_unique<ElseStatement>(TextSpan(), parser.parentNode(), nullptr);
			std::unique_ptr<BlockStatement> statement;
			IF_ERR_RET_FALSE(ParseStatementBody(TextSpan(), ScopeType_Else, elseStatement.get(), parser, stream, statement));
			elseStatement->SetBody(std::move(statement));
			elseStatement->SetSpan(start.Span.merge(stream.LastToken().Span));
			statementOut = std::move(elseStatement);
		}

		return true;
	}

	bool WhileStatementParser::TryParse(Parser& parser, TokenStream& stream, std::unique_ptr<Statement>& statementOut)
	{
		auto start = stream.Current();
		if (!stream.TryGetKeyword(Keyword_While))
		{
			return false;
		}

		TakeHandle<AttributeDeclaration>* attribute;
		parser.AcceptAttribute(&attribute, "");

		IF_ERR_RET_FALSE(stream.ExpectDelimiter('('));
		auto whileStatement = std::make_unique<WhileStatement>(TextSpan(), parser.parentNode(), nullptr, nullptr);

		if (attribute->HasResource())
		{
			whileStatement->AddAttribute(attribute->Take());
		}

		std::unique_ptr<Expression> expression;
		IF_ERR_RET_FALSE(ParseExpression(parser, stream, whileStatement.get(), expression));
		IF_ERR_RET_FALSE(stream.ExpectDelimiter(')'));
		whileStatement->SetExpression(std::move(expression));

		std::unique_ptr<BlockStatement> statement; // TODO: single line statements.
		IF_ERR_RET_FALSE(ParseStatementBody(TextSpan(), ScopeType_While, whileStatement.get(), parser, stream, statement));
		whileStatement->SetBody(std::move(statement));
		whileStatement->SetSpan(start.Span.merge(stream.LastToken().Span));
		statementOut = std::move(whileStatement);
		return true;
	}

	bool ReturnStatementParser::TryParse(Parser& parser, TokenStream& stream, std::unique_ptr<Statement>& statementOut)
	{
		auto start = stream.Current();
		if (!stream.TryGetKeyword(Keyword_Return))
		{
			return false;
		}
		parser.RejectAttribute("is not allowed in this context.");
		auto returnStatement = std::make_unique<ReturnStatement>(TextSpan(), parser.parentNode(), nullptr);
		std::unique_ptr<Expression> expression;
		IF_ERR_RET_FALSE(ParseExpression(parser, stream, returnStatement.get(), expression));
		stream.ExpectDelimiter(';');
		returnStatement->SetReturnValueExpression(std::move(expression));
		returnStatement->SetSpan(start.Span.merge(stream.LastToken().Span));
		statementOut = std::move(returnStatement);
		return true;
	}

	static bool ParseAssignment(const Token& start, std::unique_ptr<Expression> target, Parser& parser, TokenStream& stream, std::unique_ptr<Statement>& statementOut)
	{
		parser.RejectModifierList("No modifiers are allowed in this context", true);
		parser.RejectAttribute("is not allowed in this context.");

		auto current = stream.Current();
		Operator op;
		if (!current.isAssignment(op))
		{
			parser.LogError("Expected an assignment operator.", current);
		}

		std::unique_ptr<AssignmentStatement> assignmentStatement;
		if (op == Operator_Assign)
		{
			stream.TryAdvance();
			assignmentStatement = std::make_unique<AssignmentStatement>(TextSpan(), parser.parentNode(), std::move(target), nullptr);
		}
		else
		{
			stream.TryAdvance();
			assignmentStatement = std::make_unique<CompoundAssignmentStatement>(TextSpan(), parser.parentNode(), op, std::move(target), nullptr);
		}

		std::unique_ptr<Expression> expression;

		if (stream.Current().isDelimiterOf('{'))
		{
			std::unique_ptr<InitializationExpression> initExpression;
			IF_ERR_RET_FALSE(ParserHelper::TryParseInitializationExpression(parser, stream, assignmentStatement.get(), initExpression));
			expression = std::move(initExpression);
		}
		else
		{
			IF_ERR_RET_FALSE(ParseExpression(parser, stream, assignmentStatement.get(), expression));
		}

		stream.ExpectDelimiter(';');
		assignmentStatement->SetExpression(std::move(expression));
		assignmentStatement->SetSpan(start.Span.merge(stream.LastToken().Span));
		statementOut = std::move(assignmentStatement);

		return true;
	}

	static bool ParseDeclaration(const Token& start, std::unique_ptr<Expression> target, Parser& parser, TokenStream& stream, std::unique_ptr<Statement>& statementOut)
	{
		std::unique_ptr<SymbolRef> symbol;
		if (!ParserHelper::MakeConcreteSymbolRef(target.get(), SymbolRefType_Type, symbol))
		{
			parser.LogError("Expected type expression.", target->GetSpan());
		}
		ModifierList list;
		ModifierList allowed = ModifierList(AccessModifier_None, false, FunctionFlags_None, StorageClass_Const | StorageClass_Precise, InterpolationModifier_None, false);
		parser.AcceptModifierList(&list, allowed, "Invalid StorageClass flags. Only 'const' and 'precise' are allowed on local variables.", true);
		parser.RejectAttribute("is not allowed in this context.");

		TextSpan identifer;
		stream.ExpectIdentifier(identifer);

		std::vector<size_t> arraySizes;
		if (parser.TryParseArraySizes(arraySizes))
		{
			symbol->SetArrayDims(arraySizes);
		}

		auto token = stream.Current();
		if (!token.isOperatorOf(Operator_Assign) && !token.isDelimiterOf(';'))
		{
			parser.LogError("Expected a '=' or ';' in a declaration expression.", token);
			return false;
		}

		if (token.isOperatorOf(Operator_Assign))
		{
			stream.Advance();
		}

		auto declarationStatement = std::make_unique<DeclarationStatement>(TextSpan(), parser.parentNode(), std::move(symbol), list.storageClasses, identifer, nullptr);
		if (stream.Current().isDelimiterOf('{'))
		{
			std::unique_ptr<InitializationExpression> initExpression;
			IF_ERR_RET_FALSE(ParserHelper::TryParseInitializationExpression(parser, stream, declarationStatement.get(), initExpression));
			IF_ERR_RET_FALSE(stream.ExpectDelimiter(';'));
			declarationStatement->SetInitializer(std::move(initExpression));
		}
		else if (!stream.TryGetDelimiter(';'))
		{
			std::unique_ptr<Expression> expression;
			IF_ERR_RET_FALSE(ParseExpression(parser, stream, declarationStatement.get(), expression));
			stream.ExpectDelimiter(';');
			declarationStatement->SetInitializer(std::move(expression));
		}

		declarationStatement->SetSpan(start.Span.merge(stream.LastToken().Span));
		statementOut = std::move(declarationStatement);
		return true;
	}

	static bool ParseFunctionCall(const Token& start, std::unique_ptr<Expression> target, Parser& parser, TokenStream& stream, std::unique_ptr<Statement>& statementOut)
	{
		parser.RejectModifierList("No modifiers are allowed in this context", true);
		parser.RejectAttribute("is not allowed in this context.");

		Expression* end = target.get();
		while (auto getter = dynamic_cast<IChainExpression*>(end))
		{
			end = getter->chainNext().get();
		}

		if (end == nullptr || end->GetType() != NodeType_FunctionCallExpression)
		{
			if (end == nullptr)
			{
				parser.LogError("Expected a function call expression, but found an invalid expression.", target->GetSpan());
			}
			else
			{
				parser.LogError("Expected a function call expression, but got '%s' instead.", end->GetSpan(), ToString(end->GetType()));
			}
			return false;
		}

		auto functionCallStatement = std::make_unique<FunctionCallStatement>(TextSpan(), parser.parentNode(), std::move(target));
		stream.ExpectDelimiter(';');
		functionCallStatement->SetSpan(start.Span.merge(stream.LastToken().Span));
		statementOut = std::move(functionCallStatement);
		return true;
	}

	bool MemberAccessStatementParser::TryParse(Parser& parser, TokenStream& stream, std::unique_ptr<Statement>& statementOut)
	{
		auto start = stream.Current();
		std::unique_ptr<Expression> target;
		if (!ParserHelper::TryParseMemberAccessPath(parser, stream, nullptr, target))
		{
			return false;
		}

		auto current = stream.Current();

		if (current.isOperator())
		{
			return ParseAssignment(start, std::move(target), parser, stream, statementOut);
		}
		else if (current.isIdentifier())
		{
			return ParseDeclaration(start, std::move(target), parser, stream, statementOut);
		}
		else
		{
			return ParseFunctionCall(start, std::move(target), parser, stream, statementOut);
		}
	}
}