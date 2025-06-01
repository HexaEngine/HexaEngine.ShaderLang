#ifndef ARRAY_HPP
#define ARRAY_HPP

#include "ast_base.hpp"
#include "symbol_base.hpp"
#include "interfaces.hpp"

namespace HXSL
{
	class Array : public Type
	{
		friend class ASTContext;
	private:
		ast_ptr<SymbolRef> elementType;
		size_t arraySize;

		Array(const TextSpan& span, IdentifierInfo* name, ast_ptr<SymbolRef>&& elementType, size_t arraySize)
			: Type(TextSpan(), ID, name, AccessModifier_Public),
			elementType(std::move(elementType)),
			arraySize(arraySize)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_Array;
		static Array* Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, ast_ptr<SymbolRef>&& elementType, size_t arraySize);

		SymbolDef* GetElementType() const
		{
			return elementType->GetDeclaration();
		}

		ast_ptr<SymbolRef>& GetSymbolRef()
		{
			return elementType;
		}

		DEFINE_GETTER_SETTER(size_t, ArraySize, arraySize)
	};
}

#endif