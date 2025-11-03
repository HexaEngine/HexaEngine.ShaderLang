#include "statement_parser.hpp"
#include "sub_parser_registry.hpp"
#include "expression_parser.hpp"
#include "parser_helper.hpp"
#include "hybrid_expr_parser.hpp"

namespace HXSL
{
	bool MiscKeywordStatementParser::TryParse(Parser& parser, TokenStream& stream, ASTNode*& statementOut)
	{
		auto start = stream.Current();
		if (stream.TryGetKeyword(Keyword_Break))
		{
			parser.RejectAttribute(ATTRIBUTE_INVALID_IN_CONTEXT);
			IF_ERR_RET_FALSE(parser.inScope(ScopeFlags_InsideLoop | ScopeFlags_InsideSwitch, UNEXPECTED_BREAK_STATEMENT));
			IF_ERR_RET_FALSE(stream.ExpectDelimiter(';', EXPECTED_SEMICOLON));
			statementOut = BreakStatement::Create(start.Span);
			return true;
		}
		else if (stream.TryGetKeyword(Keyword_Continue))
		{
			parser.RejectAttribute(ATTRIBUTE_INVALID_IN_CONTEXT);
			IF_ERR_RET_FALSE(parser.inScope(ScopeFlags_InsideLoop, UNEXPECTED_CONTINUE_STATEMENT));
			IF_ERR_RET_FALSE(stream.ExpectDelimiter(';', EXPECTED_SEMICOLON));
			statementOut = ContinueStatement::Create(start.Span);
			return true;
		}
		else if (stream.TryGetKeyword(Keyword_Discard))
		{
			parser.RejectAttribute(ATTRIBUTE_INVALID_IN_CONTEXT);
			IF_ERR_RET_FALSE(parser.inScope(ScopeFlags_InsideFunction, UNEXPECTED_DISCARD_STATEMENT));
			IF_ERR_RET_FALSE(stream.ExpectDelimiter(';', EXPECTED_SEMICOLON));
			statementOut = DiscardStatement::Create(start.Span);
			return true;
		}
		return false;
	}

	bool BlockStatementParser::TryParse(Parser& parser, TokenStream& stream, ASTNode*& statementOut)
	{
		auto start = stream.Current();
		if (!start.isDelimiterOf('{'))
		{
			return false;
		}
		parser.RejectAttribute(ATTRIBUTE_INVALID_IN_CONTEXT);
		BlockStatement* body;
		IF_ERR_RET_FALSE(ParseStatementBody(parser.scopeType(), parser, stream, body));
		statementOut = std::move(body);
		return true;
	}

	bool ForStatementParser::TryParse(Parser& parser, TokenStream& stream, ASTNode*& statementOut)
	{
		auto start = stream.Current();

		if (!stream.TryGetKeyword(Keyword_For))
		{
			return false;
		}

		TakeHandle<AttributeDecl>* attribute = nullptr;
		parser.AcceptAttribute(&attribute, 0);

		IF_ERR_RET_FALSE(stream.ExpectDelimiter('(', EXPECTED_LEFT_PAREN));

		std::vector<AttributeDecl*> attributes;
		if (attribute && attribute->HasResource())
		{
			attributes.push_back(attribute->Take());
		}

		ASTNode* init;
		StatementParserRegistry::TryParse(parser, stream, init);
		stream.LastToken().isDelimiterOf(';');

		Expression* condition;
		HybridExpressionParser::ParseExpression(parser, stream, condition);
		stream.ExpectDelimiter(';', EXPECTED_SEMICOLON);

		Expression* iteration;
		HybridExpressionParser::ParseExpression(parser, stream, iteration);
		stream.ExpectDelimiter(')', EXPECTED_RIGHT_PAREN);

		BlockStatement* body; // TODO: single line statements.
		ParseStatementBody(ScopeType_For, parser, stream, body);

		auto span = stream.MakeFromLast(start);
		auto forStatement = ForStatement::Create(span, init, condition, iteration, body, attributes);
		statementOut = forStatement;
		return true;
	}

	static bool ParseCaseInner(Parser& parser, TokenStream& stream, std::vector<ASTNode*>& statements)
	{
		static const std::unordered_set<NodeType> breakoutTypes = { NodeType_BreakStatement, NodeType_ReturnStatement, NodeType_ContinueStatement, NodeType_DiscardStatement };

		while (true)
		{
			if (ParseStatementBodyInner(parser, stream, statements, true))
			{
				HXSL_ASSERT(parser.modifierList.Empty(), "Modifier list was not empty, forgot to accept/reject it?.");
				HXSL_ASSERT(!parser.attribute.HasResource(), "Attribute list was not empty, forgot to accept/reject it?.");
			}
			else
			{
				if (!parser.TryRecoverScope(nullptr, true))
				{
					break;
				}
			}
			if (!statements.empty())
			{
				auto& last = statements.back();
				if (last->IsAnyTypeOf(breakoutTypes)) break;
			}
		}

		return true;
	}

