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
	// The primitive root type.
	enum PrimitiveKind
	{
		PrimitiveKind_Void,
		PrimitiveKind_Bool,
		PrimitiveKind_Int,
		PrimitiveKind_Float,
		PrimitiveKind_Uint,
		PrimitiveKind_Double,
		PrimitiveKind_Min8Float,
		PrimitiveKind_Min10Float,
		PrimitiveKind_Min16Float,
		PrimitiveKind_Min12Int,
		PrimitiveKind_Min16Int,
		PrimitiveKind_Min16Uint,
		PrimitiveKind_Uint8,
		PrimitiveKind_Int16,
		PrimitiveKind_Uint16,
		PrimitiveKind_Float16,
		PrimitiveKind_Int64,
		PrimitiveKind_Uint64,
	};

	static PrimitiveKind& operator++(PrimitiveKind& kind)
	{
		kind = static_cast<PrimitiveKind>(static_cast<int>(kind) + 1);
		return kind;
	}

	static std::string ToString(PrimitiveKind kind)
	{
		switch (kind)
		{
		case PrimitiveKind_Void:
			return "void";
			break;
		case PrimitiveKind_Bool:
			return "bool";
			break;
		case PrimitiveKind_Int:
			return "int";
			break;
		case PrimitiveKind_Float:
			return "float";
			break;
		case PrimitiveKind_Uint:
			return "uint";
			break;
		case PrimitiveKind_Double:
			return "double";
			break;
		case PrimitiveKind_Min8Float:
			return "min8float";
			break;
		case PrimitiveKind_Min10Float:
			return "min10float";
			break;
		case PrimitiveKind_Min16Float:
			return "min16float";
			break;
		case PrimitiveKind_Min12Int:
			return "min12int";
			break;
		case PrimitiveKind_Min16Int:
			return "min16int";
			break;
		case PrimitiveKind_Min16Uint:
			return "min16uint";
			break;
		default:
			return "";
			break;
		}
	}

	enum PrimitiveClass
	{
		PrimitiveClass_Scalar = 0,
		PrimitiveClass_Vector = 1,
		PrimitiveClass_Matrix = 2,
	};

	class Primitive : public Type, public IHasOperatorOverloads
	{
	private:
		std::vector<std::unique_ptr<OperatorOverload>> operators;
		PrimitiveKind kind;
		PrimitiveClass _class;
		std::string backingName;
		uint32_t rows;
		uint32_t columns;
	public:
		Primitive()
			: Type(TextSpan(), NodeType_Primitive, TextSpan()),
			ASTNode(TextSpan(), NodeType_Primitive),
			kind(PrimitiveKind_Void),
			_class(PrimitiveClass_Scalar),
			rows(0),
			columns(0)
		{
			this->name = TextSpan(backingName);
		}
		Primitive(TextSpan span, PrimitiveKind kind, PrimitiveClass _class, std::string& name, uint32_t rows, uint32_t columns)
			: Type(span, NodeType_Primitive, TextSpan()),
			ASTNode(span, NodeType_Primitive),
			kind(kind),
			_class(_class),
			backingName(std::move(name)),
			rows(rows),
			columns(columns)
		{
			this->name = TextSpan(backingName);
		}

		Primitive(PrimitiveKind kind, PrimitiveClass _class, std::string& name, uint32_t rows, uint32_t columns)
			: Type(name, NodeType_Primitive, TextSpan()),
			ASTNode(name, NodeType_Primitive),
			kind(kind),
			_class(_class),
			backingName(std::move(name)),
			rows(rows),
			columns(columns)
		{
			this->name = TextSpan(backingName);
		}

		void AddOperator(std::unique_ptr<OperatorOverload> value) noexcept
		{
			RegisterChild(value);
			operators.push_back(std::move(value));
		}

		const std::vector<std::unique_ptr<OperatorOverload>>& GetOperators() const noexcept
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

		void Write(Stream& stream) const override
		{
			HXSL_ASSERT(false, "Cannot write primitive types")
		}

		void Read(Stream& stream, StringPool& container) override
		{
			HXSL_ASSERT(false, "Cannot read primitive types")
		}

		void Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<SymbolDef>>& nodes) override
		{
			HXSL_ASSERT(false, "Cannot build primitive types")
		}
	};
}
#endif