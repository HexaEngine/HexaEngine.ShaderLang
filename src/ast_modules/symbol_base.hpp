#ifndef SYMBOLS_BASE_HPP
#define SYMBOLS_BASE_HPP

#include "ast_base.hpp"
#include "semantics/symbols/symbol_handle.hpp"
#include "il/assembly.hpp"

namespace HXSL
{
	enum SymbolType
	{
		SymbolType_Unknown,

		// Not an actual symbol type, but used for symbol resolving in scopes.
		SymbolType_Scope,

		SymbolType_Namespace,
		SymbolType_Struct,
		SymbolType_Class,
		SymbolType_Array,
		SymbolType_Enum,
		SymbolType_Attribute,
		SymbolType_Primitive,
		SymbolType_Field,
		SymbolType_IntrinsicFunction,
		SymbolType_Function,
		SymbolType_Operator,
		SymbolType_Constructor,
		SymbolType_Parameter,
		SymbolType_Variable,
		SymbolType_Constant,
		SymbolType_Alias,
	};

	static std::string ToString(SymbolType type)
	{
		switch (type)
		{
		case SymbolType_Unknown:
			return "Unknown";
		case SymbolType_Scope:
			return "Scope";
		case SymbolType_Namespace:
			return "Namespace";
		case SymbolType_Struct:
			return "Struct";
		case SymbolType_Class:
			return "Class";
		case SymbolType_Enum:
			return "Enum";
		case SymbolType_Attribute:
			return "Attribute";
		case SymbolType_Primitive:
			return "Primitive";
		case SymbolType_Field:
			return "Field";
		case SymbolType_IntrinsicFunction:
			return "Intrinsic Function";
		case SymbolType_Function:
			return "Function";
		case SymbolType_Operator:
			return "Operator";
		case SymbolType_Constructor:
			return "Constructor";
		case SymbolType_Parameter:
			return "Parameter";
		case SymbolType_Variable:
			return "Variable";
		case SymbolType_Alias:
			return "Alias";
		default:
			return "Unknown";
		}
	}

	enum SymbolLookupResult
	{
		SymbolLookupResult_Success,
		SymbolLookupResult_NotFound,
		SymbolLookupResult_Ambiguous,
	};

	enum SymbolScopeType
	{
		SymbolScopeType_Global,
		SymbolScopeType_Namespace,
		SymbolScopeType_Struct,
		SymbolScopeType_Class,
		SymbolScopeType_Function,
		SymbolScopeType_Operator,
		SymbolScopeType_Block
	};

	class SymbolDef : virtual public ASTNode
	{
	private:
		SymbolDef() = delete;

		void UpdateName();
	protected:
		ast_ptr<std::string> fullyQualifiedName;
		std::string_view name;
		std::vector<SymbolRef*> references;
		const Assembly* assembly;
		SymbolHandle symbolHandle;

		SymbolDef(TextSpan span, NodeType type, TextSpan name, bool isExtern = false)
			: ASTNode(span, type, isExtern),
			assembly(nullptr)
		{
			fullyQualifiedName = make_ast_ptr<std::string>(name.str());
			UpdateName();
		}

		SymbolDef(TextSpan span, NodeType type, const std::string& name, bool isExtern = false)
			: ASTNode(span, type, isExtern),
			assembly(nullptr)
		{
			fullyQualifiedName = make_ast_ptr<std::string>(name);
			UpdateName();
		}
	public:
		void AddRef(SymbolRef* ref)
		{
			references.push_back(ref);
		}

		void RemoveRef(SymbolRef* ref)
		{
			references.erase(std::remove(references.begin(), references.end(), ref), references.end());
		}

		void SetAssembly(const Assembly* assembly, const SymbolHandle& handle);

		const Assembly* GetAssembly() const noexcept { return assembly; }

		const SymbolTable* GetTable() const noexcept { return assembly->GetSymbolTable(); }

		const SymbolHandle& GetSymbolHandle() const noexcept { return symbolHandle; }