	bool SwitchStatementParser::TryParse(Parser& parser, TokenStream& stream, ASTNode*& statementOut)
	{
		auto start = stream.Current();

		if (!stream.TryGetKeyword(Keyword_Switch))
		{
			return false;
		}

		TakeHandle<AttributeDecl>* attribute = nullptr;
		parser.AcceptAttribute(&attribute, 0);

		stream.ExpectDelimiter('(', EXPECTED_LEFT_PAREN);

		std::vector<AttributeDecl*> attributes;
		if (attribute != nullptr && attribute->HasResource())
		{
			attributes.push_back(attribute->Take());
		}
		Expression* expression;
		IF_ERR_RET_FALSE(HybridExpressionParser::ParseExpression(parser, stream, expression));
		IF_ERR_RET_FALSE(stream.ExpectDelimiter(')', EXPECTED_RIGHT_PAREN));

		parser.EnterScope(ScopeType_Switch, nullptr, true);

		static const std::unordered_set<Keyword> keywords = { Keyword_Case, Keyword_Default };

		std::vector<CaseStatement*> cases;
		DefaultCaseStatement* defaultCase = nullptr;
		while (parser.IterateScope(nullptr))
		{
			Keyword keyword;
			auto caseStart = stream.Current();
			stream.ExpectKeywords(keywords, keyword, EXPECTED_CASE_OR_DEFAULT);

			if (keyword == Keyword_Case)
			{
				Expression* caseExpression;
				HybridExpressionParser::ParseExpression(parser, stream, caseExpression, ExpressionParserFlags_SwitchCase);

				stream.ExpectOperator(Operator_Colon, EXPECTED_COLON);
				std::vector<ASTNode*> statements;
				ParseCaseInner(parser, stream, statements);
				auto span = stream.MakeFromLast(caseStart);
				auto caseStatement = CaseStatement::Create(span, caseExpression, statements);
				cases.push_back(caseStatement);
			}
			else
			{
				if (defaultCase)
				{
					parser.Log(DUPLICATE_DEFAULT_CASE, caseStart);
				}

				stream.ExpectOperator(Operator_Colon, EXPECTED_COLON);
				
				std::vector<ASTNode*> statements;
				ParseCaseInner(parser, stream, statements);

				auto span = stream.MakeFromLast(caseStart);
				defaultCase = DefaultCaseStatement::Create(span, statements);
			}
		}

		auto span = stream.MakeFromLast(start);
		auto switchStatement = SwitchStatement::Create(span, expression, cases, defaultCase, attributes);
		statementOut = std::move(switchStatement);
		return true;
	}

	static bool TryParseElseStatement(Parser& parser, TokenStream& stream, std::vector<ElseIfStatement*>& elseIfStatements, ElseStatement*& elseStatement)
	{
		auto start = stream.Current();
		if (!stream.TryGetKeyword(Keyword_Else))
		{
			return false;
		}

		elseStatement = nullptr;
		if (stream.TryGetKeyword(Keyword_If))
		{
			parser.RejectAttribute(ATTRIBUTE_INVALID_IN_CONTEXT);

			

			Expression* expression;
			stream.ExpectDelimiter('(', EXPECTED_LEFT_PAREN);
			HybridExpressionParser::ParseExpression(parser, stream, expression);
			stream.ExpectDelimiter(')', EXPECTED_RIGHT_PAREN);

			BlockStatement* statement;
			ParseStatementBody(ScopeType_ElseIf, parser, stream, statement);

			auto span = stream.MakeFromLast(start);
			auto elseIfStatement = ElseIfStatement::Create(span, expression, statement);
			elseIfStatements.push_back(elseIfStatement);
		}
		else
		{
			parser.RejectAttribute(ATTRIBUTE_INVALID_IN_CONTEXT);
			
			BlockStatement* statement;
			ParseStatementBody(ScopeType_Else, parser, stream, statement);

			auto span = stream.MakeFromLast(start);
			elseStatement = ElseStatement::Create(span, statement);
		}

		return true;
	}

