#include "statements.hpp"
#include "declarations.hpp"

namespace HXSL
{
	BlockStatement* BlockStatement::Create(ASTContext* context, const TextSpan& span, ArrayRef<ast_ptr<ASTNode>>& statements)
	{
		auto ptr = context->Alloc<BlockStatement>(TotalSizeToAlloc(statements.size()), span);
		ptr->numStatements = static_cast<uint32_t>(statements.size());
		std::uninitialized_move(statements.begin(), statements.end(), ptr->GetStatements().data());
		return ptr;
	}

	BlockStatement* BlockStatement::Create(ASTContext* context, const TextSpan& span, uint32_t numStatements)
	{
		auto ptr = context->Alloc<BlockStatement>(TotalSizeToAlloc(numStatements), span);
		ptr->numStatements = numStatements;
		ptr->GetStatements().init();
		return ptr;
	}

	DeclarationStatement* DeclarationStatement::Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, ast_ptr<SymbolRef>&& symbol, StorageClass storageClass, ast_ptr<Expression>&& initializer)
	{
		return context->Alloc<DeclarationStatement>(sizeof(DeclarationStatement), span, name, std::move(symbol), storageClass, std::move(initializer));
	}

	AssignmentStatement* AssignmentStatement::Create(ASTContext* context, const TextSpan& span, ast_ptr<Expression>&& target, ast_ptr<Expression>&& expression)
	{
		auto expr = AssignmentExpression::Create(context, span, std::move(target), std::move(expression));
		return context->Alloc<AssignmentStatement>(sizeof(AssignmentStatement), span, ID, ast_ptr<AssignmentExpression>(expr));
	}

	CompoundAssignmentStatement* Create(ASTContext* context, const TextSpan& span, Operator op, ast_ptr<Expression>&& target, ast_ptr<Expression>&& expression)
	{
		auto expr = CompoundAssignmentExpression::Create(context, span, op, std::move(target), std::move(expression));
		return context->Alloc<CompoundAssignmentStatement>(sizeof(CompoundAssignmentStatement), span, op, ast_ptr<CompoundAssignmentExpression>(expr));
	}

	ExpressionStatement* ExpressionStatement::Create(ASTContext* context, const TextSpan& span, ast_ptr<Expression>&& expression)
	{
		return context->Alloc<ExpressionStatement>(sizeof(ExpressionStatement), span, std::move(expression));
	}

	ReturnStatement* ReturnStatement::Create(ASTContext* context, const TextSpan& span, ast_ptr<Expression>&& returnValueExpression)
	{
		return context->Alloc<ReturnStatement>(sizeof(ReturnStatement), span, std::move(returnValueExpression));
	}

	ElseStatement* ElseStatement::Create(ASTContext* context, const TextSpan& span, ast_ptr<BlockStatement>&& body)
	{
		return context->Alloc<ElseStatement>(sizeof(ElseStatement), span, std::move(body));
	}

	ElseIfStatement* ElseIfStatement::Create(ASTContext* context, const TextSpan& span, ast_ptr<Expression>&& condition, ast_ptr<BlockStatement>&& body)
	{
		return context->Alloc<ElseIfStatement>(sizeof(ElseIfStatement), span, std::move(condition), std::move(body));
	}

	IfStatement* IfStatement::Create(ASTContext* context, const TextSpan& span, ast_ptr<Expression>&& condition, ast_ptr<BlockStatement>&& body, ArrayRef<ast_ptr<ElseIfStatement>>& elseIfStatements, ast_ptr<ElseStatement>&& elseStatement)
	{
		auto ptr = context->Alloc<IfStatement>(TotalSizeToAlloc(elseIfStatements.size()), span, std::move(condition), std::move(body), std::move(elseStatement));
		ptr->numElseIfStatements = static_cast<uint32_t>(elseIfStatements.size());
		std::uninitialized_move(elseIfStatements.begin(), elseIfStatements.end(), ptr->GetElseIfStatements().data());
		return ptr;
	}

	IfStatement* IfStatement::Create(ASTContext* context, const TextSpan& span, ast_ptr<Expression>&& condition, ast_ptr<BlockStatement>&& body, uint32_t numElseIfStatements, ast_ptr<ElseStatement>&& elseStatement)
	{
		auto ptr = context->Alloc<IfStatement>(TotalSizeToAlloc(numElseIfStatements), span, std::move(condition), std::move(body), std::move(elseStatement));
		ptr->numElseIfStatements = numElseIfStatements;
		ptr->GetElseIfStatements().init();
		return ptr;
	}

	CaseStatement* CaseStatement::Create(ASTContext* context, const TextSpan& span, ast_ptr<Expression>&& expression, ArrayRef<ast_ptr<ASTNode>>& statements)
	{
		auto ptr = context->Alloc<CaseStatement>(TotalSizeToAlloc(statements.size()), span, std::move(expression));
		ptr->numStatements = static_cast<uint32_t>(statements.size());
		std::uninitialized_move(statements.begin(), statements.end(), ptr->GetStatements().data());
		return ptr;
	}

	CaseStatement* CaseStatement::Create(ASTContext* context, const TextSpan& span, ast_ptr<Expression>&& expression, uint32_t numStatements)
	{
		auto ptr = context->Alloc<CaseStatement>(TotalSizeToAlloc(numStatements), span, std::move(expression));
		ptr->numStatements = static_cast<uint32_t>(numStatements);
		ptr->GetStatements().init();
		return ptr;
	}

	DefaultCaseStatement* DefaultCaseStatement::Create(ASTContext* context, const TextSpan& span, ArrayRef<ast_ptr<ASTNode>>& statements)
	{
		auto ptr = context->Alloc<DefaultCaseStatement>(TotalSizeToAlloc(statements.size()), span);
		ptr->numStatements = static_cast<uint32_t>(statements.size());
		std::uninitialized_move(statements.begin(), statements.end(), ptr->GetStatements().data());
		return ptr;
	}

	DefaultCaseStatement* DefaultCaseStatement::Create(ASTContext* context, const TextSpan& span, uint32_t numStatements)
	{
		auto ptr = context->Alloc<DefaultCaseStatement>(TotalSizeToAlloc(numStatements), span);
		ptr->numStatements = static_cast<uint32_t>(numStatements);
		ptr->GetStatements().init();
		return ptr;
	}

	SwitchStatement* SwitchStatement::Create(ASTContext* context, const TextSpan& span, ast_ptr<Expression>&& expression, ArrayRef<ast_ptr<CaseStatement>>& cases, ast_ptr<DefaultCaseStatement>&& defaultCase)
	{
		auto ptr = context->Alloc<SwitchStatement>(TotalSizeToAlloc(cases.size()), span, std::move(expression), std::move(defaultCase));
		ptr->numCases = static_cast<uint32_t>(cases.size());
		std::uninitialized_move(cases.begin(), cases.end(), ptr->GetCases().data());
		return ptr;
	}

	SwitchStatement* SwitchStatement::Create(ASTContext* context, const TextSpan& span, ast_ptr<Expression>&& expression, uint32_t numCases, ast_ptr<DefaultCaseStatement>&& defaultCase)
	{
		auto ptr = context->Alloc<SwitchStatement>(TotalSizeToAlloc(numCases), span, std::move(expression), std::move(defaultCase));
		ptr->numCases = static_cast<uint32_t>(numCases);
		ptr->GetCases().init();
		return ptr;
	}

	ForStatement* ForStatement::Create(ASTContext* context, const TextSpan& span, ast_ptr<ASTNode>&& init, ast_ptr<Expression>&& condition, ast_ptr<Expression>&& iteration, ast_ptr<BlockStatement>&& body)
	{
		return context->Alloc<ForStatement>(sizeof(ForStatement), span, std::move(init), std::move(condition), std::move(iteration), std::move(body));
	}

	BreakStatement* BreakStatement::Create(ASTContext* context, const TextSpan& span)
	{
		return context->Alloc<BreakStatement>(sizeof(BreakStatement), span);
	}

	ContinueStatement* ContinueStatement::Create(ASTContext* context, const TextSpan& span)
	{
		return context->Alloc<ContinueStatement>(sizeof(ContinueStatement), span);
	}

	DiscardStatement* DiscardStatement::Create(ASTContext* context, const TextSpan& span)
	{
		return context->Alloc<DiscardStatement>(sizeof(DiscardStatement), span);
	}

	WhileStatement* WhileStatement::Create(ASTContext* context, const TextSpan& span, ast_ptr<Expression>&& condition, ast_ptr<BlockStatement>&& body)
	{
		return context->Alloc<WhileStatement>(sizeof(WhileStatement), span, std::move(condition), std::move(body));
	}

	DoWhileStatement* DoWhileStatement::Create(ASTContext* context, const TextSpan& span, ast_ptr<Expression>&& condition, ast_ptr<BlockStatement>&& body)
	{
		return context->Alloc<DoWhileStatement>(sizeof(DoWhileStatement), span, std::move(condition), std::move(body));
	}

	/*
	void DeclarationStatement::Write(Stream& stream) const
	{
		//HXSL_ASSERT(false, "Cannot write declaration statements.");
	}

	void DeclarationStatement::Read(Stream& stream, StringPool& container)
	{
		//HXSL_ASSERT(false, "Cannot read declaration statements.");
	}

	void DeclarationStatement::Build(SymbolTable& table, size_t index, CompilationUnit* compilation, std::vector<ast_ptr<SymbolDef>>& nodes)
	{
		//HXSL_ASSERT(false, "Cannot build declaration statements.");
	}
	*/
}