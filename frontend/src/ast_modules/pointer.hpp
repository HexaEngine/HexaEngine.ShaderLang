#ifndef POINTER_HPP
#define POINTER_HPP

#include "ast_base.hpp"
#include "symbol_base.hpp"
#include "interfaces.hpp"

namespace HXSL
{
	class Pointer : public Type
	{
		friend class ASTContext;
	private:
		ast_ptr<SymbolRef> elementType;

		Pointer(const TextSpan& span, IdentifierInfo* name, ast_ptr<SymbolRef>&& elementType)
			: Type(span, ID, name, AccessModifier_Public),
			elementType(std::move(elementType))
		{
		}

	public:
		static constexpr NodeType ID = NodeType_Pointer;
		static Pointer* Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, ast_ptr<SymbolRef>&& elementType);

		SymbolDef* GetElementType() const
		{
			return elementType->GetDeclaration();
		}

		ast_ptr<SymbolRef>& GetSymbolRef()
		{
			return elementType;
		}
	};
}

#endif