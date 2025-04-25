#ifndef AST_HPP
#define AST_HPP

#include "config.h"
#include "vector.h"
#include "text_span.h"
#include "lexical/token.h"
#include "lang/language.h"
#include "stream.h"
#include "assembly.hpp"
#include "symbols/symbol_handle.hpp"

#include <memory>
#include <vector>
#include <deque>
#include <stack>
#include <functional>
#include <algorithm>
#include <limits>
#include <shared_mutex>

#define DEFINE_GETTER_SETTER(type, name, field)    \
    const type& Get##name() const noexcept { return field; } \
    void Set##name(const type &value) noexcept { field = value; }

#define DEFINE_GETTER_SETTER_PTR(type, name, field)    \
    type Get##name() const noexcept { return field; } \
    void Set##name(type value) noexcept { field = value; }

#define DEFINE_GET_SET_MOVE(type, name, field)    \
    const type& Get##name() const noexcept { return field; } \
    void Set##name(type&& value) noexcept { field = std::move(value); }

namespace HXSL
{
	class Compilation;
	class FunctionOverload;
	class OperatorOverload;
	class Struct;
	class Class;
	class Field;
	class Expression;
	class Statement;
	class LiteralExpression;
	class SymbolResolver;

	class SymbolTable;
	class SymbolMetadata;
	class StringPool;
	struct SymbolRef;

	/// <summary>
	/// Parsing: All Done.
	/// Symbol Collection: All Done.
	/// Symbol Resolve: All Done.
	/// Type Check: WIP.
	/// </summary>
	enum NodeType
	{
		/// <summary>
		/// Special node and does not exist as class.
		/// </summary>
		NodeType_Unknown,
		NodeType_Compilation,
		NodeType_Namespace,
		NodeType_Enum, // Placeholder (Will be added in the future.)
		NodeType_Primitive,
		NodeType_Struct,
		NodeType_Class, // Needs parsing logic
		NodeType_Array, // Not implemented yet. (but array accessor is.)
		NodeType_Field,
		NodeType_IntrinsicFunction, // Placeholder (Will be added in the future.)
		NodeType_FunctionOverload,
		NodeType_OperatorOverload,
		NodeType_Constructor, // Placeholder (Will be added in the future.)
		NodeType_Parameter,
		NodeType_AttributeDeclaration,
		NodeType_BlockStatement,
		NodeType_DeclarationStatement,
		NodeType_AssignmentStatement,
		NodeType_CompoundAssignmentStatement,
		NodeType_FunctionCallStatement,
		NodeType_ReturnStatement, // type-check: yes
		NodeType_IfStatement,
		NodeType_ElseStatement,
		NodeType_ElseIfStatement,
		NodeType_WhileStatement,
		NodeType_ForStatement,
		NodeType_BreakStatement,
		NodeType_ContinueStatement,
		NodeType_DiscardStatement,
		NodeType_SwitchStatement,
		NodeType_CaseStatement,
		NodeType_DefaultCaseStatement,
		NodeType_SwizzleDefinition,
		NodeType_Expression,
		NodeType_BinaryExpression, // type-check: yes
		NodeType_EmptyExpression,
		NodeType_LiteralExpression, // type-check: yes
		NodeType_MemberReferenceExpression, // type-check: yes
		NodeType_FunctionCallExpression, // type-check: yes
		NodeType_FunctionCallParameter, // type-check: yes
		NodeType_MemberAccessExpression, // type-check: yes
		NodeType_ComplexMemberAccessExpression, // type-check: yes
		NodeType_IndexerAccessExpression,
		NodeType_CastExpression,
		NodeType_TernaryExpression,
		NodeType_UnaryExpression, // type-check: yes
		NodeType_PrefixExpression,
		NodeType_PostfixExpression,
		NodeType_AssignmentExpression,
		NodeType_CompoundAssignmentExpression,
		NodeType_InitializationExpression,
		NodeType_Count,
	};

	static bool IsDataType(NodeType nodeType)
	{
		switch (nodeType) {
		case NodeType_Primitive:
		case NodeType_Struct:
		case NodeType_Class:
			return true;
		default:
			return false;
		}
	}

	static bool IsExpressionType(NodeType nodeType)
	{
		switch (nodeType) {
		case NodeType_Expression:
		case NodeType_BinaryExpression:
		case NodeType_EmptyExpression:
		case NodeType_LiteralExpression:
		case NodeType_MemberReferenceExpression:
		case NodeType_FunctionCallExpression:
		case NodeType_FunctionCallParameter:
		case NodeType_MemberAccessExpression:
		case NodeType_IndexerAccessExpression:
		case NodeType_CastExpression:
		case NodeType_TernaryExpression:
		case NodeType_UnaryExpression:
		case NodeType_PrefixExpression:
		case NodeType_PostfixExpression:
		case NodeType_AssignmentExpression:
		case NodeType_CompoundAssignmentExpression:
		case NodeType_InitializationExpression:
			return true;
		default:
			return false;
		}
	}

	static bool IsStatementType(NodeType nodeType)
	{
		switch (nodeType)
		{
		case NodeType_BlockStatement:
		case NodeType_DeclarationStatement:
		case NodeType_AssignmentStatement:
		case NodeType_CompoundAssignmentStatement:
		case NodeType_FunctionCallStatement:
		case NodeType_ReturnStatement:
		case NodeType_IfStatement:
		case NodeType_ElseStatement:
		case NodeType_ElseIfStatement:
		case NodeType_WhileStatement:
		case NodeType_ForStatement:
		case NodeType_BreakStatement:
		case NodeType_ContinueStatement:
		case NodeType_DiscardStatement:
		case NodeType_SwitchStatement:
		case NodeType_CaseStatement:
		case NodeType_DefaultCaseStatement:
			return true;
		default:
			return false;
		}
	}

	static std::string ToString(NodeType nodeType)
	{
		switch (nodeType)
		{
		case NodeType_Unknown: return "NodeType_Unknown";
		case NodeType_Compilation: return "NodeType_Compilation";
		case NodeType_Namespace: return "NodeType_Namespace";
		case NodeType_Enum: return "NodeType_Enum";
		case NodeType_Primitive: return "NodeType_Primitive";
		case NodeType_Struct: return "NodeType_Struct";
		case NodeType_Class: return "NodeType_Class";
		case NodeType_Field: return "NodeType_Field";
		case NodeType_IntrinsicFunction: return "NodeType_IntrinsicFunction";
		case NodeType_FunctionOverload: return "NodeType_Function";
		case NodeType_OperatorOverload: return "NodeType_Operator";
		case NodeType_Constructor: return "NodeType_Constructor";
		case NodeType_Parameter: return "NodeType_Parameter";
		case NodeType_AttributeDeclaration: return "NodeType_AttributeDeclaration";
		case NodeType_Expression: return "NodeType_Expression";
		case NodeType_BinaryExpression: return "NodeType_BinaryExpression";
		case NodeType_EmptyExpression: return "NodeType_EmptyExpression";
		case NodeType_LiteralExpression: return "NodeType_ConstantExpression";
		case NodeType_MemberReferenceExpression: return "NodeType_SymbolRefExpression";
		case NodeType_FunctionCallExpression: return "NodeType_FunctionCallExpression";
		case NodeType_FunctionCallParameter: return "NodeType_FunctionCallParameter";
		case NodeType_MemberAccessExpression: return "NodeType_MemberAccessExpression";
		case NodeType_IndexerAccessExpression: return "NodeType_IndexerAccessExpression";
		case NodeType_BlockStatement: return "NodeType_BlockStatement";
		case NodeType_DeclarationStatement: return "NodeType_DeclarationStatement";
		case NodeType_AssignmentStatement: return "NodeType_AssignmentStatement";
		case NodeType_CompoundAssignmentStatement: return "NodeType_CompoundAssignmentStatement";
		case NodeType_FunctionCallStatement: return "NodeType_FunctionCallStatement";
		case NodeType_ReturnStatement: return "NodeType_ReturnStatement";
		case NodeType_IfStatement: return "NodeType_IfStatement";
		case NodeType_ElseStatement: return "NodeType_ElseStatement";
		case NodeType_ElseIfStatement: return "NodeType_ElseIfStatement";
		case NodeType_WhileStatement: return "NodeType_WhileStatement";
		case NodeType_ForStatement: return "NodeType_ForStatement";
		case NodeType_BreakStatement: return "NodeType_BreakStatement";
		case NodeType_ContinueStatement: return "NodeType_ContinueStatement";
		case NodeType_DiscardStatement: return "NodeType_DiscardStatement";
		case NodeType_SwitchStatement: return "NodeType_SwitchStatement";
		case NodeType_CaseStatement: return "NodeType_CaseStatement";
		case NodeType_DefaultCaseStatement: return "NodeType_DefaultCaseStatement";
		case NodeType_CastExpression: return "NodeType_CastExpression";
		case NodeType_TernaryExpression: return "NodeType_TernaryExpression";
		case NodeType_UnaryExpression: return "NodeType_UnaryExpression";
		case NodeType_PrefixExpression: return "NodeType_PrefixExpression";
		case NodeType_PostfixExpression: return "NodeType_PostfixExpression";
		case NodeType_AssignmentExpression: return "NodeType_AssignmentExpression";
		case NodeType_CompoundAssignmentExpression: return "NodeType_CompoundAssignmentExpression";
		case NodeType_InitializationExpression: return "NodeType_InitializationExpression";
		default: return "Unknown NodeType";
		}
	}

	class ASTNode
	{
	private:
		Compilation* compilation = nullptr;
		size_t id = 0;
#ifdef DEBUG
		bool parentMissing = false;
#endif
		void AddChild(ASTNode* node)
		{
			HXSL_ASSERT(node, "Child node was null");
			children.push_back(node);
		}

