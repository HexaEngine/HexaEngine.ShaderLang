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
		SymbolRef* elementType;

		Pointer(const TextSpan& span, IdentifierInfo* name, SymbolRef* elementType)
			: Type(span, ID, name, AccessModifier_Public),
			elementType(elementType)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_Pointer;
		static Pointer* Create(const TextSpan& span, IdentifierInfo* name, SymbolRef* elementType);

		SymbolDef* GetElementType() const
		{
			return elementType->GetDeclaration();
		}

		SymbolRef* GetSymbolRef()
		{
			return elementType;
		}
	};
}

#endif