#ifndef SWIZZLE_HPP
#define SWIZZLE_HPP

#include "ast_base.hpp"
#include "symbol_base.hpp"
#include "interfaces.hpp"
#include "primitive.hpp"

namespace HXSL
{
	class SwizzleDefinition : public SymbolDef
	{
		friend class ASTContext;
	private:
		Primitive* basePrim;
		uint8_t mask;
		SymbolRef* symbol;

		SwizzleDefinition(const TextSpan& span, IdentifierInfo* name, uint8_t mask, Primitive* basePrim, SymbolRef* symbol)
			: SymbolDef(span, ID, name),
			basePrim(basePrim),
			mask(mask),
			symbol(symbol)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_SwizzleDefinition;
		static SwizzleDefinition* Create(const TextSpan& span, IdentifierInfo* name, uint8_t mask, Primitive* basePrim, SymbolRef* symbol);

		uint8_t GetMask() const noexcept { return mask; }

		Primitive* GetBasePrim() const noexcept
		{
			return basePrim;
		}

		Primitive* GetTargetPrim() const noexcept
		{
			return dyn_cast<Primitive>(symbol->GetDeclaration());
		}

		SymbolRef* GetSymbolRef()
		{
			return symbol;
		}
	};
}

#endif