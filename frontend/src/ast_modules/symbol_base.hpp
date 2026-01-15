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
		SymbolType_Pointer,
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
		SymbolScopeType_Constructor,
		SymbolScopeType_Operator,
		SymbolScopeType_Block
	};

	class SymbolDef : public ASTNode
	{
	private:
		IdentifierInfo* identifer;
		IdentifierInfo* fullyQualifiedName = nullptr;
		const Assembly* assembly = nullptr;
		SymbolHandle symbolHandle;

	protected:

		SymbolDef(const TextSpan& span, NodeType type, IdentifierInfo* identifer, bool isExtern = false)
			: ASTNode(span, type, isExtern),
			identifer(identifer)
		{
		}

	public:
		void SetAssembly(const Assembly* assembly, const SymbolHandle& handle);

		void UpdateFQN();

		const Assembly* GetAssembly() const noexcept { return assembly; }

		const SymbolTable* GetTable() const noexcept { return assembly->GetSymbolTable(); }

		const SymbolHandle& GetSymbolHandle() const noexcept { return symbolHandle; }

		const StringSpan& GetName() const;

		const StringSpan& GetFullyQualifiedName() const noexcept;

		bool IsEquivalentTo(const SymbolDef* other) const noexcept
		{
			return other != nullptr && this->GetFullyQualifiedName() == other->GetFullyQualifiedName();
		}

		const SymbolMetadata* GetMetadata() const;

		bool IsConstant() const;

		SymbolType GetSymbolType() const;

		AccessModifier GetAccessModifiers() const;

		SymbolRef* MakeSymbolRef() const;

		const StringSpan& ToString() const noexcept;
	};

	enum SymbolRefType : uint8_t
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

	class SymbolRef
	{
		friend class ASTContext;
	private:
		SymbolRefType type : 4;
		bool isFullyQualified : 1;
		bool isDeferred : 1;
		bool notFound : 1;
		TextSpan span;
		IdentifierInfo* identifier;
		SymbolDef* def;

	protected:
		SymbolRef(const TextSpan& span, IdentifierInfo* identifier, SymbolRefType type, bool isFullyQualified) : span(span), identifier(identifier), type(type), def(nullptr), isDeferred(false), notFound(false), isFullyQualified(isFullyQualified)
		{
		}

	public:
		static SymbolRef* Create(const TextSpan& span, IdentifierInfo* identifer, SymbolRefType type, bool isFullyQualified);

		IdentifierInfo* GetIdentifier() const noexcept { return identifier; }
		
		bool HasFullyQualifiedName() const noexcept { return isFullyQualified; }

		const SymbolRefType& GetType() const noexcept { return type; }

		void OverwriteType(const SymbolRefType& value) noexcept { type = value; }

		StringSpan GetName() const noexcept;

		const TextSpan& GetSpan() const noexcept { return span; }

		bool IsResolved() const noexcept { return def != nullptr; }

		void SetTable(const SymbolHandle& handle);

		void SetDeclaration(const SymbolDef* node);

		void SetNotFound(bool value) noexcept { notFound = value; }

		bool IsNotFound() const noexcept { return notFound; }

		void TrimCastType();

		const SymbolHandle& GetSymbolHandle() const;

		const StringSpan& GetFullyQualifiedName() const;

		SymbolDef* GetDeclaration() const;

		SymbolDef* GetBaseDeclaration() const;

		SymbolDef* GetAncestorDeclaration(SymbolType type) const;

		void SetDeferred(bool value) noexcept { isDeferred = value; }

		const bool& IsDeferred() const noexcept { return isDeferred; }

		SymbolRef* Clone() const;

		const StringSpan& ToString() const noexcept;

		bool IsArrayType() const noexcept { return type == SymbolRefType_ArrayType; }
	};

	class SymbolRefArray : public SymbolRef, public TrailingObjects<SymbolRefArray, size_t>
	{
		friend class ASTContext;
	private:
		TrailingObjStorage<SymbolRefArray, size_t> storage;

		SymbolRefArray(const TextSpan& span, IdentifierInfo* name) : SymbolRef(span, name, ID, false)
		{
		}

	public:
		static constexpr SymbolRefType ID = SymbolRefType_ArrayType;
		static SymbolRefArray* Create(const TextSpan& span, IdentifierInfo* name, const Span<size_t>& arrayDims);
		static SymbolRefArray* Create(const TextSpan& span, IdentifierInfo* name, uint32_t numArrayDims);

		DEFINE_TRAILING_OBJ_SPAN_GETTER(GetArrayDims, 0, storage);

		std::string MakeArrayTypeName(size_t dimIndex, SymbolDef* elementType = nullptr)
		{
			elementType = elementType ? elementType : GetBaseDeclaration();
			std::ostringstream oss;
			oss << elementType->GetFullyQualifiedName();
			auto arrayDims = GetArrayDims();
			for (size_t i = 0; i < dimIndex + 1; i++)
			{
				oss << "[";
				oss << arrayDims[i];
				oss << "]";
			}

			return oss.str();
		}
	};

	class Type : public SymbolDef
	{
	protected:
		AccessModifier accessModifiers;
	private:
		Type() = delete;
	public:
		Type(TextSpan span, NodeType type, IdentifierInfo* name, AccessModifier accessModifiers, bool isExtern = false)
			: SymbolDef(span, type, name, isExtern),
			accessModifiers(accessModifiers)
		{
		}

		DEFINE_GETTER_SETTER(AccessModifier, AccessModifiers, accessModifiers)
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

	template<typename T>
	inline static bool isa(const SymbolRef* node) { return node && node->GetType() == T::ID; }

	template<typename T>
	inline static T* dyn_cast(SymbolRef* node) { return isa<T>(node) ? static_cast<T*>(node) : nullptr; }

	template<typename T>
	inline static const T* dyn_cast(const SymbolRef* node) { return isa<T>(node) ? static_cast<const T*>(node) : nullptr; }

	template <typename T>
	inline static T* cast(SymbolRef* N) { assert(isa<T>(N) && "cast<T>() argument is not of type T!"); return static_cast<T*>(N); }

	template <typename T>
	inline static const T* cast(const SymbolRef* N) { assert(isa<T>(N) && "cast<T>() argument is not of type T!"); return static_cast<const T*>(N); }
}

#endif