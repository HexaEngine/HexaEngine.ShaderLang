#include "expressions.hpp"
#include "declarations.hpp"
#include "statements.hpp"
#include "primitive.hpp"
#include "ast_context.hpp"

namespace HXSL
{
	bool Expression::IsVoidType() const noexcept
	{
		if (!inferredType) return false;
		auto prim = dyn_cast<Primitive>(inferredType);
		if (prim)
		{
			return prim->GetKind() == PrimitiveKind_Void;
		}
		return false;
	}

	static SymbolRef* MakeOperatorSymbol()
	{
		return SymbolRef::Create(TextSpan(), nullptr, SymbolRefType_OperatorOverload, false);
	}

	PrefixExpression* PrefixExpression::Create(const TextSpan& span, Operator op, Expression* operand)
	{
		auto* context = ASTContext::GetCurrentContext();
		return context->Alloc<PrefixExpression>(sizeof(PrefixExpression), span, op, operand, MakeOperatorSymbol());
	}

	PostfixExpression* PostfixExpression::Create(const TextSpan& span, Operator op, Expression* operand)
	{
		auto* context = ASTContext::GetCurrentContext();
		return context->Alloc<PostfixExpression>(sizeof(PostfixExpression), span, op, operand, MakeOperatorSymbol());
	}

	BinaryExpression* BinaryExpression::Create(const TextSpan& span, Operator op, Expression* left, Expression* right)
	{
		auto* context = ASTContext::GetCurrentContext();
		return context->Alloc<BinaryExpression>(sizeof(BinaryExpression), span, op, left, right, MakeOperatorSymbol());
	}

	CastExpression* CastExpression::Create(const TextSpan& span, SymbolRef* operatorSymbol, SymbolRef* typeSymbol, Expression* operand)
	{
		auto* context = ASTContext::GetCurrentContext();
		auto* sym = operatorSymbol ? operatorSymbol : MakeOperatorSymbol();
		return context->Alloc<CastExpression>(sizeof(CastExpression), span, operatorSymbol, typeSymbol, operand);
	}

	TernaryExpression* TernaryExpression::Create(const TextSpan& span, Expression* condition, Expression* trueBranch, Expression* falseBranch)
	{
		auto* context = ASTContext::GetCurrentContext();
		return context->Alloc<TernaryExpression>(sizeof(TernaryExpression), span, condition, trueBranch, falseBranch);
	}

	EmptyExpression* EmptyExpression::Create(const TextSpan& span)
	{
		auto* context = ASTContext::GetCurrentContext();
		return context->Alloc<EmptyExpression>(sizeof(EmptyExpression), span);
	}

	LiteralExpression* LiteralExpression::Create(const TextSpan& span, const Token& token)
	{
		auto* context = ASTContext::GetCurrentContext();
		return context->Alloc<LiteralExpression>(sizeof(LiteralExpression), span, token);
	}

	MemberReferenceExpression* MemberReferenceExpression::Create(const TextSpan& span, SymbolRef* symbol)
	{
		auto* context = ASTContext::GetCurrentContext();
		return context->Alloc<MemberReferenceExpression>(sizeof(MemberReferenceExpression), span, symbol);
	}

	FunctionCallParameter* FunctionCallParameter::Create(const TextSpan& span, Expression* expression)
	{
		auto* context = ASTContext::GetCurrentContext();
		return context->Alloc<FunctionCallParameter>(sizeof(FunctionCallParameter), span, expression);
	}

	FunctionCallExpression* FunctionCallExpression::Create(const TextSpan& span, SymbolRef* symbol, const Span<FunctionCallParameter*>& parameters)
	{
		auto* context = ASTContext::GetCurrentContext();
		auto ptr = context->Alloc<FunctionCallExpression>(TotalSizeToAlloc(parameters.size()), span, symbol);
		ptr->storage.InitializeMove(ptr, parameters);
		REGISTER_CHILDREN_PTR(ptr, GetParameters());
		return ptr;
	}

