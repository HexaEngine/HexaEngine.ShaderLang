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
			parser.RejectAttribute(ATTRIBUTE_INVALID_IN_CONTEXT);
			IF_ERR_RET_FALSE(parser.inScope(ScopeFlags_InsideLoop | ScopeFlags_InsideSwitch, UNEXPECTED_BREAK_STATEMENT));
			IF_ERR_RET_FALSE(stream.ExpectDelimiter(';', EXPECTED_SEMICOLON));
			statementOut = std::make_unique<BreakStatement>(TextSpan());
			return true;
		}
		else if (stream.TryGetKeyword(Keyword_Continue))
		{
			parser.RejectAttribute(ATTRIBUTE_INVALID_IN_CONTEXT);
			IF_ERR_RET_FALSE(parser.inScope(ScopeFlags_InsideLoop, UNEXPECTED_CONTINUE_STATEMENT));
			IF_ERR_RET_FALSE(stream.ExpectDelimiter(';', EXPECTED_SEMICOLON));
			statementOut = std::make_unique<ContinueStatement>(TextSpan());
			return true;
		}
		else if (stream.TryGetKeyword(Keyword_Discard))
		{
			parser.RejectAttribute(ATTRIBUTE_INVALID_IN_CONTEXT);
			IF_ERR_RET_FALSE(parser.inScope(ScopeFlags_InsideFunction, UNEXPECTED_DISCARD_STATEMENT));
			IF_ERR_RET_FALSE(stream.ExpectDelimiter(';', EXPECTED_SEMICOLON));
			statementOut = std::make_unique<DiscardStatement>(TextSpan());
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
		parser.RejectAttribute(ATTRIBUTE_INVALID_IN_CONTEXT);
		std::unique_ptr<BlockStatement> body;
		IF_ERR_RET_FALSE(ParseStatementBody(parser.scopeType(), parser, stream, body));
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
		parser.AcceptAttribute(&attribute, 0);

		IF_ERR_RET_FALSE(stream.ExpectDelimiter('(', EXPECTED_LEFT_PAREN));

		auto forStatement = std::make_unique<ForStatement>(TextSpan());

		if (attribute && attribute->HasResource())
		{
			forStatement->AddAttribute(std::move(attribute->Take()));
		}

		std::unique_ptr<Statement> init;
		IF_ERR_RET_FALSE(StatementParserRegistry::TryParse(parser, stream, init));
		stream.LastToken().isDelimiterOf(';');
		forStatement->SetInit(std::move(init));

		std::unique_ptr<Expression> condition;
		IF_ERR_RET_FALSE(PrattParser::ParseExpression(parser, stream, condition));
		stream.ExpectDelimiter(';', EXPECTED_SEMICOLON);
		forStatement->SetCondition(std::move(condition));

		std::unique_ptr<Expression> iteration;
		IF_ERR_RET_FALSE(PrattParser::ParseExpression(parser, stream, iteration));
		IF_ERR_RET_FALSE(stream.ExpectDelimiter(')', EXPECTED_RIGHT_PAREN));
		forStatement->SetIteration(std::move(iteration));

		std::unique_ptr<BlockStatement> body; // TODO: single line statements.
		IF_ERR_RET_FALSE(ParseStatementBody(ScopeType_For, parser, stream, body));
		forStatement->SetBody(std::move(body));

		forStatement->SetSpan(stream.MakeFromLast(start));

		statementOut = std::move(forStatement);
		return true;
	}

	static bool ParseCaseInner(Parser& parser, TokenStream& stream, StatementContainer* container)
	{
		static const std::unordered_set<NodeType> breakoutTypes = { NodeType_BreakStatement, NodeType_ReturnStatement, NodeType_ContinueStatement, NodeType_DiscardStatement };

		while (true)
		{
			if (ParseStatementBodyInner(parser, stream, container, true))
			{
				HXSL_ASSERT(parser.modifierList.Empty(), "Modifier list was not empty, forgot to accept/reject it?.");
				HXSL_ASSERT(!parser.attribute.HasResource(), "Attribute list was not empty, forgot to accept/reject it?.");
			}
			else
			{
				if (!parser.TryRecoverScope(container->GetSelf(), true))
				{
					break;
				}
			}
			auto& statements = container->GetStatements();
			if (!statements.empty())
			{
				auto& last = statements.back();
				if (last->IsAnyTypeOf(breakoutTypes)) break;
			}
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
		parser.AcceptAttribute(&attribute, 0);

		IF_ERR_RET_FALSE(stream.ExpectDelimiter('(', EXPECTED_LEFT_PAREN));
		auto switchStatement = std::make_unique<SwitchStatement>(TextSpan());
		if (attribute->HasResource())
		{
			switchStatement->AddAttribute(std::move(attribute->Take()));
		}
		std::unique_ptr<Expression> expression;
		IF_ERR_RET_FALSE(PrattParser::ParseExpression(parser, stream, expression));
		IF_ERR_RET_FALSE(stream.ExpectDelimiter(')', EXPECTED_RIGHT_PAREN));
		switchStatement->SetExpression(std::move(expression));

		parser.EnterScope(ScopeType_Switch, switchStatement.get(), true);

		static const std::unordered_set<Keyword> keywords = { Keyword_Case, Keyword_Default };

		while (parser.IterateScope(switchStatement.get()))
		{
			Keyword keyword;
			auto caseStart = stream.Current();
			IF_ERR_RET_FALSE(stream.ExpectKeywords(keywords, keyword, EXPECTED_CASE_OR_DEFAULT));

			if (keyword == Keyword_Case)
			{
				auto caseStatement = std::make_unique<CaseStatement>(TextSpan(), nullptr);
				std::unique_ptr<Expression> caseExpression;
				IF_ERR_RET_FALSE(PrattParser::ParseExpression(parser, stream, caseExpression));
				caseStatement->SetExpression(std::move(caseExpression));
				IF_ERR_RET_FALSE(stream.ExpectOperator(Operator_Colon, EXPECTED_COLON));
				IF_ERR_RET_FALSE(ParseCaseInner(parser, stream, caseStatement.get()));
				caseStatement->SetSpan(stream.MakeFromLast(caseStart));
				switchStatement->AddCase(std::move(caseStatement));
			}
			else
			{
				if (switchStatement->GetDefaultCase())
				{
					ERR_RETURN_FALSE(parser, DUPLICATE_DEFAULT_CASE);
				}

				IF_ERR_RET_FALSE(stream.ExpectOperator(Operator_Colon, EXPECTED_COLON));
				auto defaultCaseStatement = std::make_unique<DefaultCaseStatement>(TextSpan());
				IF_ERR_RET_FALSE(ParseCaseInner(parser, stream, defaultCaseStatement.get()));
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
		parser.AcceptAttribute(&attribute, 0);

		IF_ERR_RET_FALSE(stream.ExpectDelimiter('(', EXPECTED_LEFT_PAREN));
		auto ifStatement = std::make_unique<IfStatement>(TextSpan(), nullptr, nullptr);
		if (attribute->HasResource())
		{
			ifStatement->AddAttribute(std::move(attribute->Take()));
		}
		std::unique_ptr<Expression> expression;
		IF_ERR_RET_FALSE(PrattParser::ParseExpression(parser, stream, expression));
		IF_ERR_RET_FALSE(stream.ExpectDelimiter(')', EXPECTED_RIGHT_PAREN));
		ifStatement->SetExpression(std::move(expression));

		std::unique_ptr<BlockStatement> statement; // TODO: single line statements.
		IF_ERR_RET_FALSE(ParseStatementBody(ScopeType_If, parser, stream, statement));
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
			parser.RejectAttribute(ATTRIBUTE_INVALID_IN_CONTEXT);
			IF_ERR_RET_FALSE(stream.ExpectDelimiter('(', EXPECTED_LEFT_PAREN));
			auto elseIfStatement = std::make_unique<ElseIfStatement>(TextSpan(), nullptr, nullptr);
			std::unique_ptr<Expression> expression;
			IF_ERR_RET_FALSE(PrattParser::ParseExpression(parser, stream, expression));
			IF_ERR_RET_FALSE(stream.ExpectDelimiter(')', EXPECTED_RIGHT_PAREN));
			elseIfStatement->SetExpression(std::move(expression));

			std::unique_ptr<BlockStatement> statement; // TODO: single line statements.
			IF_ERR_RET_FALSE(ParseStatementBody(ScopeType_ElseIf, parser, stream, statement));
			elseIfStatement->SetBody(std::move(statement));
			elseIfStatement->SetSpan(start.Span.merge(stream.LastToken().Span));
			statementOut = std::move(elseIfStatement);
		}
		else
		{
			parser.RejectAttribute(ATTRIBUTE_INVALID_IN_CONTEXT);
			auto elseStatement = std::make_unique<ElseStatement>(TextSpan(), nullptr);
			std::unique_ptr<BlockStatement> statement;
			IF_ERR_RET_FALSE(ParseStatementBody(ScopeType_Else, parser, stream, statement));
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
		parser.AcceptAttribute(&attribute, 0);

		IF_ERR_RET_FALSE(stream.ExpectDelimiter('(', EXPECTED_LEFT_PAREN));
		auto whileStatement = std::make_unique<WhileStatement>(TextSpan(), nullptr, nullptr);

		if (attribute->HasResource())
		{
			whileStatement->AddAttribute(attribute->Take());
		}

		std::unique_ptr<Expression> expression;
		IF_ERR_RET_FALSE(PrattParser::ParseExpression(parser, stream, expression));
		IF_ERR_RET_FALSE(stream.ExpectDelimiter(')', EXPECTED_RIGHT_PAREN));
		whileStatement->SetExpression(std::move(expression));

		std::unique_ptr<BlockStatement> statement; // TODO: single line statements.
		IF_ERR_RET_FALSE(ParseStatementBody(ScopeType_While, parser, stream, statement));
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
		parser.RejectAttribute(ATTRIBUTE_INVALID_IN_CONTEXT);
		auto returnStatement = std::make_unique<ReturnStatement>(TextSpan(), nullptr);
		std::unique_ptr<Expression> expression;
		IF_ERR_RET_FALSE(PrattParser::ParseExpression(parser, stream, expression));
		stream.ExpectDelimiter(';', EXPECTED_SEMICOLON);
		returnStatement->SetReturnValueExpression(std::move(expression));
		returnStatement->SetSpan(start.Span.merge(stream.LastToken().Span));
		statementOut = std::move(returnStatement);
		return true;
	}

	static bool ParseAssignment(const Token& start, std::unique_ptr<Expression> target, Parser& parser, TokenStream& stream, std::unique_ptr<Statement>& statementOut)
	{
		parser.RejectModifierList(NO_MODIFIER_INVALID_IN_CONTEXT, true);
		parser.RejectAttribute(ATTRIBUTE_INVALID_IN_CONTEXT);

		auto current = stream.Current();
		Operator op;
		if (!current.isAssignment(op))
		{
			parser.Log(EXPECTED_ASSIGNMENT_OP, current);
		}

		std::unique_ptr<AssignmentStatement> assignmentStatement;
		if (op == Operator_Assign)
		{
			stream.TryAdvance();
			assignmentStatement = std::make_unique<AssignmentStatement>(TextSpan(), std::move(target), nullptr);
		}
		else
		{
			stream.TryAdvance();
			assignmentStatement = std::make_unique<CompoundAssignmentStatement>(TextSpan(), op, std::move(target), nullptr);
		}

		std::unique_ptr<Expression> expression;

		if (stream.Current().isDelimiterOf('{'))
		{
			std::unique_ptr<InitializationExpression> initExpression;
			IF_ERR_RET_FALSE(ParserHelper::TryParseInitializationExpression(parser, stream, initExpression));
			expression = std::move(initExpression);
		}
		else
		{
			if (!PrattParser::ParseExpression(parser, stream, expression))
			{
				parser.TryRecoverStatement();
			}
		}

		stream.ExpectDelimiter(';', EXPECTED_SEMICOLON);
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
			parser.Log(EXPECTED_TYPE_EXPR, target->GetSpan());
		}
		ModifierList list;
		ModifierList allowed = ModifierList(AccessModifier_None, false, FunctionFlags_None, StorageClass_Const | StorageClass_Precise, InterpolationModifier_None, false);
		parser.AcceptModifierList(&list, allowed, INVALID_MODIFIER_ON_VAR);
		parser.RejectAttribute(ATTRIBUTE_INVALID_IN_CONTEXT);

		TextSpan identifer;
		stream.ExpectIdentifier(identifer, EXPECTED_IDENTIFIER);

		std::vector<size_t> arraySizes;
		if (parser.TryParseArraySizes(arraySizes))
		{
			symbol->SetArrayDims(arraySizes);
		}

		auto token = stream.Current();
		if (!token.isOperatorOf(Operator_Assign) && !token.isDelimiterOf(';'))
		{
			parser.Log(EXPECTED_EQUALS_OR_SEMICOLON_DECL, token);
			return false;
		}

		if (token.isOperatorOf(Operator_Assign))
		{
			stream.Advance();
		}

		auto declarationStatement = std::make_unique<DeclarationStatement>(TextSpan(), std::move(symbol), list.storageClasses, identifer, nullptr);
		if (stream.Current().isDelimiterOf('{'))
		{
			std::unique_ptr<InitializationExpression> initExpression;
			IF_ERR_RET_FALSE(ParserHelper::TryParseInitializationExpression(parser, stream, initExpression));
			IF_ERR_RET_FALSE(stream.ExpectDelimiter(';', EXPECTED_SEMICOLON));
			declarationStatement->SetInitializer(std::move(initExpression));
		}
		else if (!stream.TryGetDelimiter(';'))
		{
			std::unique_ptr<Expression> expression;
			if (!PrattParser::ParseExpression(parser, stream, expression))
			{
				parser.TryRecoverStatement();
			}
			stream.ExpectDelimiter(';', EXPECTED_SEMICOLON);
			declarationStatement->SetInitializer(std::move(expression));
		}

		declarationStatement->SetSpan(start.Span.merge(stream.LastToken().Span));
		statementOut = std::move(declarationStatement);
		return true;
	}

	static bool ParseFunctionCall(const Token& start, std::unique_ptr<Expression> target, Parser& parser, TokenStream& stream, std::unique_ptr<Statement>& statementOut)
	{
		parser.RejectModifierList(NO_MODIFIER_INVALID_IN_CONTEXT);
		parser.RejectAttribute(ATTRIBUTE_INVALID_IN_CONTEXT);

		Expression* end = target.get();
		while (auto getter = dynamic_cast<ChainExpression*>(end))
		{
			end = getter->GetNextExpression().get();
		}

		if (end == nullptr || end->GetType() != NodeType_FunctionCallExpression)
		{
			parser.Log(EXPECTED_FUNC_CALL_EXPR, end ? end->GetSpan() : target->GetSpan());
			return false;
		}

		auto functionCallStatement = std::make_unique<FunctionCallStatement>(TextSpan(), std::move(target));
		stream.ExpectDelimiter(';', EXPECTED_SEMICOLON);
		functionCallStatement->SetSpan(start.Span.merge(stream.LastToken().Span));
		statementOut = std::move(functionCallStatement);
		return true;
	}

	bool MemberAccessStatementParser::TryParse(Parser& parser, TokenStream& stream, std::unique_ptr<Statement>& statementOut)
	{
		auto start = stream.Current();
		std::unique_ptr<Expression> target;
		if (!ParserHelper::TryParseMemberAccessPath(parser, stream, target))
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