	bool IfStatementParser::TryParse(Parser& parser, TokenStream& stream, ASTNode*& statementOut)
	{
		auto start = stream.Current();

		if (!stream.TryGetKeyword(Keyword_If))
		{
			return false;
		}

		TakeHandle<AttributeDecl>* attribute = nullptr;
		parser.AcceptAttribute(&attribute, 0);

		IF_ERR_RET_FALSE(stream.ExpectDelimiter('(', EXPECTED_LEFT_PAREN));
		
		std::vector<AttributeDecl*> attributes;
		if (attribute != nullptr && attribute->HasResource())
		{
			attributes.push_back(attribute->Take());
		}
		Expression* expression;
		IF_ERR_RET_FALSE(HybridExpressionParser::ParseExpression(parser, stream, expression));
		IF_ERR_RET_FALSE(stream.ExpectDelimiter(')', EXPECTED_RIGHT_PAREN));

		BlockStatement* statement; // TODO: single line statements.
		IF_ERR_RET_FALSE(ParseStatementBody(ScopeType_If, parser, stream, statement));

		std::vector<ElseIfStatement*> elseIfStatements;
		ElseStatement* elseStatement = nullptr;
		while (stream.CanAdvance() && TryParseElseStatement(parser, stream, elseIfStatements, elseStatement) && elseStatement == nullptr) {}

		auto span = stream.MakeFromLast(start);
		auto ifStatement = IfStatement::Create(span, expression, statement, elseIfStatements, elseStatement, attributes);
		statementOut = std::move(ifStatement);
		return true;
	}

	bool WhileStatementParser::TryParse(Parser& parser, TokenStream& stream, ASTNode*& statementOut)
	{
		auto start = stream.Current();
		if (!stream.TryGetKeyword(Keyword_While))
		{
			return false;
		}

		TakeHandle<AttributeDecl>* attribute = nullptr;
		parser.AcceptAttribute(&attribute, 0);

		stream.ExpectDelimiter('(', EXPECTED_LEFT_PAREN);

		std::vector<AttributeDecl*> attributes;
		if (attribute != nullptr && attribute->HasResource())
		{
			attributes.push_back(attribute->Take());
		}

		Expression* expression;
		IF_ERR_RET_FALSE(HybridExpressionParser::ParseExpression(parser, stream, expression));
		stream.ExpectDelimiter(')', EXPECTED_RIGHT_PAREN);

		BlockStatement* statement;
		IF_ERR_RET_FALSE(ParseStatementBody(ScopeType_While, parser, stream, statement));

		auto span = stream.MakeFromLast(start);
		auto whileStatement = WhileStatement::Create(span, expression, statement, attributes);
		statementOut = whileStatement;
		return true;
	}

	bool DoWhileStatementParser::TryParse(Parser& parser, TokenStream& stream, ASTNode*& statementOut)
	{
		auto start = stream.Current();
		if (!stream.TryGetKeyword(Keyword_Do))
		{
			return false;
		}

		TakeHandle<AttributeDecl>* attribute = nullptr;
		parser.AcceptAttribute(&attribute, 0);

		std::vector<AttributeDecl*> attributes;
		if (attribute != nullptr && attribute->HasResource())
		{
			attributes.push_back(attribute->Take());
		}

		BlockStatement* statement;
		IF_ERR_RET_FALSE(ParseStatementBody(ScopeType_While, parser, stream, statement));
		stream.ExpectKeyword(Keyword_While, EXPECTED_WHILE);

		stream.ExpectDelimiter('(', EXPECTED_LEFT_PAREN);
		Expression* expression;
		IF_ERR_RET_FALSE(HybridExpressionParser::ParseExpression(parser, stream, expression));
		stream.ExpectDelimiter(')', EXPECTED_RIGHT_PAREN);

		auto span = stream.MakeFromLast(start);
		auto doWhileStatement = DoWhileStatement::Create(span, expression, statement, attributes);
		statementOut = doWhileStatement;

		return true;
	}

	bool ReturnStatementParser::TryParse(Parser& parser, TokenStream& stream, ASTNode*& statementOut)
	{
		auto start = stream.Current();
		if (!stream.TryGetKeyword(Keyword_Return))
		{
			return false;
		}
		parser.RejectAttribute(ATTRIBUTE_INVALID_IN_CONTEXT);
		auto returnStatement = ReturnStatement::Create(TextSpan(), nullptr);
		if (!stream.Current().isDelimiterOf(';'))
		{
			Expression* expression;
			IF_ERR_RET_FALSE(HybridExpressionParser::ParseExpression(parser, stream, expression));
			returnStatement->SetReturnValueExpression(std::move(expression));
		}

		stream.ExpectDelimiter(';', EXPECTED_SEMICOLON);

		returnStatement->SetSpan(start.Span.merge(stream.LastToken().Span));
		statementOut = std::move(returnStatement);
		return true;
	}