	FunctionCallExpression* FunctionCallExpression::Create(const TextSpan& span, SymbolRef* symbol, uint32_t numParameters)
	{
		auto* context = ASTContext::GetCurrentContext();
		auto ptr = context->Alloc<FunctionCallExpression>(TotalSizeToAlloc(numParameters), span, symbol);
		ptr->storage.SetCounts(numParameters);
		return ptr;
	}

	ConstructorCallExpression* ConstructorCallExpression::Create(const TextSpan& span, SymbolRef* symbol, const Span<FunctionCallParameter*>& parameters)
	{
		auto* context = ASTContext::GetCurrentContext();
		auto ptr = context->Alloc<ConstructorCallExpression>(TotalSizeToAlloc(parameters.size()), span, symbol);
		ptr->storage.InitializeMove(ptr, parameters);
		REGISTER_CHILDREN_PTR(ptr, GetParameters());
		return ptr;
	}

	ConstructorCallExpression* ConstructorCallExpression::Create(const TextSpan& span, SymbolRef* symbol, uint32_t numParameters)
	{
		auto* context = ASTContext::GetCurrentContext();
		auto ptr = context->Alloc<ConstructorCallExpression>(TotalSizeToAlloc(numParameters), span, symbol);
		ptr->storage.SetCounts(numParameters);
		return ptr;
	}

	MemberAccessExpression* MemberAccessExpression::Create(const TextSpan& span, SymbolRef* symbol, ChainExpression* expression)
	{
		auto* context = ASTContext::GetCurrentContext();
		return context->Alloc<MemberAccessExpression>(sizeof(MemberAccessExpression), span, symbol, expression);
	}

	IndexerAccessExpression* IndexerAccessExpression::Create(const TextSpan& span, SymbolRef* symbol, Expression* indexExpression)
	{
		auto* context = ASTContext::GetCurrentContext();
		return context->Alloc<IndexerAccessExpression>(sizeof(IndexerAccessExpression), span, symbol, indexExpression);
	}

	AssignmentExpression* AssignmentExpression::Create(const TextSpan& span, Expression* target, Expression* expression)
	{
		auto* context = ASTContext::GetCurrentContext();
		return context->Alloc<AssignmentExpression>(sizeof(AssignmentExpression), span, ID, target, expression);
	}

	CompoundAssignmentExpression* CompoundAssignmentExpression::Create(const TextSpan& span, Operator _operator, Expression* target, Expression* expression)
	{
		auto* context = ASTContext::GetCurrentContext();
		return context->Alloc<CompoundAssignmentExpression>(sizeof(CompoundAssignmentExpression), span, _operator, target, expression, MakeOperatorSymbol());
	}

	InitializationExpression* InitializationExpression::Create(const TextSpan& span, const Span<Expression*>& parameters)
	{
		auto* context = ASTContext::GetCurrentContext();
		auto ptr = context->Alloc<InitializationExpression>(TotalSizeToAlloc(parameters.size()), span);
		ptr->storage.InitializeMove(ptr, parameters);
		return ptr;
	}

	InitializationExpression* InitializationExpression::Create(const TextSpan& span, uint32_t numParameters)
	{
		auto* context = ASTContext::GetCurrentContext();
		auto ptr = context->Alloc<InitializationExpression>(TotalSizeToAlloc(numParameters), span);
		ptr->storage.SetCounts(numParameters);
		return ptr;
	}

	void BinaryExpression::ForEachChild(ASTChildCallback cb, void* userdata)
	{
		AST_ITERATE_CHILD_MUT(left);
		AST_ITERATE_CHILD_MUT(right);
	}

	void BinaryExpression::ForEachChild(ASTConstChildCallback cb, void* userdata) const
	{
		AST_ITERATE_CHILD(left);
		AST_ITERATE_CHILD(right);
	}

	void MemberReferenceExpression::ForEachChild(ASTChildCallback cb, void* userdata)
	{
		if (next) AST_ITERATE_CHILD_MUT(next);
	}

	void MemberReferenceExpression::ForEachChild(ASTConstChildCallback cb, void* userdata) const
	{
		if (next) AST_ITERATE_CHILD(next);
	}

	void FunctionCallExpression::ForEachChild(ASTChildCallback cb, void* userdata)
	{
		AST_ITERATE_CHILDREN_MUT(GetParameters);
		if (next) AST_ITERATE_CHILD_MUT(next);
	}

