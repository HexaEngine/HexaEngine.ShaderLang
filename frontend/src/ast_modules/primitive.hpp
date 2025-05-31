#ifndef PRIMITIVES_HPP
#define PRIMITIVES_HPP

#include "ast_base.hpp"
#include "symbol_base.hpp"
#include "interfaces.hpp"

#include <string>
#include <unordered_map>
#include <mutex>
#include <memory>

namespace HXSL
{
	class Primitive : public Type
	{
	private:
		std::vector<ast_ptr<OperatorOverload>> operators;
		PrimitiveKind kind;
		PrimitiveClass _class;
		std::string backingName;
		uint32_t rows;
		uint32_t columns;
	public:
		static constexpr NodeType ID = NodeType_Primitive;
		Primitive()
			: Type(TextSpan(), ID, TextSpan(), AccessModifier_Public),
			kind(PrimitiveKind_Void),
			_class(PrimitiveClass_Scalar),
			rows(0),
			columns(0)
		{
			this->name = backingName;
		}
		Primitive(TextSpan span, PrimitiveKind kind, PrimitiveClass _class, std::string& name, uint32_t rows, uint32_t columns)
			: Type(span, ID, name, AccessModifier_Public),
			kind(kind),
			_class(_class),
			backingName(std::move(name)),
			rows(rows),
			columns(columns)
		{
			this->name = backingName;
		}

		Primitive(PrimitiveKind kind, PrimitiveClass _class, std::string& name, uint32_t rows, uint32_t columns)
			: Type(TextSpan(), ID, name, AccessModifier_Public),
			kind(kind),
			_class(_class),
			backingName(std::move(name)),
			rows(rows),
			columns(columns)
		{
			this->name = backingName;
		}

		void AddOperator(ast_ptr<OperatorOverload> value) noexcept
		{
			RegisterChild(value);
			operators.push_back(std::move(value));
		}

		const std::vector<ast_ptr<OperatorOverload>>& GetOperators() const noexcept
		{
			return operators;
		}

		const uint32_t& GetRows() const noexcept
		{
			return rows;
		}

		void SetRows(const uint32_t& value)  noexcept
		{
			rows = value;
		}

		const uint32_t& GetColumns() const noexcept
		{
			return columns;
		}

		void SetColumns(const uint32_t& value)  noexcept
		{
			columns = value;
		}

		const PrimitiveKind& GetKind() const noexcept
		{
			return kind;
		}

		void SetKind(const PrimitiveKind& value)  noexcept
		{
			kind = value;
		}

		const PrimitiveClass& GetClass() const noexcept
		{
			return _class;
		}

		void SetClass(const PrimitiveClass& value)  noexcept
		{
			_class = value;
		}

		SymbolType GetSymbolType() const override
		{
			return SymbolType_Primitive;
		}

		size_t GetFieldOffset(Field* field) const override
		{
			return -1;
		}

		void Write(Stream& stream) const override
		{
			HXSL_ASSERT(false, "Cannot write primitive types")
		}

		void Read(Stream& stream, StringPool& container) override
		{
			HXSL_ASSERT(false, "Cannot read primitive types")
		}

		void Build(SymbolTable& table, size_t index, CompilationUnit* compilation, std::vector<ast_ptr<SymbolDef>>& nodes) override
		{
			HXSL_ASSERT(false, "Cannot build primitive types")
		}
	};
}
#endif