	static bool ParseAssignment(const Token& start, Expression* target, Parser& parser, TokenStream& stream, ASTNode*& statementOut)
	{
		parser.RejectModifierList(NO_MODIFIER_INVALID_IN_CONTEXT, true);
		parser.RejectAttribute(ATTRIBUTE_INVALID_IN_CONTEXT);

		auto current = stream.Current();
		Operator op;
		if (!current.isAssignment(op))
		{
			parser.Log(EXPECTED_ASSIGNMENT_OP, current);
		}

		AssignmentStatement* assignmentStatement;
		if (op == Operator_Assign)
		{
			stream.TryAdvance();
			assignmentStatement = AssignmentStatement::Create(TextSpan(), std::move(target), nullptr);
		}
		else
		{
			stream.TryAdvance();
			assignmentStatement = CompoundAssignmentStatement::Create(TextSpan(), op, std::move(target), nullptr);
		}

		Expression* expression;

		if (stream.Current().isDelimiterOf('{'))
		{
			InitializationExpression* initExpression;
			IF_ERR_RET_FALSE(ParserHelper::TryParseInitializationExpression(parser, stream, initExpression));
			expression = std::move(initExpression);
		}
		else
		{
			if (!HybridExpressionParser::ParseExpression(parser, stream, expression))
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

	static bool ParseDeclaration(const Token& start, Expression* target, Parser& parser, TokenStream& stream, ASTNode*& statementOut)
	{
		SymbolRef* symbol;
		if (!ParserHelper::MakeConcreteSymbolRef(target, SymbolRefType_Type, symbol))
		{
			parser.Log(EXPECTED_TYPE_EXPR, target->GetSpan());
		}
		ModifierList list;
		ModifierList allowed = ModifierList(AccessModifier_None, false, FunctionFlags_None, StorageClass_Const | StorageClass_Precise, InterpolationModifier_None, false);
		parser.AcceptModifierList(&list, allowed, INVALID_MODIFIER_ON_VAR);
		parser.RejectAttribute(ATTRIBUTE_INVALID_IN_CONTEXT);

		IdentifierInfo* identifier;
		stream.ExpectIdentifier(identifier, EXPECTED_IDENTIFIER);

		std::vector<size_t> arraySizes;
		if (parser.TryParseArraySizes(arraySizes))
		{
			symbol = SymbolRefArray::Create(symbol->GetSpan(), symbol->GetIdentifier(), arraySizes);
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

		Expression* expression;
		if (stream.Current().isDelimiterOf('{'))
		{
			InitializationExpression* initExpression;
			IF_ERR_RET_FALSE(ParserHelper::TryParseInitializationExpression(parser, stream, initExpression));
			IF_ERR_RET_FALSE(stream.ExpectDelimiter(';', EXPECTED_SEMICOLON));
			expression = initExpression;
		}
		else if (!stream.TryGetDelimiter(';'))
		{
			if (!HybridExpressionParser::ParseExpression(parser, stream, expression))
			{
				parser.TryRecoverStatement();
			}
			stream.ExpectDelimiter(';', EXPECTED_SEMICOLON);
		}

		auto span = stream.MakeFromLast(start);
		auto declarationStatement = DeclarationStatement::Create(span, identifier, symbol, list.storageClasses, expression);
		statementOut = declarationStatement;
		return true;
	}

	static bool ParseExpressionStatement(const Token& start, Expression* target, Parser& parser, TokenStream& stream, ASTNode*& statementOut)
	{
		parser.RejectModifierList(NO_MODIFIER_INVALID_IN_CONTEXT);
		parser.RejectAttribute(ATTRIBUTE_INVALID_IN_CONTEXT);

		ParseContext ctx = ParseContext(parser, stream);
		ctx.PushParseExpressionTask();

		if (target)
		{
			ctx.operandStack.push(target);
			auto& task = ctx.tasks.top();
			task.wasOperator = false;
			task.hadBrackets = false;
		}

		Expression* expr;
		HybridExpressionParser::ParseExpression(parser, stream, expr, ctx);

		stream.ExpectDelimiter(';', EXPECTED_SEMICOLON);
		auto span = stream.MakeFromLast(start);
		auto expressionStatement = ExpressionStatement::Create(span, expr);
		statementOut = expressionStatement;
		return true;
	}

	bool MemberAccessStatementParser::TryParse(Parser& parser, TokenStream& stream, ASTNode*& statementOut)
	{
		auto start = stream.Current();
		if (start.isDelimiterOf('(') || start.isUnaryOperator() || start.isLiteral() || start.isNumeric() || start.isKeywordOf(KeywordLiterals))
		{
			return ParseExpressionStatement(start, nullptr, parser, stream, statementOut);
		}

		Expression* target;
		if (!ParserHelper::TryParseMemberAccessPath(parser, stream, target))
		{
			return false;
		}

		auto current = stream.Current();

		if (current.isAssignment() || current.isCompoundAssignment())
		{
			return ParseAssignment(start, target, parser, stream, statementOut);
		}
		else if (current.isIdentifier())
		{
			return ParseDeclaration(start, target, parser, stream, statementOut);
		}
		else
		{
			return ParseExpressionStatement(start, target, parser, stream, statementOut);
		}
	}
}