		void RemoveChild(ASTNode* node)
		{
			HXSL_ASSERT(node, "Child node was null");
			children.erase(remove(children.begin(), children.end(), node), children.end());
		}

		bool MustHaveParent() const
		{
			return type != NodeType_Compilation && type != NodeType_Primitive && type != NodeType_IntrinsicFunction && type != NodeType_AttributeDeclaration && type != NodeType_SwizzleDefinition && !isExtern;
		}

	protected:
		std::vector<ASTNode*> children;
		TextSpan span;
		ASTNode* parent;
		NodeType type;
		bool isExtern;

		void AssignId();

	public:
		ASTNode(TextSpan span, ASTNode* parent, NodeType type, bool isExtern = false) : span(span), parent(parent), type(type), children({}), isExtern(isExtern)
		{
			//HXSL_ASSERT(parent, "Parent cannot be null");
			if (parent)
			{
				parent->AddChild(this);
				AssignId();
			}
			else
			{
				if (!MustHaveParent()) return;
#ifdef DEBUG
				parentMissing = true;
				std::cout << "[Warn]: (AST) Parent of node is null: " << ToString(type) << " Span: " << span.toString() << std::endl;
#endif
			}
		}

		virtual	Compilation* GetCompilation()
		{
			if (compilation)
			{
				return compilation;
			}
			if (parent)
			{
				compilation = parent->GetCompilation();
				return compilation;
			}
			return nullptr;
		}

		const size_t& GetID() const noexcept { return id; }

		bool IsExtern() const
		{
			return isExtern;
		}

		ASTNode* GetParent() noexcept { return parent; }
		void SetParent(ASTNode* newParent) noexcept
		{
			if (parent == newParent) return;

			if (parent)
			{
				parent->RemoveChild(this);
			}
			else if (parentMissing)
			{
#ifdef DEBUG
				std::cout << "[Info]: (AST) Recovered from Parent of node is null: " << ToString(type) << " Span: " << span.toString() << std::endl;
				parentMissing = false;
#endif
			}
			parent = newParent;
			if (newParent)
			{
				newParent->AddChild(this);
				AssignId();
			}
		}

		const std::vector<ASTNode*>& GetChildren() const noexcept { return children; }
		const NodeType& GetType() const noexcept { return type; }
		const TextSpan& GetSpan() const noexcept { return span; }
		void SetSpan(TextSpan newSpan) noexcept { span = newSpan; }
		bool IsAnyTypeOf(const std::unordered_set<NodeType>& types) const
		{
			return types.find(type) != types.end();
		}

		bool IsTypeOf(const NodeType& _type) const noexcept
		{
			return _type == type;
		}

		ASTNode* FindAncestors(const std::unordered_set<NodeType>& types, size_t maxDepth = std::numeric_limits<size_t>::max()) const noexcept
		{
			ASTNode* current = this->parent;
			size_t depth = 0;

			while (current && depth < maxDepth)
			{
				if (current->IsAnyTypeOf(types))
				{
					return current;
				}
				current = current->parent;
				++depth;
			}
			return nullptr;
		}

		ASTNode* FindAncestor(const NodeType& type, size_t maxDepth = std::numeric_limits<size_t>::max()) const noexcept
		{
			ASTNode* current = this->parent;
			size_t depth = 0;

			while (current && depth < maxDepth)
			{
				if (current->IsTypeOf(type))
				{
					return current;
				}
				current = current->parent;
				++depth;
			}
			return nullptr;
		}

		template<typename T>
		T* FindAncestor(const NodeType& type, size_t maxDepth = std::numeric_limits<size_t>::max()) const noexcept
		{
			auto node = FindAncestor(type, maxDepth);
			if (!node)
			{
				return nullptr;
			}
			return dynamic_cast<T*>(node);
		}

