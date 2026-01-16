#ifndef PRIMITIVES_HPP
#define PRIMITIVES_HPP

#include "ast_base.hpp"
#include "symbol_base.hpp"
#include "interfaces.hpp"

namespace HXSL
{
	class Primitive : public Type, public TrailingObjects<Primitive, OperatorOverload*>
	{
		friend class ASTContext;
		friend class TrailingObjects;
	private:
		PrimitiveKind kind : 5;
		PrimitiveClass _class : 2;
		uint32_t rows;
		uint32_t columns;
		TrailingObjStorage<Primitive, uint32_t> storage;

		Primitive(const TextSpan& span, IdentifierInfo* name, PrimitiveKind kind, PrimitiveClass _class, uint32_t rows, uint32_t columns)
			: Type(span, ID, name, AccessModifier_Public),
			kind(kind),
			_class(_class),
			rows(rows),
			columns(columns)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_Primitive;
		static Primitive* Create(const TextSpan& span, IdentifierInfo* name, PrimitiveKind kind, PrimitiveClass _class, uint32_t rows, uint32_t columns, ArrayRef<OperatorOverload*>& operators);
		static Primitive* Create(const TextSpan& span, IdentifierInfo* name, PrimitiveKind kind, PrimitiveClass _class, uint32_t rows, uint32_t columns, uint32_t numOperators);

		DEFINE_TRAILING_OBJ_SPAN_GETTER(GetOperators, 0, storage)

		uint32_t GetRows() const noexcept { return rows; }
		void SetRows(const uint32_t& value) noexcept { rows = value; }

		uint32_t GetColumns() const noexcept { return columns; }
		void SetColumns(const uint32_t& value) noexcept { columns = value; }

		PrimitiveKind GetKind() const noexcept { return kind; }
		void SetKind(const PrimitiveKind& value) noexcept { kind = value; }

		PrimitiveClass GetClass() const noexcept { return _class; }
		void SetClass(const PrimitiveClass& value) noexcept { _class = value; }

		void ForEachChild(ASTChildCallback cb, void* userdata);
		void ForEachChild(ASTConstChildCallback cb, void* userdata) const;
	};
}
#endif