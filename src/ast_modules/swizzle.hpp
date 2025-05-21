#ifndef SWIZZLE_HPP
#define SWIZZLE_HPP

#include "ast_base.hpp"
#include "symbol_base.hpp"
#include "interfaces.hpp"
#include "primitive.hpp"

namespace HXSL
{
	class SwizzleDefinition : public SymbolDef, public IHasSymbolRef
	{
	private:
		TextSpan expression;
		Primitive* basePrim;
		uint8_t mask;
		std::unique_ptr<SymbolRef> symbol;
	public:
		SwizzleDefinition(TextSpan expression, uint8_t mask, Primitive* basePrim, std::unique_ptr<SymbolRef> symbol)
			: ASTNode(expression, NodeType_SwizzleDefinition),
			SymbolDef(expression, NodeType_SwizzleDefinition, expression),
			expression(expression),
			basePrim(basePrim),
			mask(mask),
			symbol(std::move(symbol))
		{
		}

		uint8_t GetMask() const noexcept { return mask; }

		Primitive* GetBasePrim() const noexcept
		{
			return basePrim;
		}

		Primitive* GetTargetPrim() const noexcept
		{
			return symbol->GetDeclaration()->As<Primitive>();
		}

		const std::unique_ptr<SymbolRef>& GetSymbolRef()
		{
			return symbol;
		}

		SymbolType GetSymbolType() const override
		{
			return SymbolType_Field;
		}

		void Write(Stream& stream) const override
		{
			HXSL_ASSERT(false, "Cannot write swizzle types")
		}

		void Read(Stream& stream, StringPool& container) override
		{
			HXSL_ASSERT(false, "Cannot read swizzle types")
		}

		void Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<SymbolDef>>& nodes) override
		{
			HXSL_ASSERT(false, "Cannot build swizzle types")
		}
	};
}

#endif