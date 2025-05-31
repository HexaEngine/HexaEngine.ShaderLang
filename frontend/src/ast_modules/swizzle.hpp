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
	private:
		TextSpan expression;
		Primitive* basePrim;
		uint8_t mask;
		ast_ptr<SymbolRef> symbol;
	public:
		static constexpr NodeType ID = NodeType_SwizzleDefinition;
		SwizzleDefinition(TextSpan expression, uint8_t mask, Primitive* basePrim, ast_ptr<SymbolRef> symbol)
			: SymbolDef(expression, ID, expression),
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
			return dyn_cast<Primitive>(symbol->GetDeclaration());
		}

		const ast_ptr<SymbolRef>& GetSymbolRef()
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

		void Build(SymbolTable& table, size_t index, CompilationUnit* compilation, std::vector<ast_ptr<SymbolDef>>& nodes) override
		{
			HXSL_ASSERT(false, "Cannot build swizzle types")
		}
	};
}

#endif