		const std::string& GetFullyQualifiedName() const noexcept { return *fullyQualifiedName.get(); }

		bool IsEquivalentTo(const SymbolDef* other) const noexcept
		{
			return other != nullptr && this->GetFullyQualifiedName() == other->GetFullyQualifiedName();
		}

		const SymbolMetadata* GetMetadata() const;

		const std::string_view& GetName() const
		{
			return name;
		}

		void SetName(const std::string& name)
		{
			*fullyQualifiedName = name;
			UpdateName();
		}

		virtual bool IsConstant() const { return false; }

		virtual SymbolType GetSymbolType() const = 0;

		virtual void Write(Stream& stream) const = 0;

		virtual void Read(Stream& stream, StringPool& container) = 0;

		virtual void Build(SymbolTable& table, size_t index, CompilationUnit* compilation, std::vector<ast_ptr<SymbolDef>>& nodes) = 0;

		virtual ~SymbolDef() = default;

		ast_ptr<SymbolRef> MakeSymbolRef() const;

		std::string_view ToString() const noexcept
		{
			return name;
		}
	};

	enum SymbolRefType
	{
		SymbolRefType_Unknown,
		SymbolRefType_Namespace,
		SymbolRefType_FunctionOverload,
		SymbolRefType_OperatorOverload,
		SymbolRefType_Constructor,
		SymbolRefType_FunctionOrConstructor,
		SymbolRefType_Struct,
		SymbolRefType_Class,
		SymbolRefType_Enum,
		SymbolRefType_Identifier,
		SymbolRefType_Attribute,
		SymbolRefType_Member,
		SymbolRefType_Type,
		SymbolRefType_ArrayType,
		SymbolRefType_This,
		SymbolRefType_Any,
	};

	static bool operator==(SymbolRefType lhs, SymbolRefType rhs)
	{
		return static_cast<int>(lhs) == static_cast<int>(rhs);
	}

	static bool operator!=(SymbolRefType lhs, SymbolRefType rhs)
	{
		return !(lhs == rhs);
	}

	struct SymbolRef
	{
	private:
		ast_ptr<std::string> fullyQualifiedName;
		std::string_view name;
		bool isFullyQualified;
		TextSpan span;
		SymbolRefType type;
		SymbolHandle symbolHandle;
		std::vector<size_t> arrayDims;
		bool isDeferred;
		bool notFound;

		void UpdateName();

	public:
		SymbolRef(TextSpan span, SymbolRefType type, bool isFullyQualified) : span(span), type(type), symbolHandle({}), isDeferred(false), notFound(false), isFullyQualified(isFullyQualified)
		{
			fullyQualifiedName = make_ast_ptr<std::string>(span.str());
			UpdateName();
		}

		SymbolRef(std::string name, SymbolRefType type, bool isFullyQualified) : span(TextSpan()), type(type), symbolHandle({}), isDeferred(false), notFound(false), isFullyQualified(isFullyQualified)
		{
			fullyQualifiedName = make_ast_ptr<std::string>(name);
			UpdateName();
		}

		SymbolRef() : span({}), type(SymbolRefType_Unknown), symbolHandle({}), isDeferred(false), notFound(false), isFullyQualified(false)
		{
		}

		void TrimCastType();

		bool HasFullyQualifiedName() const noexcept { return isFullyQualified; }

		const SymbolRefType& GetType() const noexcept { return type; }

		void OverwriteType(const SymbolRefType& value) noexcept { type = value; }

		std::string MakeArrayTypeName(size_t dimIndex, SymbolDef* elementType = nullptr)
		{
			elementType = elementType ? elementType : GetBaseDeclaration();
			std::ostringstream oss;
			oss << elementType->GetFullyQualifiedName();
			for (size_t i = 0; i < dimIndex + 1; i++)
			{
				oss << "[";
				oss << arrayDims[i];
				oss << "]";
			}

			return oss.str();
		}

		const std::string_view& GetName() const noexcept
		{
			return name;
		}