	void FunctionCallExpression::ForEachChild(ASTConstChildCallback cb, void* userdata) const
	{
		AST_ITERATE_CHILDREN(GetParameters);
		if (next) AST_ITERATE_CHILD(next);
	}

	void FunctionCallParameter::ForEachChild(ASTChildCallback cb, void* userdata)
	{
		AST_ITERATE_CHILD_MUT(expression);
	}

	void FunctionCallParameter::ForEachChild(ASTConstChildCallback cb, void* userdata) const
	{
		AST_ITERATE_CHILD(expression);
	}

	void MemberAccessExpression::ForEachChild(ASTChildCallback cb, void* userdata)
	{
		if (next) AST_ITERATE_CHILD_MUT(next);
	}

	void MemberAccessExpression::ForEachChild(ASTConstChildCallback cb, void* userdata) const
	{
		if (next) AST_ITERATE_CHILD(next);
	}

	void IndexerAccessExpression::ForEachChild(ASTChildCallback cb, void* userdata)
	{
		AST_ITERATE_CHILD_MUT(indexExpression);
		if (next) AST_ITERATE_CHILD_MUT(next);
	}

	void IndexerAccessExpression::ForEachChild(ASTConstChildCallback cb, void* userdata) const
	{
		AST_ITERATE_CHILD(indexExpression);
		if (next) AST_ITERATE_CHILD(next);
	}

	void CastExpression::ForEachChild(ASTChildCallback cb, void* userdata)
	{
		AST_ITERATE_CHILD_MUT(operand);
	}

	void CastExpression::ForEachChild(ASTConstChildCallback cb, void* userdata) const
	{
		AST_ITERATE_CHILD(operand);
	}

	void TernaryExpression::ForEachChild(ASTChildCallback cb, void* userdata)
	{
		AST_ITERATE_CHILD_MUT(condition);
		AST_ITERATE_CHILD_MUT(trueBranch);
		AST_ITERATE_CHILD_MUT(falseBranch);
	}

	void TernaryExpression::ForEachChild(ASTConstChildCallback cb, void* userdata) const
	{
		AST_ITERATE_CHILD(condition);
		AST_ITERATE_CHILD(trueBranch);
		AST_ITERATE_CHILD(falseBranch);
	}

	void PrefixExpression::ForEachChild(ASTChildCallback cb, void* userdata)
	{
		AST_ITERATE_CHILD_MUT(operand);
	}

	void PrefixExpression::ForEachChild(ASTConstChildCallback cb, void* userdata) const
	{
		AST_ITERATE_CHILD(operand);
	}

	void PostfixExpression::ForEachChild(ASTChildCallback cb, void* userdata)
	{
		AST_ITERATE_CHILD_MUT(operand);
	}

	void PostfixExpression::ForEachChild(ASTConstChildCallback cb, void* userdata) const
	{
		AST_ITERATE_CHILD(operand);
	}

	void AssignmentExpression::ForEachChild(ASTChildCallback cb, void* userdata)
	{
		AST_ITERATE_CHILD_MUT(target);
		AST_ITERATE_CHILD_MUT(expression);
	}

	void AssignmentExpression::ForEachChild(ASTConstChildCallback cb, void* userdata) const
	{
		AST_ITERATE_CHILD(target);
		AST_ITERATE_CHILD(expression);
	}

	void CompoundAssignmentExpression::ForEachChild(ASTChildCallback cb, void* userdata)
	{
		AST_ITERATE_CHILD_MUT(target);
		AST_ITERATE_CHILD_MUT(expression);
	}

	void CompoundAssignmentExpression::ForEachChild(ASTConstChildCallback cb, void* userdata) const
	{
		AST_ITERATE_CHILD(target);
		AST_ITERATE_CHILD(expression);
	}

	void InitializationExpression::ForEachChild(ASTChildCallback cb, void* userdata)
	{
		AST_ITERATE_CHILDREN_MUT(GetParameters);
	}

	void InitializationExpression::ForEachChild(ASTConstChildCallback cb, void* userdata) const
	{
		AST_ITERATE_CHILDREN(GetParameters);
	}
}