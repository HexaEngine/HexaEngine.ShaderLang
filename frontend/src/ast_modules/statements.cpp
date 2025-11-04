#include "statements.hpp"
#include "declarations.hpp"

namespace HXSL
{
	BlockStatement* BlockStatement::Create(const TextSpan& span, const ArrayRef<ASTNode*>& statements)
	{
		auto context = ASTContext::GetCurrentContext();
		auto ptr = context->Alloc<BlockStatement>(TotalSizeToAlloc(statements.size()), span);
		ptr->storage.InitializeMove(ptr, statements);
		return ptr;
	}

	DeclarationStatement* DeclarationStatement::Create(const TextSpan& span, IdentifierInfo* name, SymbolRef* symbol, StorageClass storageClass, Expression* initializer)
	{
		auto context = ASTContext::GetCurrentContext();
		return context->Alloc<DeclarationStatement>(sizeof(DeclarationStatement), span, name, symbol, storageClass, initializer);
	}

	AssignmentStatement* AssignmentStatement::Create(const TextSpan& span, Expression* target, Expression* expression)
	{
		auto context = ASTContext::GetCurrentContext();
		auto expr = AssignmentExpression::Create(span, target, expression);
		return context->Alloc<AssignmentStatement>(sizeof(AssignmentStatement), span, ID, expr);
	}

	CompoundAssignmentStatement* CompoundAssignmentStatement::Create(const TextSpan& span, Operator op, Expression* target, Expression* expression)
	{
		auto context = ASTContext::GetCurrentContext();
		auto expr = CompoundAssignmentExpression::Create(span, op, target, expression);
		return context->Alloc<CompoundAssignmentStatement>(sizeof(CompoundAssignmentStatement), span, op, expr);
	}

	ExpressionStatement* ExpressionStatement::Create(const TextSpan& span, Expression* expression)
	{
		auto context = ASTContext::GetCurrentContext();
		return context->Alloc<ExpressionStatement>(sizeof(ExpressionStatement), span, expression);
	}

	ReturnStatement* ReturnStatement::Create(const TextSpan& span, Expression* returnValueExpression)
	{
		auto context = ASTContext::GetCurrentContext();
		return context->Alloc<ReturnStatement>(sizeof(ReturnStatement), span, returnValueExpression);
	}

	ElseStatement* ElseStatement::Create(const TextSpan& span, BlockStatement* body)
	{
		auto context = ASTContext::GetCurrentContext();
		return context->Alloc<ElseStatement>(sizeof(ElseStatement), span, body);
	}

	ElseIfStatement* ElseIfStatement::Create(const TextSpan& span, Expression* condition, BlockStatement* body)
	{
		auto context = ASTContext::GetCurrentContext();
		return context->Alloc<ElseIfStatement>(sizeof(ElseIfStatement), span, condition, body);
	}

	IfStatement* IfStatement::Create(const TextSpan& span, Expression* condition, BlockStatement* body, const ArrayRef<ElseIfStatement*>& elseIfStatements, ElseStatement* elseStatement, const ArrayRef<AttributeDecl*>& attributes)
	{
		auto context = ASTContext::GetCurrentContext();
		auto ptr = context->Alloc<IfStatement>(TotalSizeToAlloc(elseIfStatements.size(), attributes.size()), span, condition, body, elseStatement);
		ptr->storage.InitializeMove(ptr, elseIfStatements, attributes);
		return ptr;
	}

	IfStatement* IfStatement::Create(const TextSpan& span, Expression* condition, BlockStatement* body, uint32_t numElseIfStatements, ElseStatement* elseStatement, uint32_t numAttributes)
	{
		auto context = ASTContext::GetCurrentContext();
		auto ptr = context->Alloc<IfStatement>(TotalSizeToAlloc(numElseIfStatements, numAttributes), span, condition, body, elseStatement);
		ptr->storage.SetCounts(numElseIfStatements, numAttributes);
		return ptr;
	}

	CaseStatement* CaseStatement::Create(const TextSpan& span, Expression* expression, const ArrayRef<ASTNode*>& statements)
	{
		auto context = ASTContext::GetCurrentContext();
		auto ptr = context->Alloc<CaseStatement>(TotalSizeToAlloc(statements.size()), span, expression);
		ptr->storage.InitializeMove(ptr, statements);
		return ptr;
	}

	CaseStatement* CaseStatement::Create(const TextSpan& span, Expression* expression, uint32_t numStatements)
	{
		auto context = ASTContext::GetCurrentContext();
		auto ptr = context->Alloc<CaseStatement>(TotalSizeToAlloc(numStatements), span, expression);
		ptr->storage.SetCounts(numStatements);
		return ptr;
	}

	DefaultCaseStatement* DefaultCaseStatement::Create(const TextSpan& span, const ArrayRef<ASTNode*>& statements)
	{
		auto context = ASTContext::GetCurrentContext();
		auto ptr = context->Alloc<DefaultCaseStatement>(TotalSizeToAlloc(statements.size()), span);
		ptr->storage.InitializeMove(ptr, statements);
		return ptr;
	}

