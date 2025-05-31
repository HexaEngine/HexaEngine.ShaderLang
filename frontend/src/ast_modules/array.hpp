#ifndef ARRAY_HPP
#define ARRAY_HPP

#include "ast_base.hpp"
#include "symbol_base.hpp"
#include "interfaces.hpp"

namespace HXSL
{
	class Array : public Type
	{
	private:
		ast_ptr<SymbolRef> elementType;
		std::string backingName;
		size_t arraySize;
	public:
		static constexpr NodeType ID = NodeType_Array;
		Array(std::string& name, ast_ptr<SymbolRef>& elementType, size_t arraySize)
			: Type(TextSpan(), ID, TextSpan(), AccessModifier_Public),
			elementType(std::move(elementType)),
			backingName(std::move(name)),
			arraySize(arraySize)
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
			return SymbolType_Array;
		}

		size_t GetFieldOffset(Field* field) const override
		{
			return -1;
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

		void Build(SymbolTable& table, size_t index, CompilationUnit* compilation, std::vector<ast_ptr<SymbolDef>>& nodes) override
		{
			HXSL_ASSERT(false, "Cannot build array types")
		}
	};
}

#endif