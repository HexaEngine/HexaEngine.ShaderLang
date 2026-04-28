#ifndef AST_ENUM_HPP
#define AST_ENUM_HPP

#include "ast_base.hpp"
#include "symbol_base.hpp"
#include "expressions.hpp"

namespace HXSL
{
	class EnumItem : public SymbolDef, public IHasExpressions
	{
		friend class ASTContext;
	private:
		Expression* value;
		union 
		{
			uint64_t computedValue = 0;
			int64_t computedSignedValue;
		};

	public:
		static constexpr NodeType ID = NodeType_EnumItem;

	protected:
		EnumItem(const TextSpan& span, IdentifierInfo* name, Expression* value)
			: SymbolDef(span, ID, name),
			value(value)
		{
			REGISTER_CHILD(value);
		}

	public:

		static EnumItem* Create(const TextSpan& span, IdentifierInfo* name, Expression* value);

		DEFINE_GETTER_SETTER_PTR(Expression*, Value, value);

		DEFINE_GETTER_SETTER(uint64_t, ComputedValue, computedValue);

		DEFINE_GETTER_SETTER(int64_t, ComputedSignedValue, computedSignedValue);

		void ForEachChild(ASTChildCallback cb, void* userdata);
		void ForEachChild(ASTConstChildCallback cb, void* userdata) const;
		void ForEachExpr(ExprChildCallback cb, void* userdata);
		void ForEachExpr(ExprConstChildCallback cb, void* userdata) const;
	};

	class Enum : public SymbolDef, public TrailingObjects<Enum, EnumItem*>
	{
		friend class ASTContext;
	private:
		TrailingObjStorage<Enum, uint32_t> storage;
		AccessModifier accessModifiers;
		SymbolRef* baseTypeRef;

	public:
		static constexpr NodeType ID = NodeType_Enum;
		static Enum* Create(const TextSpan& span, IdentifierInfo* name, AccessModifier accessModifiers, SymbolRef* baseTypeRef, const Span<EnumItem*>& items);

	protected:
		Enum(const TextSpan& span, IdentifierInfo* name, AccessModifier accessModifiers, SymbolRef* baseTypeRef)
			: SymbolDef(span, ID, name),
			accessModifiers(accessModifiers),
			baseTypeRef(baseTypeRef)
		{
		}

	public:
		DEFINE_TRAILING_OBJ_SPAN_GETTER(GetItems, 0, storage);

		AccessModifier GetAccessModifiers() const { return accessModifiers; }

		SymbolRef* GetSymbolRef() noexcept { return baseTypeRef; }
		SymbolDef* GetBaseType() const;

		void ForEachChild(ASTChildCallback cb, void* userdata);
		void ForEachChild(ASTConstChildCallback cb, void* userdata) const;
		std::string DebugName() const;
	};
}

#endif