	DefaultCaseStatement* DefaultCaseStatement::Create(const TextSpan& span, uint32_t numStatements)
	{
		auto context = ASTContext::GetCurrentContext();
		auto ptr = context->Alloc<DefaultCaseStatement>(TotalSizeToAlloc(numStatements), span);
		ptr->storage.SetCounts(numStatements);
		return ptr;
	}

	SwitchStatement* SwitchStatement::Create(const TextSpan& span, Expression* expression, const ArrayRef<CaseStatement*>& cases, DefaultCaseStatement* defaultCase, const ArrayRef<AttributeDecl*>& attributes)
	{
		auto context = ASTContext::GetCurrentContext();
		auto ptr = context->Alloc<SwitchStatement>(TotalSizeToAlloc(cases.size(), attributes.size()), span, expression, defaultCase);
		ptr->storage.InitializeMove(ptr, cases, attributes);
		return ptr;
	}

	SwitchStatement* SwitchStatement::Create(const TextSpan& span, Expression* expression, uint32_t numCases, DefaultCaseStatement* defaultCase, uint32_t numAttributes)
	{
		auto context = ASTContext::GetCurrentContext();
		auto ptr = context->Alloc<SwitchStatement>(TotalSizeToAlloc(numCases, numAttributes), span, expression, defaultCase);
		ptr->storage.SetCounts(numCases, numAttributes);
		return ptr;
	}

	ForStatement* ForStatement::Create(const TextSpan& span, ASTNode* init, Expression* condition, Expression* iteration, BlockStatement* body, const ArrayRef<AttributeDecl*>& attributes)
	{
		auto context = ASTContext::GetCurrentContext();
		auto ptr = context->Alloc<ForStatement>(TotalSizeToAlloc(attributes.size()), span, init, condition, iteration, body);
		ptr->storage.InitializeMove(ptr, attributes);
		return ptr;
	}

	ForStatement* ForStatement::Create(const TextSpan& span, ASTNode* init, Expression* condition, Expression* iteration, BlockStatement* body, uint32_t numAttributes)
	{
		auto context = ASTContext::GetCurrentContext();
		auto ptr = context->Alloc<ForStatement>(TotalSizeToAlloc(numAttributes), span, init, condition, iteration, body);
		ptr->storage.SetCounts(numAttributes);
		return ptr;
	}

	BreakStatement* BreakStatement::Create(const TextSpan& span)
	{
		auto context = ASTContext::GetCurrentContext();
		return context->Alloc<BreakStatement>(sizeof(BreakStatement), span);
	}

	ContinueStatement* ContinueStatement::Create(const TextSpan& span)
	{
		auto context = ASTContext::GetCurrentContext();
		return context->Alloc<ContinueStatement>(sizeof(ContinueStatement), span);
	}

	DiscardStatement* DiscardStatement::Create(const TextSpan& span)
	{
		auto context = ASTContext::GetCurrentContext();
		return context->Alloc<DiscardStatement>(sizeof(DiscardStatement), span);
	}

	WhileStatement* WhileStatement::Create(const TextSpan& span, Expression* condition, BlockStatement* body, const ArrayRef<AttributeDecl*>& attributes)
	{
		auto context = ASTContext::GetCurrentContext();
		auto ptr = context->Alloc<WhileStatement>(TotalSizeToAlloc(attributes.size()), span, condition, body);
		ptr->storage.InitializeMove(ptr, attributes);
		return ptr;
	}

	WhileStatement* WhileStatement::Create(const TextSpan& span, Expression* condition, BlockStatement* body, uint32_t numAttributes)
	{
		auto context = ASTContext::GetCurrentContext();
		auto ptr = context->Alloc<WhileStatement>(TotalSizeToAlloc(numAttributes), span, condition, body);
		ptr->storage.SetCounts(numAttributes);
		return ptr;
	}

	DoWhileStatement* DoWhileStatement::Create(const TextSpan& span, Expression* condition, BlockStatement* body, const ArrayRef<AttributeDecl*>& attributes)
	{
		auto context = ASTContext::GetCurrentContext();
		auto ptr = context->Alloc<DoWhileStatement>(TotalSizeToAlloc(attributes.size()), span, condition, body);
		ptr->storage.InitializeMove(ptr, attributes);
		return ptr;
	}

	DoWhileStatement* DoWhileStatement::Create(const TextSpan& span, Expression* condition, BlockStatement* body, uint32_t numAttributes)
	{
		auto context = ASTContext::GetCurrentContext();
		auto ptr = context->Alloc<DoWhileStatement>(TotalSizeToAlloc(numAttributes), span, condition, body);
		ptr->storage.SetCounts(numAttributes);
		return ptr;
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

	void DeclarationStatement::Build(SymbolTable& table, size_t index, CompilationUnit* compilation, std::vector<SymbolDef*>& nodes)
	{
		//HXSL_ASSERT(false, "Cannot build declaration statements.");
	}
	*/
}