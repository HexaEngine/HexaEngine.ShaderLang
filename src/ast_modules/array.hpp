#ifndef ARRAY_HPP
#define ARRAY_HPP

#include "ast_base.hpp"
#include "symbol_base.hpp"
#include "interfaces.hpp"

namespace HXSL
{
	class Array : public Type, public IHasSymbolRef
	{
	private:
		std::unique_ptr<SymbolRef> elementType;
		std::string backingName;
		size_t arraySize;
	public:
		Array(std::string& name, std::unique_ptr<SymbolRef>& elementType, size_t arraySize)
			: Type(TextSpan(), nullptr, NodeType_Array, TextSpan()),
			ASTNode(TextSpan(), nullptr, NodeType_Array),
			elementType(std::move(elementType)),
			backingName(std::move(name)),
			arraySize(arraySize)
		{
			this->name = TextSpan(backingName);
		}

		SymbolDef* GetElementType() const
		{
			return elementType->GetDeclaration();
		}

		std::unique_ptr<SymbolRef>& GetSymbolRef()
		{
			return elementType;
		}

		SymbolType GetSymbolType() const override
		{
			return SymbolType_Array;
		}

		DEFINE_GETTER_SETTER(size_t, ArraySize, arraySize)

			void Write(Stream& stream) const override
		{
			HXSL_ASSERT(false, "Cannot write array types")
		}

		void Read(Stream& stream, StringPool& container) override
		{
			HXSL_ASSERT(false, "Cannot read array types")
		}

		void Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<SymbolDef>>& nodes) override
		{
			HXSL_ASSERT(false, "Cannot build array types")
		}
	};
}

#endif