#ifndef POINTER_HPP
#define POINTER_HPP

#include "ast_base.hpp"
#include "symbol_base.hpp"
#include "interfaces.hpp"

namespace HXSL
{
	class Pointer : public Type, public IHasSymbolRef
	{
	private:
		ast_ptr<SymbolRef> elementType;
		std::string backingName;

	public:
		Pointer(std::string& name, ast_ptr<SymbolRef>& elementType)
			: Type(TextSpan(), NodeType_Pointer, TextSpan(), AccessModifier_Public),
			ASTNode(TextSpan(), NodeType_Pointer),
			elementType(std::move(elementType)),
			backingName(std::move(name))
		{
			this->name = backingName;
		}

		SymbolDef* GetElementType() const
		{
			return elementType->GetDeclaration();
		}

		ast_ptr<SymbolRef>& GetSymbolRef()
		{
			return elementType;
		}

		SymbolType GetSymbolType() const override
		{
			return SymbolType_Pointer;
		}

		size_t GetFieldOffset(Field* field) const override
		{
			return -1;
		}

		void Write(Stream& stream) const override
		{
			HXSL_ASSERT(false, "Cannot write pointer types")
		}

		void Read(Stream& stream, StringPool& container) override
		{
			HXSL_ASSERT(false, "Cannot read pointer types")
		}

		void Build(SymbolTable& table, size_t index, CompilationUnit* compilation, std::vector<ast_ptr<SymbolDef>>& nodes) override
		{
			HXSL_ASSERT(false, "Cannot build pointer types")
		}
	};
}

#endif