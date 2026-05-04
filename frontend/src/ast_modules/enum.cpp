#include "enum.hpp"

namespace HXSL
{
	EnumItem* EnumItem::Create(const TextSpan& span, IdentifierInfo* name, Expression* value)
	{
		auto context = ASTContext::GetCurrentContext();
		return context->Alloc<EnumItem>(sizeof(EnumItem), span, name, value);
	}

	void EnumItem::ForEachChild(ASTChildCallback cb, void* userdata)
	{
		AST_ITERATE_CHILD_MUT(value);
	}

	void EnumItem::ForEachChild(ASTConstChildCallback cb, void* userdata) const
	{
		AST_ITERATE_CHILD(value);
	}

	void EnumItem::ForEachExpr(ExprChildCallback cb, void* userdata)
	{
		AST_ITERATE_CHILD_EXPR_MUT(value);
	}

	void EnumItem::ForEachExpr(ExprConstChildCallback cb, void* userdata) const
	{
		AST_ITERATE_CHILD(value);
	}

	std::string EnumItem::DebugName() const
	{
		std::ostringstream oss;
		oss << "[" << HXSL::ToString(type) << "] Name: " << GetName();
		return oss.str();
	}

	Enum* Enum::Create(const TextSpan& span, IdentifierInfo* name, AccessModifier accessModifiers, SymbolRef* baseTypeRef, const Span<EnumItem*>& items)
	{
		auto context = ASTContext::GetCurrentContext();
		auto node = context->Alloc<Enum>(TotalSizeToAlloc(items.size()), span, name, accessModifiers, baseTypeRef);
		node->storage.InitializeMove(node, items);
		REGISTER_CHILDREN_PTR(node, GetItems());
		return node;
	}

	SymbolDef* Enum::GetBaseType() const
	{
		return baseTypeRef->GetDeclaration();
	}

	void Enum::ForEachChild(ASTChildCallback cb, void* userdata)
	{
		AST_ITERATE_CHILDREN_MUT(GetItems);
	}

	void Enum::ForEachChild(ASTConstChildCallback cb, void* userdata) const
	{
		AST_ITERATE_CHILDREN(GetItems);
	}

	std::string Enum::DebugName() const
	{
		std::ostringstream oss;
		oss << "[" << HXSL::ToString(type) << "] Name: " << GetName();
		return oss.str();
	}
}