		const TextSpan& GetSpan() const noexcept { return span; }

		bool IsArray() const noexcept { return !arrayDims.empty(); }

		void SetArrayDims(std::vector<size_t> dims) noexcept { arrayDims = std::move(dims); if (type == SymbolRefType_Type) type = SymbolRefType_ArrayType; }

		std::vector<size_t>& GetArrayDims() noexcept { return arrayDims; }

		size_t GetArrayDimCount() const noexcept { return arrayDims.size(); }

		bool IsResolved() const noexcept { return !symbolHandle.invalid(); }

		void SetTable(const SymbolHandle& handle);

		void SetDeclaration(const SymbolDef* node);

		void SetNotFound(bool value) noexcept { notFound = value; }

		const bool& IsNotFound() const noexcept { return notFound; }

		const SymbolHandle& GetSymbolHandle() const { return symbolHandle; }

		const std::string& GetFullyQualifiedName() const;

		const SymbolMetadata* GetMetadata() const;

		SymbolDef* GetDeclaration() const;

		SymbolDef* GetBaseDeclaration() const;

		SymbolDef* GetAncestorDeclaration(SymbolType type) const;

		void Write(Stream& stream) const;

		void Read(Stream& stream);

		void SetDeferred(bool value) noexcept { isDeferred = value; }

		const bool& IsDeferred() const noexcept { return isDeferred; }

		ast_ptr<SymbolRef> Clone() const
		{
			auto cloned = make_ast_ptr<SymbolRef>();
			if (fullyQualifiedName)
				cloned->fullyQualifiedName = make_ast_ptr<std::string>(*fullyQualifiedName);
			cloned->span = span;
			cloned->type = type;
			cloned->symbolHandle = symbolHandle;
			cloned->arrayDims = arrayDims;
			cloned->isDeferred = isDeferred;
			cloned->notFound = notFound;
			return cloned;
		}

		const std::string& ToString() const noexcept
		{
			return *fullyQualifiedName;
		}
	};

	class Type : public SymbolDef
	{
	private:
		Type() = delete;
	public:
		Type(TextSpan span, NodeType type, TextSpan name, bool isExtern = false) : SymbolDef(span, type, name, isExtern)
		{
		}

		Type(TextSpan span, NodeType type, const std::string& name, bool isExtern = false) : SymbolDef(span, type, name, isExtern)
		{
		}

		virtual size_t GetFieldOffset(Field* field) const = 0;

		virtual ~Type() {}
	};

	static SymbolRefType ConvertSymbolTypeToSymbolRefType(SymbolType type)
	{
		switch (type)
		{
		case SymbolType_Unknown:
			return SymbolRefType_Unknown;
		case SymbolType_Namespace:
			return SymbolRefType_Namespace;
		case SymbolType_Struct:
			return SymbolRefType_Struct;
		case SymbolType_Class:
			return SymbolRefType_Class;
		case SymbolType_Array:
			return SymbolRefType_ArrayType;
		case SymbolType_Enum:
			return SymbolRefType_Enum;
		case SymbolType_Attribute:
			return SymbolRefType_Attribute;
		case SymbolType_Primitive:
			return SymbolRefType_Type;
		case SymbolType_Field:
			return SymbolRefType_Member;
		case SymbolType_IntrinsicFunction:
			return SymbolRefType_FunctionOverload;
		case SymbolType_Function:
			return SymbolRefType_FunctionOverload;
		case SymbolType_Operator:
			return SymbolRefType_OperatorOverload;
		case SymbolType_Constructor:
			return SymbolRefType_Constructor;
		case SymbolType_Parameter:
			return SymbolRefType_Identifier;
		case SymbolType_Variable:
			return SymbolRefType_Identifier;
		case SymbolType_Constant:
			return SymbolRefType_Identifier;
		case SymbolType_Alias:
			return SymbolRefType_Identifier;
		default:
			return SymbolRefType_Unknown;
		}
	}
}

#endif