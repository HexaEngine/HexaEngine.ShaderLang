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
		PrimitiveKind_UInt,
		PrimitiveKind_Half,
		PrimitiveKind_Float,
		PrimitiveKind_Double,
		PrimitiveKind_Min8Float,
		PrimitiveKind_Min10Float,
		PrimitiveKind_Min16Float,
		PrimitiveKind_Min12Int,
		PrimitiveKind_Min16Int,
		PrimitiveKind_Min16UInt,
		PrimitiveKind_UInt8,
		PrimitiveKind_Int16,
		PrimitiveKind_UInt16,
		PrimitiveKind_Int64,
		PrimitiveKind_UInt64,
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
		case PrimitiveKind_Void: return "void";
		case PrimitiveKind_Bool: return "bool";
		case PrimitiveKind_Int: return "int";
		case PrimitiveKind_UInt: return "uint";
		case PrimitiveKind_Half: return "half";
		case PrimitiveKind_Float: return "float";
		case PrimitiveKind_Double: return "double";
		case PrimitiveKind_Min8Float: return "min8float";
		case PrimitiveKind_Min10Float: return "min10float";
		case PrimitiveKind_Min16Float: return "min16float";
		case PrimitiveKind_Min12Int: return "min12int";
		case PrimitiveKind_Min16Int: return "min16int";
		case PrimitiveKind_Min16UInt: return "min16uint";

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
		std::vector<ast_ptr<OperatorOverload>> operators;
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
			this->name = backingName;
		}
		Primitive(TextSpan span, PrimitiveKind kind, PrimitiveClass _class, std::string& name, uint32_t rows, uint32_t columns)
			: Type(span, NodeType_Primitive, name),
			ASTNode(span, NodeType_Primitive),
			kind(kind),
			_class(_class),
			backingName(std::move(name)),
			rows(rows),
			columns(columns)
		{
			this->name = backingName;
		}

		Primitive(PrimitiveKind kind, PrimitiveClass _class, std::string& name, uint32_t rows, uint32_t columns)
			: Type(TextSpan(), NodeType_Primitive, name),
			ASTNode(TextSpan(), NodeType_Primitive),
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