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

	static ast_ptr<SymbolRef> MakeOperatorSymbol(ASTContext* context)
	{
		return ast_ptr<SymbolRef>(SymbolRef::Create(context, TextSpan(), nullptr, SymbolRefType_OperatorOverload, false));
	}

	PrefixExpression* PrefixExpression::Create(ASTContext* context, const TextSpan& span, Operator op, ast_ptr<Expression>&& operand)
	{
		return context->Alloc<PrefixExpression>(sizeof(PrefixExpression), span, op, std::move(operand), MakeOperatorSymbol(context));
	}

	PostfixExpression* PostfixExpression::Create(ASTContext* context, const TextSpan& span, Operator op, ast_ptr<Expression>&& operand)
	{
		return context->Alloc<PostfixExpression>(sizeof(PostfixExpression), span, op, std::move(operand), MakeOperatorSymbol(context));
	}

	BinaryExpression* BinaryExpression::Create(ASTContext* context, const TextSpan& span, Operator op, ast_ptr<Expression>&& left, ast_ptr<Expression>&& right)
	{
		return context->Alloc<BinaryExpression>(sizeof(BinaryExpression), span, op, std::move(left), std::move(right), MakeOperatorSymbol(context));
	}

	CastExpression* CastExpression::Create(ASTContext* context, const TextSpan& span, std::optional<ast_ptr<SymbolRef>> operatorSymbol, ast_ptr<SymbolRef>&& typeSymbol, ast_ptr<Expression>&& operand)
	{
		ast_ptr<SymbolRef> sym = operatorSymbol.has_value() ? std::move(operatorSymbol.value()) : MakeOperatorSymbol(context);
		return context->Alloc<CastExpression>(sizeof(CastExpression), span, std::move(sym), std::move(typeSymbol), std::move(operand));
	}

	TernaryExpression* TernaryExpression::Create(ASTContext* context, const TextSpan& span, ast_ptr<Expression>&& condition, ast_ptr<Expression>&& trueBranch, ast_ptr<Expression>&& falseBranch)
	{
		return context->Alloc<TernaryExpression>(sizeof(TernaryExpression), span, std::move(condition), std::move(trueBranch), std::move(falseBranch));
	}

	EmptyExpression* EmptyExpression::Create(ASTContext* context, const TextSpan& span)
	{
		return context->Alloc<EmptyExpression>(sizeof(EmptyExpression), span);
	}

	LiteralExpression* LiteralExpression::Create(ASTContext* context, const TextSpan& span, const Token& token)
	{
		return context->Alloc<LiteralExpression>(sizeof(LiteralExpression), span, token);
	}

	MemberReferenceExpression* MemberReferenceExpression::Create(ASTContext* context, const TextSpan& span, ast_ptr<SymbolRef>&& symbol)
	{
		return context->Alloc<MemberReferenceExpression>(sizeof(MemberReferenceExpression), span, std::move(symbol));
	}

	FunctionCallParameter* FunctionCallParameter::Create(ASTContext* context, const TextSpan& span, ast_ptr<Expression>&& expression)
	{
		return context->Alloc<FunctionCallParameter>(sizeof(FunctionCallParameter), span, std::move(expression));
	}

	FunctionCallExpression* FunctionCallExpression::Create(ASTContext* context, const TextSpan& span, ast_ptr<SymbolRef>&& symbol, ArrayRef<ast_ptr<FunctionCallParameter>>& parameters)
	{
		auto ptr = context->Alloc<FunctionCallExpression>(TotalSizeToAlloc(parameters.size()), span, std::move(symbol));
		ptr->numParameters = static_cast<uint32_t>(parameters.size());
		std::uninitialized_move(parameters.begin(), parameters.end(), ptr->GetParameters().data());
		return ptr;
	}

	FunctionCallExpression* FunctionCallExpression::Create(ASTContext* context, const TextSpan& span, ast_ptr<SymbolRef>&& symbol, uint32_t numParameters)
	{
		auto ptr = context->Alloc<FunctionCallExpression>(TotalSizeToAlloc(numParameters), span, std::move(symbol));
		ptr->numParameters = numParameters;
		ptr->GetParameters().init();
		return ptr;
	}

	MemberAccessExpression* MemberAccessExpression::Create(ASTContext* context, const TextSpan& span, ast_ptr<SymbolRef>&& symbol, ast_ptr<ChainExpression>&& expression)
	{
		return context->Alloc<MemberAccessExpression>(sizeof(MemberAccessExpression), span, std::move(symbol), std::move(expression));
	}

	IndexerAccessExpression* IndexerAccessExpression::Create(ASTContext* context, const TextSpan& span, ast_ptr<SymbolRef>&& symbol, ast_ptr<Expression>&& indexExpression)
	{
		return context->Alloc<IndexerAccessExpression>(sizeof(IndexerAccessExpression), span, std::move(symbol), std::move(indexExpression));
	}

	AssignmentExpression* AssignmentExpression::Create(ASTContext* context, const TextSpan& span, ast_ptr<Expression>&& target, ast_ptr<Expression>&& expression)
	{
		return context->Alloc<AssignmentExpression>(sizeof(AssignmentExpression), span, ID, std::move(target), std::move(expression));
	}

	CompoundAssignmentExpression* CompoundAssignmentExpression::Create(ASTContext* context, const TextSpan& span, Operator _operator, ast_ptr<Expression>&& target, ast_ptr<Expression>&& expression)
	{
		return context->Alloc<CompoundAssignmentExpression>(sizeof(CompoundAssignmentExpression), span, _operator, std::move(target), std::move(expression), MakeOperatorSymbol(context));
	}

	InitializationExpression* InitializationExpression::Create(ASTContext* context, const TextSpan& span, ArrayRef<ast_ptr<Expression>>& parameters)
	{
		auto ptr = context->Alloc<InitializationExpression>(TotalSizeToAlloc(parameters.size()), span);
		ptr->numParameters = static_cast<uint32_t>(parameters.size());
		std::uninitialized_move(parameters.begin(), parameters.end(), ptr->GetParameters().data());
		return ptr;
	}

	InitializationExpression* InitializationExpression::Create(ASTContext* context, const TextSpan& span, uint32_t numParameters)
	{
		auto ptr = context->Alloc<InitializationExpression>(TotalSizeToAlloc(numParameters), span);
		ptr->numParameters = numParameters;
		ptr->GetParameters().init();
		return ptr;
	}
}