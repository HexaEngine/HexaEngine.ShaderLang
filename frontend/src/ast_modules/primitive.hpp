#ifndef PRIMITIVES_HPP
#define PRIMITIVES_HPP

#include "ast_base.hpp"
#include "symbol_base.hpp"
#include "interfaces.hpp"

namespace HXSL
{
	class Primitive : public Type, TrailingObjects<Primitive, ast_ptr<OperatorOverload>>
	{
		friend class ASTContext;
		friend class TrailingObjects;
	private:
		PrimitiveKind kind : 5;
		PrimitiveClass _class : 2;
		uint32_t rows;
		uint32_t columns;
		uint32_t numOperators;

		Primitive(const TextSpan& span, IdentifierInfo* name, PrimitiveKind kind, PrimitiveClass _class, uint32_t rows, uint32_t columns)
			: Type(span, ID, name, AccessModifier_Public),
			numOperators(numOperators),
			kind(kind),
			_class(_class),
			rows(rows),
			columns(columns)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_Primitive;
		static Primitive* Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, PrimitiveKind kind, PrimitiveClass _class, uint32_t rows, uint32_t columns, ArrayRef<ast_ptr<OperatorOverload>>& operators);
		static Primitive* Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, PrimitiveKind kind, PrimitiveClass _class, uint32_t rows, uint32_t columns, uint32_t numOperators);

		ArrayRef<ast_ptr<OperatorOverload>> GetOperators() noexcept
		{
			return { GetTrailingObjects<0>(numOperators) , numOperators };
		}

		uint32_t GetRows() const noexcept { return rows; }
		void SetRows(const uint32_t& value) noexcept { rows = value; }

		uint32_t GetColumns() const noexcept { return columns; }
		void SetColumns(const uint32_t& value) noexcept { columns = value; }

		PrimitiveKind GetKind() const noexcept { return kind; }
		void SetKind(const PrimitiveKind& value) noexcept { kind = value; }

		PrimitiveClass GetClass() const noexcept { return _class; }
		void SetClass(const PrimitiveClass& value) noexcept { _class = value; }
	};
}
#endif