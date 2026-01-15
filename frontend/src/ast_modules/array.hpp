#ifndef ARRAY_HPP
#define ARRAY_HPP

#include "ast_base.hpp"
#include "symbol_base.hpp"
#include "interfaces.hpp"

namespace HXSL
{
	class ArrayDecl : public Type
	{
		friend class ASTContext;
	private:
		SymbolRef* elementType;
		size_t arraySize;

		ArrayDecl(const TextSpan& span, IdentifierInfo* name, SymbolRef* elementType, size_t arraySize)
			: Type(TextSpan(), ID, name, AccessModifier_Public),
			elementType(elementType),
			arraySize(arraySize)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_Array;
		static ArrayDecl* Create(const TextSpan& span, IdentifierInfo* name, SymbolRef* elementType, size_t arraySize);

		SymbolDef* GetElementType() const
		{
			return elementType->GetDeclaration();
		}

		SymbolRef* GetSymbolRef()
		{
			return elementType;
		}

		DEFINE_GETTER_SETTER(size_t, ArraySize, arraySize);

		void ForEachChild(ASTChildCallback cb, void* userdata) {}
		void ForEachChild(ASTConstChildCallback cb, void* userdata) const {}
	};
}

#endif