		virtual	std::string DebugName() const
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] ID: " << id << " Span: " + span.toString();
			return oss.str();
		}

		template <typename T>
		T* As() { return dynamic_cast<T*>(this); };
		virtual ~ASTNode()
		{
			if (parent)
			{
				parent->RemoveChild(this);
				parent = nullptr;
			}
#ifdef DEBUG
			else if (parentMissing)
			{
				std::cout << "[Info]: (AST) Recovered from Parent is null (Destroyed): " << ToString(type) << " Span: " << span.toString() << std::endl;
			}
#endif
		}
	};

	class ITypeCheckable
	{
	public:
		virtual void TypeCheck(SymbolResolver& resolver) = 0;
		virtual ~ITypeCheckable() = default;
	};

	enum SymbolType
	{
		SymbolType_Unknown,

		// Not an actual symbol type, but used for symbol resolving in scopes.
		SymbolType_Scope,

		SymbolType_Namespace,
		SymbolType_Struct,
		SymbolType_Class,
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
	protected:
		std::unique_ptr<std::string> fullyQualifiedName;
		TextSpan name;
		std::vector<SymbolRef*> references;
		const Assembly* assembly;
		SymbolHandle symbolHandle;

		SymbolDef(TextSpan span, ASTNode* parent, NodeType type, TextSpan name, bool isExtern = false)
			: ASTNode(span, parent, type, isExtern),
			name(name),
			assembly(nullptr)
		{
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

		const TextSpan& GetName() const
		{
			return name;
		}

		void SetName(const TextSpan& name)
		{
			this->name = name;
		}

		virtual bool IsConstant() const { return false; }

		virtual SymbolType GetSymbolType() const = 0;

		virtual void Write(Stream& stream) const = 0;

		virtual void Read(Stream& stream, StringPool& container) = 0;

		virtual void Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<SymbolDef>>& nodes) = 0;

		virtual ~SymbolDef() = default;
	};

	class Container : virtual public ASTNode
	{
	protected:
		std::vector<std::unique_ptr<FunctionOverload>> functions;
		std::vector<std::unique_ptr<OperatorOverload>> operators;
		std::vector<std::unique_ptr<Struct>> structs;
		std::vector<std::unique_ptr<Class>> classes;
		std::vector<std::unique_ptr<Field>> fields;

	public:
		Container(TextSpan span, ASTNode* parent, NodeType type, bool isExtern = false)
			:ASTNode(span, parent, type, isExtern)
		{
		}
		virtual ~Container() {}
		void AddFunction(std::unique_ptr<FunctionOverload> function);

		void AddOperator(std::unique_ptr<OperatorOverload> _operator);

		void AddStruct(std::unique_ptr<Struct> _struct);

		void AddClass(std::unique_ptr<Class> _class);

		void AddField(std::unique_ptr<Field> field);

		const std::vector<std::unique_ptr<FunctionOverload>>& GetFunctions() const noexcept
		{
			return functions;
		}

		const std::vector<std::unique_ptr<OperatorOverload>>& GetOperators() const noexcept
		{
			return operators;
		}

		const std::vector<std::unique_ptr<Struct>>& GetStructs() const noexcept
		{
			return structs;
		}

		const std::vector<std::unique_ptr<Class>>& GetClasses() const noexcept
		{
			return classes;
		}

		const std::vector<std::unique_ptr<Field>>& GetFields() const noexcept
		{
			return fields;
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
		SymbolRefType_Enum,
		SymbolRefType_Identifier,
		SymbolRefType_Attribute,
		SymbolRefType_Member,
		SymbolRefType_Type,
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

	class Type : public SymbolDef
	{
	private:
		Type() = delete;
	public:
		Type(TextSpan span, ASTNode* parent, NodeType type, TextSpan name, bool isExtern = false) : SymbolDef(span, parent, type, name, isExtern)
		{
		}
		virtual ~Type() {}
	};

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

	enum PrimitiveClass
	{
		PrimitiveClass_Scalar = 0,
		PrimitiveClass_Vector = 1,
		PrimitiveClass_Matrix = 2,
	};

	class Primitive : public Type
	{
	private:
		std::vector<std::unique_ptr<OperatorOverload>> operators;
		PrimitiveKind kind;
		PrimitiveClass _class;
		std::string backingName;
		uint rows;
		uint columns;
	public:
		Primitive()
			: Type(TextSpan(), nullptr, NodeType_Primitive, TextSpan()),
			ASTNode(TextSpan(), nullptr, NodeType_Primitive),
			kind(PrimitiveKind_Void),
			_class(PrimitiveClass_Scalar),
			rows(0),
			columns(0)
		{
			this->name = TextSpan(backingName);
		}
		Primitive(TextSpan span, ASTNode* parent, PrimitiveKind kind, PrimitiveClass _class, std::string& name, uint rows, uint columns)
			: Type(span, parent, NodeType_Primitive, TextSpan()),
			ASTNode(span, parent, NodeType_Primitive),
			kind(kind),
			_class(_class),
			backingName(std::move(name)),
			rows(rows),
			columns(columns)
		{
			this->name = TextSpan(backingName);
		}

		Primitive(PrimitiveKind kind, PrimitiveClass _class, std::string& name, uint rows, uint columns)
			: Type(name, nullptr, NodeType_Primitive, TextSpan()),
			ASTNode(name, nullptr, NodeType_Primitive),
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
			operators.push_back(std::move(value));
		}

		const std::vector<std::unique_ptr<OperatorOverload>>& GetOperators() const noexcept
		{
			return operators;
		}

		const uint& GetRows() const noexcept
		{
			return rows;
		}

		void SetRows(const uint& value)  noexcept
		{
			rows = value;
		}

		const uint& GetColumns() const noexcept
		{
			return columns;
		}

		void SetColumns(const uint& value)  noexcept
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

	struct SymbolRef
	{
	private:
		std::unique_ptr<std::string> fullyQualifiedName;
		TextSpan span;
		SymbolRefType type;
		SymbolHandle symbolHandle;
		std::vector<size_t> arrayDims;
		bool isDeferred;
		bool notFound;

	public:
		SymbolRef(TextSpan span, SymbolRefType type, bool isFullyQualified) : span(span), type(type), symbolHandle({}), isDeferred(false), notFound(false)
		{
			if (isFullyQualified)
			{
				fullyQualifiedName = std::make_unique<std::string>(span.toString());
			}
		}
		SymbolRef() : span({}), type(SymbolRefType_Unknown), symbolHandle({}), isDeferred(false), notFound(false)
		{
		}

		bool HasFullyQualifiedName() const noexcept { return fullyQualifiedName.get() != nullptr; }

		const SymbolRefType& GetType() const noexcept { return type; }

		void OverwriteType(const SymbolRefType& value) noexcept { type = value; }

		const TextSpan& GetName() const noexcept
		{
			return span;
		}

		bool IsArray() const noexcept { return !arrayDims.empty(); }

		const std::vector<size_t>& GetArrayDims() const noexcept { return arrayDims; }

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

		void Write(Stream& stream) const;

		void Read(Stream& stream);

		void SetDeferred(bool value) noexcept { isDeferred = value; }

		const bool& IsDeferred() const noexcept { return isDeferred; }
	};

	class IHasSymbolRef
	{
	public:
		virtual const std::unique_ptr<SymbolRef>& GetSymbolRef() = 0;
		virtual ~IHasSymbolRef() = default;
	};

	struct LazySymbol
	{
		TextSpan span;
		SymbolRefType type;
		bool fqn;
		std::unique_ptr<SymbolRef> ptr;

		LazySymbol() : span({}), type(SymbolRefType_Unknown), fqn(false)
		{
		}

		LazySymbol(std::unique_ptr<SymbolRef> ptr) : span({}), type(SymbolRefType_Unknown), ptr(std::move(ptr)), fqn(false)
		{
		}

		LazySymbol(TextSpan span, SymbolRefType type, bool fqn) : span(span), type(type), fqn(fqn)
		{
		}

		std::unique_ptr<SymbolRef> make(SymbolRefType overwrite)
		{
			if (ptr.get())
			{
				return std::move(ptr);
			}

			if (span.Text == nullptr)
			{
				HXSL_ASSERT(false, "Attempted to double create a LazySymbol");
				return {};
			}
			auto result = std::make_unique<SymbolRef>(span, overwrite, fqn);
			span = {};
			type = {};
			return std::move(result);
		};

		std::unique_ptr<SymbolRef> make()
		{
			return std::move(make(type));
		};
	};

	class SwizzleDefinition : virtual public ASTNode, public SymbolDef, public IHasSymbolRef
	{
	private:
		TextSpan expression;
		std::unique_ptr<SymbolRef> symbol;
	public:
		SwizzleDefinition(TextSpan expression, std::unique_ptr<SymbolRef> symbol)
			: ASTNode(expression, nullptr, NodeType_SwizzleDefinition),
			SymbolDef(expression, nullptr, NodeType_SwizzleDefinition, expression),
			expression(expression),
			symbol(std::move(symbol))
		{
		}

		const std::unique_ptr<SymbolRef>& GetSymbolRef()
		{
			return symbol;
		}

		SymbolType GetSymbolType() const override
		{
			return SymbolType_Field;
		}

		void Write(Stream& stream) const override
		{
			HXSL_ASSERT(false, "Cannot write swizzle types")
		}

		void Read(Stream& stream, StringPool& container) override
		{
			HXSL_ASSERT(false, "Cannot read swizzle types")
		}

		void Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<SymbolDef>>& nodes) override
		{
			HXSL_ASSERT(false, "Cannot build swizzle types")
		}
	};

	class IHasExpressions
	{
	private:
		std::vector<Expression*> expressions;

	protected:
		void RegisterExpression(Expression* expr)
		{
			if (expr == nullptr) return;
			expressions.push_back(expr);
		}

		void UnregisterExpression(Expression* expr)
		{
			if (expr == nullptr) return;
			expressions.erase(remove(expressions.begin(), expressions.end(), expr), expressions.end());
		}

	public:
		const std::vector<Expression*>& GetExpressions() const noexcept
		{
			return expressions;
		}

		virtual ~IHasExpressions() = default;
	};

#define DEFINE_GET_SET_MOVE_REG_EXPR(type, name, field)    \
    const type& Get##name() const noexcept { return field; } \
    void Set##name(type&& value) noexcept { UnregisterExpression(field.get()); field = std::move(value); RegisterExpression(field.get()); }

	class AttributeDeclaration : public ASTNode, public IHasExpressions
	{
	private:
		std::unique_ptr<SymbolRef> symbol;
		std::vector<std::unique_ptr<Expression>> parameters;
	public:
		AttributeDeclaration(TextSpan span, ASTNode* parent, std::unique_ptr<SymbolRef> symbol, std::vector<std::unique_ptr<Expression>> parameters)
			: ASTNode(span, parent, NodeType_AttributeDeclaration),
			symbol(std::move(symbol)),
			parameters(std::move(parameters))
		{
			for (auto& param : this->parameters)
			{
				RegisterExpression(param.get());
			}
		}
		AttributeDeclaration(TextSpan span, ASTNode* parent)
			: ASTNode(span, parent, NodeType_AttributeDeclaration)
		{
		}

		std::unique_ptr<SymbolRef>& GetSymbolRef()
		{
			return symbol;
		}

		DEFINE_GET_SET_MOVE(std::unique_ptr<SymbolRef>, Symbol, symbol)

		const std::vector<std::unique_ptr<Expression>>& GetParameters() const noexcept
		{
			return parameters;
		}

		void SetParameters(std::vector<std::unique_ptr<Expression>> value) noexcept
		{
			for (auto& param : parameters)
			{
				UnregisterExpression(param.get());
			}
			parameters = std::move(value);
			for (auto& param : parameters)
			{
				RegisterExpression(param.get());
			}
		}
	};

	class AttributeContainer
	{
	protected:
		std::vector<std::unique_ptr<AttributeDeclaration>> attributes;
	public:
		virtual ~AttributeContainer() {}
		void AddAttribute(std::unique_ptr<AttributeDeclaration> attribute)
		{
			attribute->SetParent(dynamic_cast<ASTNode*>(this));
			attributes.push_back(std::move(attribute));
		}

		const std::vector<std::unique_ptr<AttributeDeclaration>>& GetAttributes() const noexcept
		{
			return attributes;
		}
	};

	class Parameter : virtual public ASTNode, public SymbolDef, public IHasSymbolRef
	{
	private:
		ParameterFlags flags;
		InterpolationModifier interpolationModifiers;
		std::unique_ptr<SymbolRef> symbol;
		TextSpan semantic;

	public:
		Parameter()
			: SymbolDef(TextSpan(), nullptr, NodeType_Parameter, TextSpan(), true),
			ASTNode(TextSpan(), nullptr, NodeType_Parameter, true),
			flags(ParameterFlags_In),
			interpolationModifiers(InterpolationModifier_Linear),
			semantic(TextSpan())
		{
		}
		Parameter(TextSpan span, ASTNode* parent, ParameterFlags flags, InterpolationModifier interpolationModifiers, std::unique_ptr<SymbolRef> symbol, TextSpan name, TextSpan semantic)
			: SymbolDef(span, parent, NodeType_Parameter, name),
			ASTNode(span, parent, NodeType_Parameter),
			flags(flags),
			interpolationModifiers(interpolationModifiers),
			symbol(std::move(symbol)),
			semantic(semantic)
		{
		}

		void SetSymbolRef(std::unique_ptr<SymbolRef> ref)
		{
			symbol = std::move(ref);
		}

		std::unique_ptr<SymbolRef>& GetSymbolRef() override
		{
			return symbol;
		}

		SymbolType GetSymbolType() const override
		{
			return SymbolType_Parameter;
		}

		void Write(Stream& stream) const override;

		void Read(Stream& stream, StringPool& container) override;

		void Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<SymbolDef>>& nodes) override;

		~Parameter() override
		{
		}
	};

	class Statement : virtual public ASTNode
	{
	protected:
		Statement(TextSpan span, ASTNode* parent, NodeType type)
			: ASTNode(span, parent, type)
		{
		}
	};

	class StatementContainer
	{
	protected:
		std::vector<std::unique_ptr<Statement>> statements;

	public:
		virtual ~StatementContainer() = default;

		void AddStatement(std::unique_ptr<Statement> statement)
		{
			statements.push_back(std::move(statement));
		}

		const std::vector<std::unique_ptr<Statement>>& GetStatements() const
		{
			return statements;
		}
	};

	class BlockStatement : public Statement, public StatementContainer
	{
	public:
		BlockStatement(TextSpan span, ASTNode* parent)
			: Statement(span, parent, NodeType_BlockStatement),
			ASTNode(span, parent, NodeType_BlockStatement)
		{
		}

		std::string DebugName() const override
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] ID: " << GetID();
			return oss.str();
		}
	};

	struct ExpressionTraits
	{
		bool IsConstant = false;

		ExpressionTraits()
		{
		}

		ExpressionTraits(bool isConstant) : IsConstant(isConstant)
		{
		}
	};

	class Expression : public ASTNode
	{
	private:
		SymbolDef* inferredType;
		size_t lazyEvalState;
		ExpressionTraits traits;
	protected:
		Expression(TextSpan span, ASTNode* parent, NodeType type)
			: ASTNode(span, parent, type),
			lazyEvalState(0),
			inferredType(nullptr)
		{
		}

	public:

		const ExpressionTraits& GetTraits() const noexcept
		{
			return traits;
		}

		void SetTraits(const ExpressionTraits& traits) noexcept
		{
			this->traits = traits;
		}

		SymbolDef* GetInferredType() const noexcept
		{
			return inferredType;
		}

		void SetInferredType(SymbolDef* def) noexcept
		{
			inferredType = def;
			ResetLazyEvalState();
		}

		const size_t& GetLazyEvalState() const noexcept { return lazyEvalState; }

		void SetLazyEvalState(const size_t& value) noexcept { lazyEvalState = value; }

		void IncrementLazyEvalState() noexcept { lazyEvalState++; }

		void ResetLazyEvalState() noexcept { lazyEvalState = 0; }
	};

	class IChainExpression
	{
	public:
		virtual ~IChainExpression() = default;

		virtual	void chain(std::unique_ptr<Expression> expression) = 0;

		virtual const std::unique_ptr<Expression>& chainNext() = 0;
	};

	class UnaryExpression : public Expression
	{
	private:
		std::unique_ptr<SymbolRef> operatorSymbol;
		Operator _operator;
		std::unique_ptr<Expression> operand;
	protected:
		UnaryExpression(TextSpan span, ASTNode* parent, NodeType type, Operator op, std::unique_ptr<Expression> operand)
			: Expression(span, parent, type),
			_operator(op),
			operand(std::move(operand))
		{
			if (this->operand) this->operand->SetParent(this);
		}

	public:

		std::string BuildOverloadSignature() const
		{
			std::ostringstream oss;
			oss << "operator" << ToString(_operator) << "(" << operand->GetInferredType()->GetFullyQualifiedName() << ")";
			return oss.str();
		}

		std::unique_ptr<SymbolRef>& GetOperatorSymbolRef()
		{
			return operatorSymbol;
		}

		DEFINE_GETTER_SETTER(Operator, Operator, _operator)

			DEFINE_GET_SET_MOVE(std::unique_ptr<Expression>, Operand, operand)
	};

	class PrefixExpression : public UnaryExpression
	{
	public:
		PrefixExpression(TextSpan span, ASTNode* parent, Operator op, std::unique_ptr<Expression> operand)
			: UnaryExpression(span, parent, NodeType_PrefixExpression, op, std::move(operand))
		{
		}
	};

	class PostfixExpression : public UnaryExpression
	{
	public:
		PostfixExpression(TextSpan span, ASTNode* parent, Operator op, std::unique_ptr<Expression> operand)
			: UnaryExpression(span, parent, NodeType_PostfixExpression, op, std::move(operand))
		{
		}
	};

	class BinaryExpression : public Expression, public IChainExpression
	{
	private:
		Operator _operator;
		std::unique_ptr<SymbolRef> operatorSymbol;
		std::unique_ptr<Expression> left;
		std::unique_ptr<Expression> right;
	public:
		BinaryExpression(TextSpan span, ASTNode* parent, Operator op, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
			: Expression(span, parent, NodeType_BinaryExpression),
			_operator(op),
			left(std::move(left)),
			right(std::move(right)),
			operatorSymbol(std::make_unique<SymbolRef>(TextSpan(), SymbolRefType_OperatorOverload, false))
		{
			if (this->left) this->left->SetParent(this);
			if (this->right) this->right->SetParent(this);
		}

		std::string BuildOverloadSignature() const
		{
			std::ostringstream oss;
			oss << "operator" << ToString(_operator) << "(" << left->GetInferredType()->GetFullyQualifiedName() << "," << right->GetInferredType()->GetFullyQualifiedName() << ")";
			return oss.str();
		}

		std::unique_ptr<SymbolRef>& GetOperatorSymbolRef()
		{
			return operatorSymbol;
		}

		void chain(std::unique_ptr<Expression> expression) override
		{
			right = std::move(expression);
			right->SetParent(this);
		}

		virtual const std::unique_ptr<Expression>& chainNext() override
		{
			return right;
		}

		DEFINE_GETTER_SETTER(Operator, Operator, _operator)

			DEFINE_GET_SET_MOVE(std::unique_ptr<Expression>, Left, left)

			DEFINE_GET_SET_MOVE(std::unique_ptr<Expression>, Right, right)
	};

	class CastExpression : public Expression
	{
	private:
		std::unique_ptr<SymbolRef> operatorSymbol;
		std::unique_ptr<Expression> typeExpression;
		std::unique_ptr<Expression> operand;
	public:
		CastExpression(TextSpan span, ASTNode* parent, std::unique_ptr<Expression> typeExpression, std::unique_ptr<Expression> operand)
			: Expression(span, parent, NodeType_CastExpression),
			typeExpression(std::move(typeExpression)),
			operand(std::move(operand))
		{
			if (this->typeExpression) this->typeExpression->SetParent(this);
			if (this->operand) this->operand->SetParent(this);
		}

		std::string BuildOverloadSignature() const
		{
			std::ostringstream oss;
			oss << "operator" << "#" << typeExpression->GetInferredType()->GetFullyQualifiedName() << "(" << operand->GetInferredType()->GetFullyQualifiedName() << ")";
			return oss.str();
		}

		std::unique_ptr<SymbolRef>& GetOperatorSymbolRef()
		{
			return operatorSymbol;
		}

		DEFINE_GET_SET_MOVE(std::unique_ptr<Expression>, TypeExpression, typeExpression)

			DEFINE_GET_SET_MOVE(std::unique_ptr<Expression>, Operand, operand)
	};

	class TernaryExpression : public Expression
	{
	private:
		std::unique_ptr<Expression> condition;
		std::unique_ptr<Expression> left;
		std::unique_ptr<Expression> right;
	public:
		TernaryExpression(TextSpan span, ASTNode* parent, std::unique_ptr<Expression> condition, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
			: Expression(span, parent, NodeType_TernaryExpression),
			condition(std::move(condition)),
			left(std::move(left)),
			right(std::move(right))
		{
			if (this->left) this->left->SetParent(this);
			if (this->right) this->right->SetParent(this);
		}

		DEFINE_GET_SET_MOVE(std::unique_ptr<Expression>, Condition, condition)

			DEFINE_GET_SET_MOVE(std::unique_ptr<Expression>, Left, left)

			DEFINE_GET_SET_MOVE(std::unique_ptr<Expression>, Right, right)
	};

	class EmptyExpression : public Expression
	{
	public:
		EmptyExpression(TextSpan span, ASTNode* parent)
			: Expression(span, parent, NodeType_EmptyExpression)
		{
		}
	};

	class LiteralExpression : public Expression
	{
	private:
		Token literal;
	public:
		LiteralExpression(TextSpan span, ASTNode* parent, Token token)
			: Expression(span, parent, NodeType_LiteralExpression),
			literal(token)
		{
		}

		DEFINE_GETTER_SETTER(Token, Literal, literal)
	};

	class MemberReferenceExpression : public Expression, public IHasSymbolRef
	{
	private:
		std::unique_ptr<SymbolRef> symbol;
	public:
		MemberReferenceExpression(TextSpan span, ASTNode* parent, std::unique_ptr<SymbolRef> symbol)
			: Expression(span, parent, NodeType_MemberReferenceExpression),
			symbol(std::move(symbol))
		{
		}

		std::unique_ptr<SymbolRef>& GetSymbolRef() override
		{
			return symbol;
		}

		DEFINE_GET_SET_MOVE(std::unique_ptr<SymbolRef>, Symbol, symbol)
	};

	class FunctionCallParameter : public ASTNode
	{
	private:
		std::unique_ptr<Expression> expression;
	public:
		FunctionCallParameter(TextSpan span, ASTNode* parent, std::unique_ptr<Expression> expression)
			: ASTNode(span, parent, NodeType_FunctionCallParameter),
			expression(std::move(expression))
		{
			if (this->expression) this->expression->SetParent(this);
		}

		DEFINE_GET_SET_MOVE(std::unique_ptr<Expression>, Expression, expression)
	};

	class FunctionCallExpression : public Expression, public IHasSymbolRef
	{
	private:
		std::unique_ptr<SymbolRef> symbol;
		std::vector<std::unique_ptr<FunctionCallParameter>> parameters;
	public:
		FunctionCallExpression(TextSpan span, ASTNode* parent, std::unique_ptr<SymbolRef> symbol, std::vector<std::unique_ptr<FunctionCallParameter>> parameters)
			: Expression(span, parent, NodeType_FunctionCallExpression),
			symbol(std::move(symbol)),
			parameters(std::move(parameters))
		{
		}

		FunctionCallExpression(TextSpan span, ASTNode* parent, std::unique_ptr<SymbolRef> symbol)
			: Expression(span, parent, NodeType_FunctionCallExpression),
			symbol(std::move(symbol))
		{
		}

		bool CanBuildOverloadSignature()
		{
			for (auto& param : parameters)
			{
				if (!param->GetExpression()->GetInferredType())
				{
					return false;
				}
			}
			return false;
		}

		std::string BuildOverloadSignature()
		{
			std::ostringstream oss;
			oss << symbol->GetName().toString() << "(";
			bool first = true;
			for (auto& param : parameters)
			{
				if (!first)
				{
					oss << ",";
				}
				first = false;
				oss << param->GetExpression()->GetInferredType()->GetFullyQualifiedName();
			}
			oss << ")";
			return oss.str();
		}

		std::unique_ptr<SymbolRef>& GetSymbolRef() override
		{
			return symbol;
		}

		DEFINE_GET_SET_MOVE(std::unique_ptr<SymbolRef>, Symbol, symbol)

			DEFINE_GET_SET_MOVE(std::vector<std::unique_ptr<FunctionCallParameter>>, Parameters, parameters)
	};

	class MemberAccessExpression : public Expression, public IChainExpression, public IHasSymbolRef
	{
	private:
		std::unique_ptr<SymbolRef> symbol;
		std::unique_ptr<Expression> expression;
	public:
		MemberAccessExpression(TextSpan span, ASTNode* parent, std::unique_ptr<SymbolRef> symbol, std::unique_ptr<Expression> expression)
			: Expression(span, parent, NodeType_MemberAccessExpression),
			symbol(std::move(symbol)),
			expression(std::move(expression))
		{
		}

		std::unique_ptr<SymbolRef>& GetSymbolRef() override
		{
			return symbol;
		}

		void chain(std::unique_ptr<Expression> expression) override
		{
			this->expression = std::move(expression);
			this->expression->SetParent(this);
		}

		virtual const std::unique_ptr<Expression>& chainNext() override
		{
			return expression;
		}

		DEFINE_GET_SET_MOVE(std::unique_ptr<SymbolRef>, Symbol, symbol)

			DEFINE_GET_SET_MOVE(std::unique_ptr<Expression>, Expression, expression)
	};

	class ComplexMemberAccessExpression : public Expression, public IChainExpression, public IHasSymbolRef
	{
	private:
		std::unique_ptr<SymbolRef> symbol;
		std::unique_ptr<Expression> left;
		std::unique_ptr<Expression> right;
	public:
		ComplexMemberAccessExpression(TextSpan span, ASTNode* parent, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
			: Expression(span, parent, NodeType_ComplexMemberAccessExpression),
			symbol(std::make_unique<SymbolRef>()),
			left(std::move(left)),
			right(std::move(right))
		{
		}

		std::unique_ptr<SymbolRef>& GetSymbolRef() override
		{
			return symbol;
		}

		void chain(std::unique_ptr<Expression> expression) override
		{
			this->right = std::move(expression);
			this->right->SetParent(this);
			if (left && right)
			{
				span = left->GetSpan().merge(right->GetSpan());
			}
		}

		virtual const std::unique_ptr<Expression>& chainNext() override
		{
			return right;
		}

		DEFINE_GET_SET_MOVE(std::unique_ptr<SymbolRef>, Symbol, symbol)

			DEFINE_GET_SET_MOVE(std::unique_ptr<Expression>, Left, left)

			DEFINE_GET_SET_MOVE(std::unique_ptr<Expression>, Right, right)
	};

	class IndexerAccessExpression : public Expression
	{
	private:
		std::unique_ptr<SymbolRef> symbol;
		std::vector<std::unique_ptr<Expression>> indices;

	public:
		IndexerAccessExpression(TextSpan span, ASTNode* parent, std::unique_ptr<SymbolRef> symbol, std::vector<std::unique_ptr<Expression>> indices)
			: Expression(span, parent, NodeType_IndexerAccessExpression),
			symbol(std::move(symbol)),
			indices(std::move(indices))
		{
		}

		IndexerAccessExpression(TextSpan span, ASTNode* parent, std::unique_ptr<SymbolRef> symbol)
			: Expression(span, parent, NodeType_IndexerAccessExpression),
			symbol(std::move(symbol))
		{
		}

		void AddIndex(std::unique_ptr<Expression> expression)
		{
			expression->SetParent(this);
			indices.push_back(std::move(expression));
		}

		DEFINE_GET_SET_MOVE(std::vector<std::unique_ptr<Expression>>, Indices, indices)

			DEFINE_GET_SET_MOVE(std::unique_ptr<SymbolRef>, Symbol, symbol)
	};

	class AssignmentExpression : public Expression {
	private:
		std::unique_ptr<Expression> target;
		std::unique_ptr<Expression> expression;

	protected:
		AssignmentExpression(TextSpan span, ASTNode* parent, NodeType type, std::unique_ptr<Expression> target, std::unique_ptr<Expression> expression)
			: Expression(span, parent, type),
			target(std::move(target)),
			expression(std::move(expression))
		{
			if (this->target) this->target->SetParent(this);
			if (this->expression) this->expression->SetParent(this);
		}

	public:
		AssignmentExpression(TextSpan span, ASTNode* parent, std::unique_ptr<Expression> target, std::unique_ptr<Expression> expression)
			: Expression(span, parent, NodeType_AssignmentExpression),
			target(std::move(target)),
			expression(std::move(expression))
		{
			if (this->target) this->target->SetParent(this);
			if (this->expression) this->expression->SetParent(this);
		}

		DEFINE_GET_SET_MOVE(std::unique_ptr<Expression>, Target, target)

			DEFINE_GET_SET_MOVE(std::unique_ptr<Expression>, Expression, expression)
	};

	class CompoundAssignmentExpression : public AssignmentExpression {
	private:
		Operator _operator;

	public:
		CompoundAssignmentExpression(TextSpan span, ASTNode* parent, Operator op, std::unique_ptr<Expression> target, std::unique_ptr<Expression> expression)
			: AssignmentExpression(span, parent, NodeType_CompoundAssignmentExpression, std::move(target), std::move(expression)),
			_operator(op)
		{
		}

		DEFINE_GETTER_SETTER(Operator, Operator, _operator)
	};

	class InitializationExpression : public Expression
	{
	private:
		std::vector<std::unique_ptr<Expression>> parameters;

	public:
		InitializationExpression(TextSpan span, ASTNode* parent, std::vector<std::unique_ptr<Expression>> parameters)
			: Expression(span, parent, NodeType_InitializationExpression),
			parameters(std::move(parameters))
		{
		}
		InitializationExpression(TextSpan span, ASTNode* parent)
			: Expression(span, parent, NodeType_InitializationExpression)
		{
		}

		void AddParameter(std::unique_ptr<Expression> parameter)
		{
			parameters.push_back(std::move(parameter));
		}

		DEFINE_GET_SET_MOVE(std::vector<std::unique_ptr<Expression>>, Parameters, parameters)
	};

	class DeclarationStatement : public Statement, public SymbolDef, public IHasSymbolRef, public IHasExpressions
	{
	private:
		StorageClass storageClass;
		std::unique_ptr<SymbolRef> symbol;
		std::unique_ptr<Expression> initializer;

	public:
		DeclarationStatement()
			: Statement(TextSpan(), nullptr, NodeType_DeclarationStatement),
			SymbolDef(TextSpan(), nullptr, NodeType_DeclarationStatement, TextSpan(), true),
			ASTNode(TextSpan(), nullptr, NodeType_DeclarationStatement, true),
			storageClass(StorageClass_None)
		{
		}

		DeclarationStatement(TextSpan span, ASTNode* parent, std::unique_ptr<SymbolRef> symbol, StorageClass storageClass, TextSpan name, std::unique_ptr<Expression> initializer)
			: Statement(span, parent, NodeType_DeclarationStatement),
			SymbolDef(span, parent, NodeType_DeclarationStatement, name),
			ASTNode(span, parent, NodeType_DeclarationStatement),
			storageClass(storageClass),
			symbol(std::move(symbol)),
			initializer(std::move(initializer))
		{
			if (this->initializer)
			{
				this->initializer->SetParent(this);
				RegisterExpression(this->initializer.get());
			}
		}

		std::unique_ptr<SymbolRef>& GetSymbolRef() override
		{
			return symbol;
		}

		const StorageClass& GetStorageClass() const noexcept
		{
			return storageClass;
		}

		bool IsConstant() const override
		{
			return (storageClass & StorageClass_Const) != 0;
		}

		SymbolType GetSymbolType() const override
		{
			return SymbolType_Variable;
		}

		void Write(Stream& stream) const override;

		void Read(Stream& stream, StringPool& container) override;

		void Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<SymbolDef>>& nodes) override;

		DEFINE_GET_SET_MOVE(std::unique_ptr<SymbolRef>, Symbol, symbol)

			DEFINE_GET_SET_MOVE_REG_EXPR(std::unique_ptr<Expression>, Initializer, initializer)
	};

	class AssignmentStatement : public Statement, IHasExpressions
	{
	private:
		std::unique_ptr<Expression> target;
		std::unique_ptr<Expression> expression;

	protected:
		AssignmentStatement(TextSpan span, ASTNode* parent, NodeType type, std::unique_ptr<Expression> target, std::unique_ptr<Expression> expression)
			: Statement(span, parent, type),
			ASTNode(span, parent, type),
			target(std::move(target)),
			expression(std::move(expression))
		{
			if (this->target)
			{
				this->target->SetParent(this);
				RegisterExpression(this->target.get());
			}

			if (this->expression)
			{
				this->expression->SetParent(this);
				RegisterExpression(this->expression.get());
			}
		}

	public:
		AssignmentStatement(TextSpan span, ASTNode* parent, std::unique_ptr<Expression> target, std::unique_ptr<Expression> expression)
			: Statement(span, parent, NodeType_AssignmentStatement),
			ASTNode(span, parent, NodeType_AssignmentStatement),
			target(std::move(target)),
			expression(std::move(expression))
		{
			if (this->target)
			{
				this->target->SetParent(this);
				RegisterExpression(this->target.get());
			}

			if (this->expression)
			{
				this->expression->SetParent(this);
				RegisterExpression(this->expression.get());
			}
		}

		DEFINE_GET_SET_MOVE_REG_EXPR(std::unique_ptr<Expression>, Target, target)

			DEFINE_GET_SET_MOVE_REG_EXPR(std::unique_ptr<Expression>, Expression, expression)
	};

	class CompoundAssignmentStatement : public AssignmentStatement {
	private:
		Operator _operator;

	public:
		CompoundAssignmentStatement(TextSpan span, ASTNode* parent, Operator op, std::unique_ptr<Expression> target, std::unique_ptr<Expression> expression)
			: AssignmentStatement(span, parent, NodeType_CompoundAssignmentStatement, std::move(target), std::move(expression)),
			ASTNode(span, parent, NodeType_CompoundAssignmentStatement),
			_operator(op)
		{
		}

		DEFINE_GETTER_SETTER(Operator, Operator, _operator)
	};

	class FunctionCallStatement : public Statement, public IHasExpressions
	{
	private:
		std::unique_ptr<Expression> expression;

	public:
		FunctionCallStatement(TextSpan span, ASTNode* parent, std::unique_ptr<Expression> expression)
			: Statement(span, parent, NodeType_FunctionCallStatement),
			ASTNode(span, parent, NodeType_FunctionCallStatement),
			expression(std::move(expression))
		{
			if (this->expression)
			{
				this->expression->SetParent(this);
				RegisterExpression(this->expression.get());
			}
		}

		DEFINE_GET_SET_MOVE_REG_EXPR(std::unique_ptr<Expression>, Expression, expression)
	};

	class ReturnStatement : public Statement, public IHasExpressions, public ITypeCheckable
	{
	private:
		std::unique_ptr<Expression> returnValueExpression;

	public:
		ReturnStatement(TextSpan span, ASTNode* parent, std::unique_ptr<Expression> returnValueExpression)
			: Statement(span, parent, NodeType_ReturnStatement),
			ASTNode(span, parent, NodeType_ReturnStatement),
			returnValueExpression(std::move(returnValueExpression))
		{
			if (this->returnValueExpression)
			{
				this->returnValueExpression->SetParent(this);
				RegisterExpression(this->returnValueExpression.get());
			}
		}

		void TypeCheck(SymbolResolver& resolver) override;

		DEFINE_GET_SET_MOVE_REG_EXPR(std::unique_ptr<Expression>, ReturnValueExpression, returnValueExpression)
	};

	class IfStatement : public Statement, public AttributeContainer, public IHasExpressions
	{
	private:
		std::unique_ptr<Expression> expression;
		std::unique_ptr<BlockStatement> body;

	public:
		IfStatement(TextSpan span, ASTNode* parent, std::unique_ptr<Expression> expression, std::unique_ptr<BlockStatement> body)
			: Statement(span, parent, NodeType_IfStatement),
			ASTNode(span, parent, NodeType_IfStatement),
			expression(std::move(expression)),
			body(std::move(body))
		{
			if (this->expression)
			{
				this->expression->SetParent(this);
				RegisterExpression(this->expression.get());
			}
			if (this->body)
			{
				this->body->SetParent(this);
			}
		}

		DEFINE_GET_SET_MOVE_REG_EXPR(std::unique_ptr<Expression>, Expression, expression)

			DEFINE_GET_SET_MOVE(std::unique_ptr<BlockStatement>, Body, body)
	};

	class ElseStatement : public Statement
	{
	private:
		std::unique_ptr<BlockStatement> body;

	public:
		ElseStatement(TextSpan span, ASTNode* parent, std::unique_ptr<BlockStatement> body)
			: Statement(span, parent, NodeType_ElseStatement),
			ASTNode(span, parent, NodeType_ElseStatement),
			body(std::move(body))
		{
			if (this->body) this->body->SetParent(this);
		}

		DEFINE_GET_SET_MOVE(std::unique_ptr<BlockStatement>, Body, body)
	};

	class ElseIfStatement : public Statement, public IHasExpressions
	{
	private:
		std::unique_ptr<Expression> expression;
		std::unique_ptr<BlockStatement> body;

	public:
		ElseIfStatement(TextSpan span, ASTNode* parent, std::unique_ptr<Expression> expression, std::unique_ptr<BlockStatement> body)
			: Statement(span, parent, NodeType_ElseIfStatement),
			ASTNode(span, parent, NodeType_ElseIfStatement),
			expression(std::move(expression)),
			body(std::move(body))
		{
			if (this->expression)
			{
				this->expression->SetParent(this);
				RegisterExpression(this->expression.get());
			}
			if (this->body)
			{
				this->body->SetParent(this);
			}
		}

		DEFINE_GET_SET_MOVE_REG_EXPR(std::unique_ptr<Expression>, Expression, expression)

			DEFINE_GET_SET_MOVE(std::unique_ptr<BlockStatement>, Body, body)
	};

	class CaseStatement : public Statement, public StatementContainer, public IHasExpressions
	{
	private:
		std::unique_ptr<Expression> expression;
	public:
		CaseStatement(TextSpan span, ASTNode* parent, std::unique_ptr<Expression> expression)
			: Statement(span, parent, NodeType_CaseStatement),
			ASTNode(span, parent, NodeType_CaseStatement),
			expression(std::move(expression))
		{
			if (this->expression)
			{
				this->expression->SetParent(this);
				RegisterExpression(this->expression.get());
			}
		}

		DEFINE_GET_SET_MOVE_REG_EXPR(std::unique_ptr<Expression>, Expression, expression)
	};

	class DefaultCaseStatement : public Statement, public StatementContainer
	{
	private:

	public:
		DefaultCaseStatement(TextSpan span, ASTNode* parent)
			: Statement(span, parent, NodeType_DefaultCaseStatement),
			ASTNode(span, parent, NodeType_DefaultCaseStatement)
		{
		}
	};

	class SwitchStatement : public Statement, public AttributeContainer, public IHasExpressions
	{
	private:
		std::unique_ptr<Expression> expression;
		std::vector<std::unique_ptr<CaseStatement>> cases;
		std::unique_ptr<DefaultCaseStatement> defaultCase;
	public:
		SwitchStatement(TextSpan span, ASTNode* parent, std::unique_ptr<Expression> expression, std::vector<std::unique_ptr<CaseStatement>> cases, std::unique_ptr<DefaultCaseStatement> defaultCase)
			: Statement(span, parent, NodeType_SwitchStatement),
			ASTNode(span, parent, NodeType_SwitchStatement),
			expression(std::move(expression)),
			cases(std::move(cases)),
			defaultCase(std::move(defaultCase))
		{
			if (this->expression)
			{
				this->expression->SetParent(this);
				RegisterExpression(this->expression.get());
			}
		}
		SwitchStatement(TextSpan span, ASTNode* parent)
			: Statement(span, parent, NodeType_SwitchStatement),
			ASTNode(span, parent, NodeType_SwitchStatement)
		{
		}

		DEFINE_GET_SET_MOVE_REG_EXPR(std::unique_ptr<Expression>, Expression, expression)

			void AddCase(std::unique_ptr<CaseStatement> _case) { cases.push_back(std::move(_case)); }

		DEFINE_GET_SET_MOVE(std::unique_ptr<DefaultCaseStatement>, DefaultCase, defaultCase)
	};

	class ForStatement : public Statement, public AttributeContainer, public IHasExpressions
	{
	private:
		std::unique_ptr<Statement> init;
		std::unique_ptr<Expression> condition;
		std::unique_ptr<Expression> iteration;
		std::unique_ptr<BlockStatement> body;
	public:
		ForStatement(TextSpan span, ASTNode* parent, std::unique_ptr<Statement> init, std::unique_ptr<Expression> condition, std::unique_ptr<BlockStatement> body)
			: Statement(span, parent, NodeType_ForStatement),
			ASTNode(span, parent, NodeType_ForStatement),
			init(std::move(init)),
			condition(std::move(condition)),
			body(std::move(body))
		{
			if (this->condition)
			{
				this->condition->SetParent(this);
				RegisterExpression(this->condition.get());
			}
			if (this->iteration)
			{
				this->iteration->SetParent(this);
				RegisterExpression(this->iteration.get());
			}
		}
		ForStatement(TextSpan span, ASTNode* parent)
			: Statement(span, parent, NodeType_ForStatement),
			ASTNode(span, parent, NodeType_ForStatement)
		{
		}

		DEFINE_GET_SET_MOVE(std::unique_ptr<Statement>, Init, init)
			DEFINE_GET_SET_MOVE_REG_EXPR(std::unique_ptr<Expression>, Condition, condition)
			DEFINE_GET_SET_MOVE_REG_EXPR(std::unique_ptr<Expression>, Iteration, iteration)
			DEFINE_GET_SET_MOVE(std::unique_ptr<BlockStatement>, Body, body)
	};

	class BreakStatement : public Statement
	{
	public:
		BreakStatement(TextSpan span, ASTNode* parent)
			: Statement(span, parent, NodeType_BreakStatement),
			ASTNode(span, parent, NodeType_BreakStatement)
		{
		}
	};

	class ContinueStatement : public Statement
	{
	public:
		ContinueStatement(TextSpan span, ASTNode* parent)
			: Statement(span, parent, NodeType_ContinueStatement),
			ASTNode(span, parent, NodeType_ContinueStatement)
		{
		}
	};

	class DiscardStatement : public Statement
	{
	public:
		DiscardStatement(TextSpan span, ASTNode* parent)
			: Statement(span, parent, NodeType_DiscardStatement),
			ASTNode(span, parent, NodeType_DiscardStatement)
		{
		}
	};

	class WhileStatement : public Statement, public IHasExpressions, public AttributeContainer
	{
	private:
		std::unique_ptr<Expression> expression;
		std::unique_ptr<BlockStatement> body;

	public:
		WhileStatement(TextSpan span, ASTNode* parent, std::unique_ptr<Expression> expression, std::unique_ptr<BlockStatement> body)
			: Statement(span, parent, NodeType_WhileStatement),
			ASTNode(span, parent, NodeType_WhileStatement),
			expression(std::move(expression)),
			body(std::move(body))
		{
			if (this->expression)
			{
				this->expression->SetParent(this);
				RegisterExpression(this->expression.get());
			}
			if (this->body)
			{
				this->body->SetParent(this);
			}
		}

		DEFINE_GET_SET_MOVE_REG_EXPR(std::unique_ptr<Expression>, Expression, expression)

			DEFINE_GET_SET_MOVE(std::unique_ptr<BlockStatement>, Body, body)
	};

	class FunctionOverload : public SymbolDef, public AttributeContainer
	{
	protected:
		std::string cachedSignature;
		AccessModifier accessModifiers;
		FunctionFlags functionFlags;
		std::unique_ptr<SymbolRef> returnSymbol;
		std::vector<std::unique_ptr<Parameter>> parameters;
		TextSpan semantic;
		std::unique_ptr<BlockStatement> body;

		FunctionOverload(TextSpan span, ASTNode* parent, NodeType type, AccessModifier accessModifiers, FunctionFlags functionFlags, TextSpan name)
			: SymbolDef(span, parent, type, name),
			ASTNode(span, parent, type),
			accessModifiers(accessModifiers),
			functionFlags(functionFlags)
		{
		}
		FunctionOverload(TextSpan span, ASTNode* parent, NodeType type, AccessModifier accessModifiers, FunctionFlags functionFlags, TextSpan name, std::unique_ptr<SymbolRef> returnSymbol, std::vector<std::unique_ptr<Parameter>> parameters, TextSpan semantic)
			: SymbolDef(span, parent, type, name),
			ASTNode(span, parent, type),
			accessModifiers(accessModifiers),
			functionFlags(functionFlags),
			returnSymbol(std::move(returnSymbol)),
			parameters(std::move(parameters)),
			semantic(semantic)
		{
		}
		FunctionOverload(TextSpan span, ASTNode* parent, NodeType type, AccessModifier accessModifiers, FunctionFlags functionFlags, TextSpan name, std::unique_ptr<SymbolRef> returnSymbol)
			: SymbolDef(span, parent, type, name),
			ASTNode(span, parent, type),
			accessModifiers(accessModifiers),
			functionFlags(functionFlags),
			returnSymbol(std::move(returnSymbol))
		{
		}

	public:
		FunctionOverload()
			: SymbolDef(TextSpan(), nullptr, NodeType_FunctionOverload, TextSpan(), true),
			ASTNode(TextSpan(), nullptr, NodeType_FunctionOverload, true),
			accessModifiers(AccessModifier_Private),
			functionFlags(FunctionFlags_None),
			semantic(TextSpan())
		{
		}
		FunctionOverload(TextSpan span, ASTNode* parent, AccessModifier accessModifiers, FunctionFlags functionFlags, TextSpan name, std::unique_ptr<SymbolRef> returnSymbol, std::vector<std::unique_ptr<Parameter>> parameters, TextSpan semantic)
			: SymbolDef(span, parent, NodeType_FunctionOverload, name),
			ASTNode(span, parent, NodeType_FunctionOverload),
			accessModifiers(accessModifiers),
			functionFlags(functionFlags),
			returnSymbol(std::move(returnSymbol)),
			parameters(std::move(parameters)),
			semantic(semantic)
		{
		}

		FunctionOverload(TextSpan span, ASTNode* parent, AccessModifier accessModifiers, FunctionFlags functionFlags, TextSpan name, std::unique_ptr<SymbolRef> returnSymbol)
			: SymbolDef(span, parent, NodeType_FunctionOverload, name),
			ASTNode(span, parent, NodeType_FunctionOverload),
			accessModifiers(accessModifiers),
			functionFlags(functionFlags),
			returnSymbol(std::move(returnSymbol))
		{
		}

		void AddParameter(std::unique_ptr<Parameter> parameter)
		{
			parameter->SetParent(this);
			parameters.push_back(std::move(parameter));
		}

		std::unique_ptr<SymbolRef>& GetReturnSymbolRef()
		{
			return returnSymbol;
		}

		SymbolType GetSymbolType() const override
		{
			return SymbolType_Function;
		}

		virtual std::string BuildOverloadSignature(bool placeholder) noexcept
		{
			if (!placeholder && !cachedSignature.empty())
			{
				return cachedSignature;
			}

			std::ostringstream oss;
			oss << name.toString() << "(";
			bool first = true;
			for (auto& param : parameters)
			{
				if (!first)
				{
					oss << ",";
				}
				first = false;
				if (placeholder)
				{
					oss << param->GetID();
				}
				else
				{
					oss << param->GetSymbolRef()->GetFullyQualifiedName();
				}
			}
			oss << ")";

			if (placeholder)
			{
				return oss.str();
			}
			else
			{
				cachedSignature = oss.str();
				return cachedSignature;
			}
		}

		std::string BuildOverloadSignature() noexcept
		{
			return BuildOverloadSignature(false);
		}

		std::string BuildTemporaryOverloadSignature() noexcept
		{
			return BuildOverloadSignature(true);
		}

		void Write(Stream& stream) const override;

		void Read(Stream& stream, StringPool& container) override;

		void Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<SymbolDef>>& nodes) override;

		std::string DebugName() const override
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] ID: " << GetID() << " Name: " + name.toString();
			return oss.str();
		}
		DEFINE_GETTER_SETTER(AccessModifier, AccessModifiers, accessModifiers)
			DEFINE_GET_SET_MOVE(std::unique_ptr<SymbolRef>, ReturnSymbol, returnSymbol)
			DEFINE_GETTER_SETTER(TextSpan, Name, name)
			DEFINE_GET_SET_MOVE(std::vector<std::unique_ptr<Parameter>>, Parameters, parameters)
			DEFINE_GETTER_SETTER(TextSpan, Semantic, semantic)
			DEFINE_GET_SET_MOVE(std::unique_ptr<BlockStatement>, Body, body)
	};

	class OperatorOverload : public FunctionOverload
	{
	private:
		OperatorFlags operatorFlags;
		Operator _operator;
	public:
		OperatorOverload()
			: FunctionOverload(TextSpan(), nullptr, NodeType_OperatorOverload, AccessModifier_Private, FunctionFlags_None, TextSpan()),
			ASTNode(TextSpan(), nullptr, NodeType_OperatorOverload, true),
			_operator(Operator_Unknown),
			operatorFlags(OperatorFlags_None)
		{
		}
		OperatorOverload(TextSpan span, ASTNode* parent, AccessModifier accessModifiers, FunctionFlags functionFlags, OperatorFlags operatorFlags, TextSpan name, Operator _operator, std::unique_ptr<SymbolRef> returnSymbol, std::vector<std::unique_ptr<Parameter>> parameters, TextSpan semantic)
			: FunctionOverload(span, parent, NodeType_OperatorOverload, accessModifiers, functionFlags, name, std::move(returnSymbol), std::move(parameters), semantic),
			ASTNode(span, parent, NodeType_OperatorOverload),
			_operator(_operator),
			operatorFlags(operatorFlags)
		{
		}

		OperatorOverload(TextSpan span, ASTNode* parent, AccessModifier accessModifiers, FunctionFlags functionFlags, OperatorFlags operatorFlags, TextSpan name, Operator _operator)
			: FunctionOverload(span, parent, NodeType_OperatorOverload, accessModifiers, functionFlags, name),
			ASTNode(span, parent, NodeType_OperatorOverload),
			_operator(_operator),
			operatorFlags(operatorFlags)
		{
		}

		OperatorOverload(TextSpan span, ASTNode* parent, AccessModifier accessModifiers, FunctionFlags functionFlags, OperatorFlags operatorFlags, TextSpan name, Operator _operator, std::unique_ptr<SymbolRef> returnSymbol)
			: FunctionOverload(span, parent, NodeType_OperatorOverload, accessModifiers, functionFlags, name, std::move(returnSymbol)),
			ASTNode(span, parent, NodeType_OperatorOverload),
			_operator(_operator),
			operatorFlags(operatorFlags)
		{
		}

		std::string BuildOverloadSignature(bool placeholder) noexcept override
		{
			if (!placeholder && !cachedSignature.empty())
			{
				return cachedSignature;
			}

			std::ostringstream oss;
			oss << "operator";

			if (_operator != Operator_Cast)
			{
				oss << ToString(_operator);
			}
			else
			{
				if (placeholder)
				{
					oss << "#" << GetID();
				}
				else
				{
					oss << "#" << returnSymbol->GetFullyQualifiedName();
				}
			}

			oss << "(";
			bool first = true;
			for (auto& param : parameters)
			{
				if (!first)
				{
					oss << ",";
				}
				first = false;

				if (placeholder)
				{
					oss << param->GetID();
				}
				else
				{
					oss << param->GetSymbolRef()->GetFullyQualifiedName();
				}
			}
			oss << ")";

			if (placeholder)
			{
				return oss.str();
			}
			else
			{
				cachedSignature = oss.str();
				return cachedSignature;
			}
		}

		std::string BuildOverloadSignature() noexcept
		{
			return BuildOverloadSignature(false);
		}

		std::string BuildTemporaryOverloadSignature() noexcept
		{
			return BuildOverloadSignature(true);
		}

		SymbolType GetSymbolType() const override
		{
			return SymbolType_Operator;
		}

		const OperatorFlags& GetOperatorFlags() const noexcept { return operatorFlags; }

		void SetOperatorFlags(const OperatorFlags& value) noexcept { operatorFlags = value; }

		const Operator& GetOperator() const noexcept { return _operator; }

		void SetOperator(const Operator& value) noexcept { _operator = value; }

		void Write(Stream& stream) const override;

		void Read(Stream& stream, StringPool& container) override;

		void Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<SymbolDef>>& nodes) override;
	};

	class Field : virtual public ASTNode, public SymbolDef, public IHasSymbolRef
	{
	private:
		AccessModifier accessModifiers;
		StorageClass storageClass;
		InterpolationModifier interpolationModifiers;
		std::unique_ptr<SymbolRef> symbol;
		TextSpan semantic;
	public:
		Field()
			: SymbolDef(TextSpan(), nullptr, NodeType_Field, TextSpan(), true),
			ASTNode(TextSpan(), nullptr, NodeType_Field, true),
			accessModifiers(AccessModifier_Private),
			storageClass(StorageClass_None),
			interpolationModifiers(InterpolationModifier_Linear),
			semantic(TextSpan())
		{
		}
		Field(TextSpan span, ASTNode* parent, AccessModifier access, StorageClass storageClass, InterpolationModifier interpolationModifiers, TextSpan name, std::unique_ptr<SymbolRef> symbol, TextSpan semantic)
			: SymbolDef(span, parent, NodeType_Field, name),
			ASTNode(span, parent, NodeType_Field),
			accessModifiers(access),
			storageClass(storageClass),
			interpolationModifiers(interpolationModifiers),
			symbol(std::move(symbol)),
			semantic(semantic)
		{
		}

		void SetSymbolRef(std::unique_ptr<SymbolRef> ref)
		{
			symbol = std::move(ref);
		}

		std::unique_ptr<SymbolRef>& GetSymbolRef() override
		{
			return symbol;
		}

		const StorageClass& GetStorageClass() const noexcept
		{
			return storageClass;
		}

		bool IsConstant() const override
		{
			return (storageClass & StorageClass_Const) != 0;
		}

		SymbolType GetSymbolType() const override
		{
			return SymbolType_Field;
		}

		void Write(Stream& stream) const override;

		void Read(Stream& stream, StringPool& container) override;

		void Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<SymbolDef>>& nodes) override;

		DEFINE_GETTER_SETTER(AccessModifier, AccessModifiers, accessModifiers)
	};

	class Struct : public Type, public Container
	{
	private:
		AccessModifier accessModifiers;
	public:
		Struct()
			: Type(TextSpan(), nullptr, NodeType_Struct, TextSpan(), true),
			ASTNode(TextSpan(), nullptr, NodeType_Struct, true),
			Container(TextSpan(), nullptr, NodeType_Struct, true),
			accessModifiers(AccessModifier_Private)
		{
		}
		Struct(TextSpan span, ASTNode* parent, AccessModifier access, TextSpan name)
			: Type(span, parent, NodeType_Struct, name),
			ASTNode(span, parent, NodeType_Struct),
			Container(span, parent, NodeType_Struct),
			accessModifiers(access)
		{
		}

		std::string DebugName() const override
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] ID: " << GetID() << " Name: " + name.toString();
			return oss.str();
		}

		SymbolType GetSymbolType() const override
		{
			return SymbolType_Struct;
		}

		void Write(Stream& stream) const override;

		void Read(Stream& stream, StringPool& container) override;

		void Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<SymbolDef>>& nodes) override;

		DEFINE_GETTER_SETTER(AccessModifier, AccessModifiers, accessModifiers)
	};

	class Class : public Type, public Container
	{
	private:
		AccessModifier accessModifiers;
	public:
		Class()
			: Type(TextSpan(), nullptr, NodeType_Class, TextSpan(), true),
			ASTNode(TextSpan(), nullptr, NodeType_Class, true),
			Container(TextSpan(), nullptr, NodeType_Class, true),
			accessModifiers(AccessModifier_Private)
		{
		}
		Class(TextSpan span, ASTNode* parent, AccessModifier access, TextSpan name)
			: Type(span, parent, NodeType_Class, name),
			ASTNode(span, parent, NodeType_Class),
			Container(span, parent, NodeType_Class),
			accessModifiers(access)
		{
		}

		std::string DebugName() const override
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] ID: " << GetID() << " Name: " + name.toString();
			return oss.str();
		}

		SymbolType GetSymbolType() const override
		{
			return SymbolType_Struct;
		}

		void Write(Stream& stream) const override;

		void Read(Stream& stream, StringPool& container) override;

		void Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<SymbolDef>>& nodes) override;

		DEFINE_GETTER_SETTER(AccessModifier, AccessModifiers, accessModifiers)
	};

	class Array : public Type
	{
	};

	struct NamespaceDeclaration
	{
		TextSpan Span;
		TextSpan Name;

		NamespaceDeclaration() = default;
		NamespaceDeclaration(TextSpan span, TextSpan name) : Span(span), Name(name) {}
	};

	class AssemblyCollection;

	struct AssemblySymbolRef
	{
		Assembly* TargetAssembly;
		SymbolHandle LookupHandle;

		AssemblySymbolRef() = default;

		AssemblySymbolRef(Assembly* TargetAssembly, const SymbolHandle& lookupHandle)
			: TargetAssembly(TargetAssembly), LookupHandle(lookupHandle)
		{
		}
	};

	struct UsingDeclaration
	{
		TextSpan Span;
		TextSpan Target;
		TextSpan Alias;
		bool IsAlias;
		std::vector<AssemblySymbolRef> AssemblyReferences;

		UsingDeclaration() : IsAlias(false) {}
		UsingDeclaration(TextSpan span, TextSpan name) : Span(span), Target(name), Alias({}), IsAlias(false) {}
		UsingDeclaration(TextSpan span, TextSpan name, TextSpan alias) : Span(span), Target(name), Alias(alias), IsAlias(true) {}

		bool Warmup(const AssemblyCollection& references);
	};

	struct VariableReference
	{
		TextSpan Span;
		TextSpan PropertyName;
		TextSpan ShaderPropertyName;

		VariableReference() = default;
	};

	class Namespace : virtual public ASTNode, public Container, public SymbolDef
	{
	private:
		std::vector<UsingDeclaration> usings;
		std::vector<AssemblySymbolRef> references;
	public:
		Namespace()
			: SymbolDef(TextSpan(), nullptr, NodeType_Namespace, TextSpan(), true),
			ASTNode(TextSpan(), nullptr, NodeType_Namespace, true),
			Container(TextSpan(), nullptr, NodeType_Namespace, true)
		{
		}
		Namespace(ASTNode* parent, const NamespaceDeclaration& declaration)
			: SymbolDef(declaration.Span, parent, NodeType_Namespace, declaration.Name),
			ASTNode(declaration.Span, parent, NodeType_Namespace),
			Container(declaration.Span, parent, NodeType_Namespace)
		{
		}

		void AddUsing(UsingDeclaration _using)
		{
			usings.push_back(_using);
		}

		std::vector<UsingDeclaration>& GetUsings() { return usings; }

		const std::vector<AssemblySymbolRef>& GetAssemblyReferences() { return references; }

		virtual SymbolType GetSymbolType() const { return SymbolType_Namespace; }

		void Write(Stream& stream) const override;

		void Read(Stream& stream, StringPool& container) override;

		void Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<SymbolDef>>& nodes) override;

		void Warmup(const AssemblyCollection& references);
	};

	class Compilation : virtual public ASTNode, public ILogger
	{
	private:
		std::atomic<size_t> currentID = 0;
		std::vector<std::unique_ptr<Namespace>> namespaces;
		std::vector<UsingDeclaration> usings;

		std::vector<std::unique_ptr<Primitive>> primitives;
		std::vector<std::unique_ptr<Class>> primitiveClasses;

		std::shared_mutex _mutex;

		friend class PrimitiveManager;
		friend class PrimitiveBuilder;

	public:
		Compilation(bool isExtern = false)
			: ASTNode({ }, nullptr, NodeType_Compilation, isExtern), ILogger()
		{
			AssignId();
		}

		size_t GetNextID()
		{
			return currentID.fetch_add(1, std::memory_order_relaxed);
		}

		Compilation* GetCompilation() override
		{
			return this;
		}

		Namespace* AddNamespace(const NamespaceDeclaration& declaration)
		{
			std::shared_lock<std::shared_mutex> lock(_mutex);

			for (auto& ns : namespaces)
			{
				if (ns->GetName() == declaration.Name)
				{
					return ns.get();
				}
			}

			lock.unlock();
			std::unique_lock<std::shared_mutex> uniqueLock(_mutex);

			auto ns = std::make_unique<Namespace>(this, declaration);
			auto pNs = ns.get();
			namespaces.push_back(std::move(ns));
			return pNs;
		}

		void AddNamespace(std::unique_ptr<Namespace> ns)
		{
			HXSL_ASSERT(isExtern, "Cannot add namespace HXSL manually on non extern compilations.");
			ns->SetParent(this);
			namespaces.push_back(std::move(ns));
		}

		const std::vector<std::unique_ptr<Namespace>>& GetNamespaces() const noexcept
		{
			return namespaces;
		}

		void AddUsing(UsingDeclaration _using)
		{
			usings.push_back(_using);
		}

		void Clear()
		{
			usings.clear();
			namespaces.clear();
		}

		std::vector<UsingDeclaration>& GetUsings() noexcept { return usings; }
	};

	static std::unique_ptr<SymbolDef> CreateInstance(NodeType type)
	{
		switch (type)
		{
		case NodeType_Namespace:
			return std::make_unique<Namespace>();
		case NodeType_Field:
			return std::make_unique<Field>();
		case NodeType_FunctionOverload:
			return std::make_unique<FunctionOverload>();
		case NodeType_OperatorOverload:
			return std::make_unique<OperatorOverload>();
		case NodeType_Struct:
			return std::make_unique<Struct>();
		case NodeType_Parameter:
			return std::make_unique<Parameter>();
		case NodeType_DeclarationStatement:
			return std::make_unique<DeclarationStatement>();
		default:
			return nullptr;
		}
	}
}

#endif