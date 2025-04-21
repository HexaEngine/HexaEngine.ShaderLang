#ifndef NODES_H
#define NODES_H

#include "config.h"
#include "vector.h"
#include "text_span.h"
#include "lexical/token.h"
#include "lang/language.h"
#include "stream.h"
#include "assembly.hpp"

#include <memory>
#include <vector>
#include <deque>
#include <stack>
#include <functional>
#include <algorithm>

namespace HXSL
{
	class Compilation;
	class HXSLFunction;
	class HXSLStruct;
	class HXSLClass;
	class HXSLField;

	class SymbolTable;
	class SymbolMetadata;
	class StringPool;
	struct HXSLSymbolRef;

	enum HXSLNodeType
	{
		HXSLNodeType_Unknown,
		HXSLNodeType_Symbol,
		HXSLNodeType_Compilation,
		HXSLNodeType_Namespace,
		HXSLNodeType_Enum, // Placeholder (Will be added in the future.)
		HXSLNodeType_Primitive,
		HXSLNodeType_Struct,
		HXSLNodeType_Class, // Placeholder (Will be added in the future.)
		HXSLNodeType_Field,
		HXSLNodeType_IntrinsicFunction, // Placeholder (Will be added in the future.)
		HXSLNodeType_Function,
		HXSLNodeType_Constructor, // Placeholder (Will be added in the future.)
		HXSLNodeType_Parameter,
		HXSLNodeType_AttributeDeclaration,
		HXSLNodeType_Expression,
		HXSLNodeType_BinaryExpression,
		HXSLNodeType_EmptyExpression,
		HXSLNodeType_LiteralExpression,
		HXSLNodeType_SymbolRefExpression,
		HXSLNodeType_FunctionCallExpression,
		HXSLNodeType_FunctionCallParameter,
		HXSLNodeType_MemberAccessExpression,
		HXSLNodeType_IndexerAccessExpression,
		HXSLNodeType_BlockStatement,
		HXSLNodeType_DeclarationStatement,
		HXSLNodeType_AssignmentStatement,
		HXSLNodeType_CompoundAssignmentStatement,
		HXSLNodeType_FunctionCallStatement,
		HXSLNodeType_ReturnStatement,
		HXSLNodeType_IfStatement,
		HXSLNodeType_ElseStatement,
		HXSLNodeType_ElseIfStatement,
		HXSLNodeType_WhileStatement,
		HXSLNodeType_ForStatement,
		HXSLNodeType_BreakStatement,
		HXSLNodeType_ContinueStatement,
		HXSLNodeType_DiscardStatement,
		HXSLNodeType_SwitchStatement,
		HXSLNodeType_CaseStatement,
		HXSLNodeType_DefaultCaseStatement,
		HXSLNodeType_CastExpression,
		HXSLNodeType_TernaryExpression,
		HXSLNodeType_UnaryExpression,
		HXSLNodeType_PrefixExpression,
		HXSLNodeType_PostfixExpression,
		HXSLNodeType_AssignmentExpression,
		HXSLNodeType_CompoundAssignmentExpression,
		HXSLNodeType_InitializationExpression,
		HXSLNodeType_SwizzleDefinition,
		HXSLNodeType_Array,
		//HXSLNodeType_ConstantExpression,
	};

	static bool IsDataType(HXSLNodeType nodeType)
	{
		switch (nodeType) {
		case HXSLNodeType_Primitive:
		case HXSLNodeType_Struct:
		case HXSLNodeType_Class:
			return true;
		default:
			return false;
		}
	}

	static bool IsExpressionType(HXSLNodeType nodeType)
	{
		switch (nodeType) {
		case HXSLNodeType_Expression:
		case HXSLNodeType_BinaryExpression:
		case HXSLNodeType_EmptyExpression:
		case HXSLNodeType_LiteralExpression:
		case HXSLNodeType_SymbolRefExpression:
		case HXSLNodeType_FunctionCallExpression:
		case HXSLNodeType_FunctionCallParameter:
		case HXSLNodeType_MemberAccessExpression:
		case HXSLNodeType_IndexerAccessExpression:
		case HXSLNodeType_CastExpression:
		case HXSLNodeType_TernaryExpression:
		case HXSLNodeType_UnaryExpression:
		case HXSLNodeType_PrefixExpression:
		case HXSLNodeType_PostfixExpression:
		case HXSLNodeType_AssignmentExpression:
		case HXSLNodeType_CompoundAssignmentExpression:
		case HXSLNodeType_InitializationExpression:
			return true;
		default:
			return false;
		}
	}

	static bool IsStatementType(HXSLNodeType nodeType)
	{
		switch (nodeType)
		{
		case HXSLNodeType_BlockStatement:
		case HXSLNodeType_DeclarationStatement:
		case HXSLNodeType_AssignmentStatement:
		case HXSLNodeType_CompoundAssignmentStatement:
		case HXSLNodeType_FunctionCallStatement:
		case HXSLNodeType_ReturnStatement:
		case HXSLNodeType_IfStatement:
		case HXSLNodeType_ElseStatement:
		case HXSLNodeType_ElseIfStatement:
		case HXSLNodeType_WhileStatement:
		case HXSLNodeType_ForStatement:
		case HXSLNodeType_BreakStatement:
		case HXSLNodeType_ContinueStatement:
		case HXSLNodeType_DiscardStatement:
		case HXSLNodeType_SwitchStatement:
		case HXSLNodeType_CaseStatement:
		case HXSLNodeType_DefaultCaseStatement:
			return true;
		default:
			return false;
		}
	}

	static std::string toString(HXSLNodeType nodeType)
	{
		switch (nodeType)
		{
		case HXSLNodeType_Unknown: return "HXSLNodeType_Unknown";
		case HXSLNodeType_Symbol: return "HXSLNodeType_Symbol";
		case HXSLNodeType_Compilation: return "HXSLNodeType_Compilation";
		case HXSLNodeType_Namespace: return "HXSLNodeType_Namespace";
		case HXSLNodeType_Enum: return "HXSLNodeType_Enum";
		case HXSLNodeType_Primitive: return "HXSLNodeType_Primitive";
		case HXSLNodeType_Struct: return "HXSLNodeType_Struct";
		case HXSLNodeType_Class: return "HXSLNodeType_Class";
		case HXSLNodeType_Field: return "HXSLNodeType_Field";
		case HXSLNodeType_IntrinsicFunction: return "HXSLNodeType_IntrinsicFunction";
		case HXSLNodeType_Function: return "HXSLNodeType_Function";
		case HXSLNodeType_Constructor: return "HXSLNodeType_Constructor";
		case HXSLNodeType_Parameter: return "HXSLNodeType_Parameter";
		case HXSLNodeType_AttributeDeclaration: return "HXSLNodeType_AttributeDeclaration";
		case HXSLNodeType_Expression: return "HXSLNodeType_Expression";
		case HXSLNodeType_BinaryExpression: return "HXSLNodeType_BinaryExpression";
		case HXSLNodeType_EmptyExpression: return "HXSLNodeType_EmptyExpression";
		case HXSLNodeType_LiteralExpression: return "HXSLNodeType_ConstantExpression";
		case HXSLNodeType_SymbolRefExpression: return "HXSLNodeType_SymbolRefExpression";
		case HXSLNodeType_FunctionCallExpression: return "HXSLNodeType_FunctionCallExpression";
		case HXSLNodeType_FunctionCallParameter: return "HXSLNodeType_FunctionCallParameter";
		case HXSLNodeType_MemberAccessExpression: return "HXSLNodeType_MemberAccessExpression";
		case HXSLNodeType_IndexerAccessExpression: return "HXSLNodeType_IndexerAccessExpression";
		case HXSLNodeType_BlockStatement: return "HXSLNodeType_BlockStatement";
		case HXSLNodeType_DeclarationStatement: return "HXSLNodeType_DeclarationStatement";
		case HXSLNodeType_AssignmentStatement: return "HXSLNodeType_AssignmentStatement";
		case HXSLNodeType_CompoundAssignmentStatement: return "HXSLNodeType_CompoundAssignmentStatement";
		case HXSLNodeType_FunctionCallStatement: return "HXSLNodeType_FunctionCallStatement";
		case HXSLNodeType_ReturnStatement: return "HXSLNodeType_ReturnStatement";
		case HXSLNodeType_IfStatement: return "HXSLNodeType_IfStatement";
		case HXSLNodeType_ElseStatement: return "HXSLNodeType_ElseStatement";
		case HXSLNodeType_ElseIfStatement: return "HXSLNodeType_ElseIfStatement";
		case HXSLNodeType_WhileStatement: return "HXSLNodeType_WhileStatement";
		case HXSLNodeType_ForStatement: return "HXSLNodeType_ForStatement";
		case HXSLNodeType_BreakStatement: return "HXSLNodeType_BreakStatement";
		case HXSLNodeType_ContinueStatement: return "HXSLNodeType_ContinueStatement";
		case HXSLNodeType_DiscardStatement: return "HXSLNodeType_DiscardStatement";
		case HXSLNodeType_SwitchStatement: return "HXSLNodeType_SwitchStatement";
		case HXSLNodeType_CaseStatement: return "HXSLNodeType_CaseStatement";
		case HXSLNodeType_DefaultCaseStatement: return "HXSLNodeType_DefaultCaseStatement";
		case HXSLNodeType_CastExpression: return "HXSLNodeType_CastExpression";
		case HXSLNodeType_TernaryExpression: return "HXSLNodeType_TernaryExpression";
		case HXSLNodeType_UnaryExpression: return "HXSLNodeType_UnaryExpression";
		case HXSLNodeType_PrefixExpression: return "HXSLNodeType_PrefixExpression";
		case HXSLNodeType_PostfixExpression: return "HXSLNodeType_PostfixExpression";
		case HXSLNodeType_AssignmentExpression: return "HXSLNodeType_AssignmentExpression";
		case HXSLNodeType_CompoundAssignmentExpression: return "HXSLNodeType_CompoundAssignmentExpression";
		case HXSLNodeType_InitializationExpression: return "HXSLNodeType_InitializationExpression";
		default: return "Unknown HXSLNodeType";
		}
	}

	class ASTNode
	{
	private:
		Compilation* m_compilation = nullptr;
		size_t ID;

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
			return type != HXSLNodeType_Compilation && type != HXSLNodeType_Primitive && type != HXSLNodeType_IntrinsicFunction && type != HXSLNodeType_AttributeDeclaration && type != HXSLNodeType_SwizzleDefinition && !isExtern;
		}

	protected:
		std::vector<ASTNode*> children;
		TextSpan Span;
		ASTNode* parent;
		HXSLNodeType type;
		bool isExtern;

		void AssignId();

	public:
		ASTNode(TextSpan span, ASTNode* parent, HXSLNodeType type, bool isExtern = false) : Span(span), parent(parent), type(type), children({}), isExtern(isExtern)
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
				std::cout << "[Warn]: (AST) Parent of node is null: " << toString(type) << " Span: " << span.toString() << std::endl;
#endif
			}
		}

		virtual	Compilation* GetCompilation()
		{
			if (m_compilation)
			{
				return m_compilation;
			}
			if (parent)
			{
				m_compilation = parent->GetCompilation();
				return m_compilation;
			}
			return nullptr;
		}

		const size_t& GetID() const noexcept { return ID; }

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
			else if (MustHaveParent())
			{
#ifdef DEBUG
				std::cout << "[Info]: (AST) Recovered from Parent of node is null: " << toString(type) << " Span: " << Span.toString() << std::endl;
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
		const HXSLNodeType& GetType() const noexcept { return type; }
		const TextSpan& GetSpan() const noexcept { return Span; }
		void SetSpan(TextSpan newSpan) noexcept { Span = newSpan; }
		bool IsAnyTypeOf(const std::unordered_set<HXSLNodeType>& types) const
		{
			return types.find(type) != types.end();
		}

		bool IsTypeOf(const HXSLNodeType& _type) const
		{
			return _type == type;
		}

		virtual	std::string DebugName() const
		{
			std::ostringstream oss;
			oss << "[" << toString(type) << "] ID: " << ID << " Span: " + Span.toString();
			return oss.str();
		}

		template <typename T>
		T* As() { return dynamic_cast<T*>(this); };
		virtual ~ASTNode()
		{
			if (parent)
			{
				parent->RemoveChild(this);
			}
		}
	};

	enum SymbolType
	{
		HXSLSymbolType_Unknown,

		// Not an actual symbol type, but used for symbol resolving in scopes.
		HXSLSymbolType_Scope,
		HXSLSymbolType_Namespace,
		HXSLSymbolType_Struct,
		HXSLSymbolType_Class,
		HXSLSymbolType_Enum,
		HXSLSymbolType_Attribute,
		HXSLSymbolType_Primitive,
		HXSLSymbolType_Field,
		HXSLSymbolType_IntrinsicFunction,
		HXSLSymbolType_Function,
		HXSLSymbolType_Constructor,
		HXSLSymbolType_Parameter,
		HXSLSymbolType_Variable,
		HXSLSymbolType_Alias,
	};

	static std::string ToString(SymbolType type)
	{
		switch (type)
		{
		case HXSLSymbolType_Unknown:
			return "Unknown";
		case HXSLSymbolType_Scope:
			return "Scope";
		case HXSLSymbolType_Namespace:
			return "Namespace";
		case HXSLSymbolType_Struct:
			return "Struct";
		case HXSLSymbolType_Class:
			return "Class";
		case HXSLSymbolType_Enum:
			return "Enum";
		case HXSLSymbolType_Attribute:
			return "Attribute";
		case HXSLSymbolType_Primitive:
			return "Primitive";
		case HXSLSymbolType_Field:
			return "Field";
		case HXSLSymbolType_IntrinsicFunction:
			return "Intrinsic Function";
		case HXSLSymbolType_Function:
			return "Function";
		case HXSLSymbolType_Constructor:
			return "Constructor";
		case HXSLSymbolType_Parameter:
			return "Parameter";
		case HXSLSymbolType_Variable:
			return "Variable";
		case HXSLSymbolType_Alias:
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

	enum HXSLSymbolScopeType
	{
		HXSLSymbolScopeType_Global,
		HXSLSymbolScopeType_Namespace,
		HXSLSymbolScopeType_Struct,
		HXSLSymbolScopeType_Class,
		HXSLSymbolScopeType_Function,
		HXSLSymbolScopeType_Block
	};

	class HXSLSymbolDef : virtual public ASTNode
	{
	private:
		HXSLSymbolDef() = delete;
	protected:
		std::unique_ptr<std::string> FullyQualifiedName;
		TextSpan Name;
		std::vector<HXSLSymbolRef*> references;
		const Assembly* m_assembly;
		size_t TableIndex;

		HXSLSymbolDef(TextSpan span, ASTNode* parent, HXSLNodeType type, TextSpan name, bool isExtern = false)
			: ASTNode(span, parent, type, isExtern),
			Name(name),
			TableIndex(0),
			m_assembly(nullptr)
		{
		}
	public:
		void AddRef(HXSLSymbolRef* ref)
		{
			references.push_back(ref);
		}

		void RemoveRef(HXSLSymbolRef* ref)
		{
			references.erase(std::remove(references.begin(), references.end(), ref), references.end());
		}

		void SetAssembly(const Assembly* assembly, size_t tableIndex);

		[[deprecated("Use SetAssembly instead of SetTable")]]
		void SetTable(const SymbolTable* symbolTable, size_t tableIndex)
		{
			HXSL_ASSERT(false, "Use of deprecated function.");
		}

		const Assembly* GetAssembly() const noexcept { return m_assembly; }

		const SymbolTable* GetTable() const noexcept { return m_assembly->GetSymbolTable(); }

		const size_t& GetTableIndex() const noexcept { return TableIndex; }

		const std::string& GetFullyQualifiedName() const noexcept { return *FullyQualifiedName.get(); }

		const SymbolMetadata* GetMetadata() const;

		const TextSpan& GetName() const
		{
			return Name;
		}

		void SetName(const TextSpan& name)
		{
			Name = name;
		}

		virtual SymbolType GetSymbolType() const = 0;

		virtual void Write(HXSLStream& stream) const = 0;

		virtual void Read(HXSLStream& stream, StringPool& container) = 0;

		virtual void Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<HXSLSymbolDef>>& nodes) = 0;

		virtual ~HXSLSymbolDef() = default;
	};

	class HXSLContainer : virtual public ASTNode
	{
	protected:
		std::vector<std::unique_ptr<HXSLFunction>> Functions;
		std::vector<std::unique_ptr<HXSLStruct>> Structs;
		std::vector<std::unique_ptr<HXSLClass>> Classes;
		std::vector<std::unique_ptr<HXSLField>> Fields;

	public:
		HXSLContainer(TextSpan span, ASTNode* parent, HXSLNodeType type, bool isExtern = false)
			:ASTNode(span, parent, type, isExtern)
		{
		}
		virtual ~HXSLContainer() {}
		void AddFunction(std::unique_ptr<HXSLFunction> function);

		void AddStruct(std::unique_ptr<HXSLStruct> _struct);

		void AddClass(std::unique_ptr<HXSLClass> _class);

		void AddField(std::unique_ptr<HXSLField> field);

		const std::vector<std::unique_ptr<HXSLFunction>>& GetFunctions() const noexcept
		{
			return Functions;
		}

		const std::vector<std::unique_ptr<HXSLStruct>>& GetStructs() const noexcept
		{
			return Structs;
		}

		const std::vector<std::unique_ptr<HXSLClass>>& GetClasses() const noexcept
		{
			return Classes;
		}

		const std::vector<std::unique_ptr<HXSLField>>& GetFields() const noexcept
		{
			return Fields;
		}
	};

	enum HXSLSymbolRefType
	{
		HXSLSymbolRefType_Unknown,
		HXSLSymbolRefType_Namespace,
		HXSLSymbolRefType_Function,
		HXSLSymbolRefType_Constructor,
		HXSLSymbolRefType_FunctionOrConstructor,
		HXSLSymbolRefType_Struct,
		HXSLSymbolRefType_Enum,
		HXSLSymbolRefType_Variable,
		HXSLSymbolRefType_Attribute,
		HXSLSymbolRefType_Member,
		HXSLSymbolRefType_AnyType,
		HXSLSymbolRefType_Any,
	};

	class HXSLType : public HXSLSymbolDef
	{
	private:
		HXSLType() = delete;
	public:
		HXSLType(TextSpan span, ASTNode* parent, HXSLNodeType type, TextSpan name, bool isExtern = false) : HXSLSymbolDef(span, parent, type, name, isExtern)
		{
		}
		virtual ~HXSLType() {}
	};

	// The primitive root type.
	enum HXSLPrimitiveKind
	{
		HXSLPrimitiveKind_Void,
		HXSLPrimitiveKind_Bool,
		HXSLPrimitiveKind_Int,
		HXSLPrimitiveKind_Float,
		HXSLPrimitiveKind_Uint,
		HXSLPrimitiveKind_Double,
		HXSLPrimitiveKind_Min8Float,
		HXSLPrimitiveKind_Min10Float,
		HXSLPrimitiveKind_Min16Float,
		HXSLPrimitiveKind_Min12Int,
		HXSLPrimitiveKind_Min16Int,
		HXSLPrimitiveKind_Min16Uint,
		HXSLPrimitiveKind_Uint8,
		HXSLPrimitiveKind_Int16,
		HXSLPrimitiveKind_Uint16,
		HXSLPrimitiveKind_Float16,
		HXSLPrimitiveKind_Int64,
		HXSLPrimitiveKind_Uint64,
	};

	static HXSLPrimitiveKind& operator++(HXSLPrimitiveKind& kind) {
		kind = static_cast<HXSLPrimitiveKind>(static_cast<int>(kind) + 1);
		return kind;
	}

	enum HXSLPrimitiveClass
	{
		HXSLPrimitiveClass_Scalar = 0,
		HXSLPrimitiveClass_Vector = 1,
		HXSLPrimitiveClass_Matrix = 2,
	};

	class HXSLPrimitive : public HXSLType
	{
	private:
		HXSLPrimitiveKind Kind;
		HXSLPrimitiveClass Class;
		std::string backing_name;
		uint Rows;
		uint Columns;
	public:
		HXSLPrimitive(TextSpan span, ASTNode* parent, HXSLPrimitiveKind kind, HXSLPrimitiveClass _class, std::string& name, uint rows, uint columns)
			: HXSLType(span, parent, HXSLNodeType_Primitive, TextSpan()),
			ASTNode(span, parent, HXSLNodeType_Primitive),
			Kind(kind),
			Class(_class),
			backing_name(std::move(name)),
			Rows(rows),
			Columns(columns)
		{
			Name = TextSpan(backing_name);
		}

		HXSLPrimitive(HXSLPrimitiveKind kind, HXSLPrimitiveClass _class, std::string& name, uint rows, uint columns)
			: HXSLType(name, nullptr, HXSLNodeType_Primitive, TextSpan()),
			ASTNode(name, nullptr, HXSLNodeType_Primitive),
			Kind(kind),
			Class(_class),
			backing_name(std::move(name)),
			Rows(rows),
			Columns(columns)
		{
			Name = TextSpan(backing_name);
		}

		const uint& GetRows() const
		{
			return Rows;
		}

		const uint& GetColumns() const
		{
			return Columns;
		}

		const HXSLPrimitiveKind& GetKind() const
		{
			return Kind;
		}

		const HXSLPrimitiveClass& GetClass() const
		{
			return Class;
		}

		SymbolType GetSymbolType() const override
		{
			return HXSLSymbolType_Primitive;
		}

		void Write(HXSLStream& stream) const override
		{
			HXSL_ASSERT(false, "Cannot write primitive types")
		}

		void Read(HXSLStream& stream, StringPool& container) override
		{
			HXSL_ASSERT(false, "Cannot read primitive types")
		}

		void Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<HXSLSymbolDef>>& nodes) override
		{
			HXSL_ASSERT(false, "Cannot build primitive types")
		}
	};

	struct HXSLSymbolRef
	{
	private:
		std::unique_ptr<std::string> FullyQualifiedName;
		TextSpan Span;
		HXSLSymbolRefType Type;
		const SymbolTable* Table;
		size_t TableIndex;
		std::vector<size_t> arrayDims;

	public:
		HXSLSymbolRef(Token token, HXSLSymbolRefType type) : Span(token.Span), Type(type), Table(nullptr), TableIndex(0)
		{
		}
		HXSLSymbolRef(TextSpan span, HXSLSymbolRefType type) : Span(span), Type(type), Table(nullptr), TableIndex(0)
		{
		}
		HXSLSymbolRef() : Span({}), Type(HXSLSymbolRefType_Unknown), Table(nullptr), TableIndex(0)
		{
		}

		HXSLSymbolRefType GetType() const noexcept { return Type; }

		const TextSpan& GetSpan() const noexcept
		{
			return Span;
		}

		bool IsArray() const noexcept { return !arrayDims.empty(); }

		const std::vector<size_t>& GetArrayDims() const noexcept { return arrayDims; }

		bool IsResolved() const noexcept { return Table != nullptr; }

		void SetTable(const SymbolTable* table, size_t tableIndex);

		const SymbolTable* GetTable() const { return Table; }

		const size_t& GetTableIndex() const { return TableIndex; }

		const std::string& GetFullyQualifiedName() const;

		const SymbolMetadata* GetMetadata() const;

		HXSLSymbolDef* GetDeclaration() const;

		void Write(HXSLStream& stream) const;

		void Read(HXSLStream& stream);
	};

	class IHXSLHasSymbolRef
	{
	public:
		virtual const std::unique_ptr<HXSLSymbolRef>& GetSymbolRef() = 0;
		virtual ~IHXSLHasSymbolRef() = default;
	};

	struct LazySymbol
	{
		Token token;
		HXSLSymbolRefType type;
		std::unique_ptr<HXSLSymbolRef> ptr;

		LazySymbol() : token({}), type(HXSLSymbolRefType_Unknown)
		{
		}

		LazySymbol(std::unique_ptr<HXSLSymbolRef> ptr) : token({}), type(HXSLSymbolRefType_Unknown), ptr(std::move(ptr))
		{
		}

		LazySymbol(Token token, HXSLSymbolRefType type) : token(token), type(type)
		{
		}

		std::unique_ptr<HXSLSymbolRef> make(HXSLSymbolRefType overwrite)
		{
			if (ptr.get())
			{
				return std::move(ptr);
			}

			if (token.Type == TokenType_Unknown)
			{
				HXSL_ASSERT(false, "Attempted to double create a LazySymbol");
				return {};
			}
			auto result = std::make_unique<HXSLSymbolRef>(token, overwrite);
			token = {};
			type = {};
			return std::move(result);
		};

		std::unique_ptr<HXSLSymbolRef> make()
		{
			return std::move(make(type));
		};
	};

	class HXSLSwizzleDefinition : virtual public ASTNode, public HXSLSymbolDef, public IHXSLHasSymbolRef
	{
	private:
		TextSpan Expression;
		std::unique_ptr<HXSLSymbolRef> Symbol;
	public:
		HXSLSwizzleDefinition(TextSpan expression, std::unique_ptr<HXSLSymbolRef> symbol)
			: ASTNode(expression, nullptr, HXSLNodeType_SwizzleDefinition),
			HXSLSymbolDef(expression, nullptr, HXSLNodeType_SwizzleDefinition, expression),
			Expression(expression),
			Symbol(std::move(symbol))
		{
		}

		const std::unique_ptr<HXSLSymbolRef>& GetSymbolRef()
		{
			return Symbol;
		}

		SymbolType GetSymbolType() const override
		{
			return HXSLSymbolType_Field;
		}

		void Write(HXSLStream& stream) const override
		{
			HXSL_ASSERT(false, "Cannot write swizzle types")
		}

		void Read(HXSLStream& stream, StringPool& container) override
		{
			HXSL_ASSERT(false, "Cannot read swizzle types")
		}

		void Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<HXSLSymbolDef>>& nodes) override
		{
			HXSL_ASSERT(false, "Cannot build swizzle types")
		}
	};

	class HXSLLiteralExpression;

	class HXSLAttributeDeclaration : public ASTNode
	{
	private:
		std::unique_ptr<HXSLSymbolRef> Symbol;
		std::vector<std::unique_ptr<HXSLLiteralExpression>> Parameters;
	public:
		HXSLAttributeDeclaration(TextSpan span, ASTNode* parent, std::unique_ptr<HXSLSymbolRef> symbol, std::vector<std::unique_ptr<HXSLLiteralExpression>> parameters)
			: ASTNode(span, parent, HXSLNodeType_AttributeDeclaration),
			Symbol(std::move(symbol)),
			Parameters(std::move(parameters))
		{
		}
		HXSLAttributeDeclaration(TextSpan span, ASTNode* parent)
			: ASTNode(span, parent, HXSLNodeType_AttributeDeclaration)
		{
		}

		std::unique_ptr<HXSLSymbolRef>& GetSymbolRef()
		{
			return Symbol;
		}

		DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLSymbolRef>, Symbol)
			DEFINE_GETTER_SETTER_MOVE(std::vector<std::unique_ptr<HXSLLiteralExpression>>, Parameters)
	};

	class HXSLAttributeContainer
	{
	protected:
		std::vector<std::unique_ptr<HXSLAttributeDeclaration>> Attributes;
	public:
		virtual ~HXSLAttributeContainer() {}
		void AddAttribute(std::unique_ptr<HXSLAttributeDeclaration> attribute)
		{
			attribute->SetParent(dynamic_cast<ASTNode*>(this));
			Attributes.push_back(std::move(attribute));
		}
	};

	class HXSLParameter : virtual public ASTNode, public HXSLSymbolDef, public IHXSLHasSymbolRef
	{
	private:
		HXSLParameterFlags Flags;
		std::unique_ptr<HXSLSymbolRef> Symbol;
		TextSpan Semantic;

	public:
		HXSLParameter()
			: HXSLSymbolDef(TextSpan(), nullptr, HXSLNodeType_Parameter, TextSpan(), true),
			ASTNode(TextSpan(), nullptr, HXSLNodeType_Parameter, true),
			Flags(HXSLParameterFlags_None),
			Semantic(TextSpan())
		{
		}
		HXSLParameter(TextSpan span, ASTNode* parent, HXSLParameterFlags flags, std::unique_ptr<HXSLSymbolRef> symbol, TextSpan name, TextSpan semantic)
			: HXSLSymbolDef(span, parent, HXSLNodeType_Parameter, name),
			ASTNode(span, parent, HXSLNodeType_Parameter),
			Flags(flags),
			Symbol(std::move(symbol)),
			Semantic(semantic)
		{
		}

		std::unique_ptr<HXSLSymbolRef>& GetSymbolRef() override
		{
			return Symbol;
		}

		SymbolType GetSymbolType() const override
		{
			return HXSLSymbolType_Parameter;
		}

		void Write(HXSLStream& stream) const override;

		void Read(HXSLStream& stream, StringPool& container) override;

		void Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<HXSLSymbolDef>>& nodes) override;

		~HXSLParameter() override
		{
		}
	};

	class HXSLStatement : virtual public ASTNode
	{
	protected:
		HXSLStatement(TextSpan span, ASTNode* parent, HXSLNodeType type)
			: ASTNode(span, parent, type)
		{
		}
	};

	class HXSLStatementContainer
	{
	protected:
		std::vector<std::unique_ptr<HXSLStatement>> Statements;

	public:
		virtual ~HXSLStatementContainer() = default;

		void AddStatement(std::unique_ptr<HXSLStatement> statement)
		{
			Statements.push_back(std::move(statement));
		}

		const std::vector<std::unique_ptr<HXSLStatement>>& GetStatements() const
		{
			return Statements;
		}
	};

	class HXSLBlockStatement : public HXSLStatement, public HXSLStatementContainer
	{
	public:
		HXSLBlockStatement(TextSpan span, ASTNode* parent)
			: HXSLStatement(span, parent, HXSLNodeType_BlockStatement),
			ASTNode(span, parent, HXSLNodeType_BlockStatement)
		{
		}

		std::string DebugName() const override
		{
			return "[" + toString(type) + "]";
		}
	};

	class HXSLExpression : public ASTNode
	{
	private:
		HXSLSymbolDef* InferredType;
		bool LazyEval;
	protected:
		HXSLExpression(TextSpan span, ASTNode* parent, HXSLNodeType type)
			: ASTNode(span, parent, type),
			LazyEval(false),
			InferredType(nullptr)
		{
		}

	public:

		HXSLSymbolDef* GetInferredType() const noexcept
		{
			return InferredType;
		}

		void SetInferredType(HXSLSymbolDef* def) noexcept
		{
			InferredType = def;
			LazyEval = false;
		}

		DEFINE_GETTER_SETTER(bool, LazyEval)
	};

	class IHXSLChainExpression
	{
	public:
		virtual ~IHXSLChainExpression() = default;

		virtual	void chain(std::unique_ptr<HXSLExpression> expression)
		{
			HXSL_ASSERT(false, "Chain method not implemented");
		}

		virtual const std::unique_ptr<HXSLExpression>& chainNext()
		{
			HXSL_ASSERT(false, "Get Chain method not implemented");
			return nullptr;
		}
	};

	class HXSLUnaryExpression : public HXSLExpression
	{
	private:
		HXSLOperator Operator;
		std::unique_ptr<HXSLExpression> Operand;
	protected:
		HXSLUnaryExpression(TextSpan span, ASTNode* parent, HXSLNodeType type, HXSLOperator op, std::unique_ptr<HXSLExpression> operand)
			: HXSLExpression(span, parent, type),
			Operator(op),
			Operand(std::move(operand))
		{
			if (Operand) Operand->SetParent(this);
		}

	public:

		DEFINE_GETTER_SETTER(HXSLOperator, Operator)

			DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLExpression>, Operand)
	};

	class HXSLPrefixExpression : public HXSLUnaryExpression
	{
	public:
		HXSLPrefixExpression(TextSpan span, ASTNode* parent, HXSLOperator op, std::unique_ptr<HXSLExpression> operand)
			: HXSLUnaryExpression(span, parent, HXSLNodeType_PrefixExpression, op, std::move(operand))
		{
		}
	};

	class HXSLPostfixExpression : public HXSLUnaryExpression
	{
	public:
		HXSLPostfixExpression(TextSpan span, ASTNode* parent, HXSLOperator op, std::unique_ptr<HXSLExpression> operand)
			: HXSLUnaryExpression(span, parent, HXSLNodeType_PostfixExpression, op, std::move(operand))
		{
		}
	};

	class HXSLBinaryExpression : public HXSLExpression, public IHXSLChainExpression
	{
	private:
		HXSLOperator Operator;
		std::unique_ptr<HXSLExpression> Left;
		std::unique_ptr<HXSLExpression> Right;
	public:
		HXSLBinaryExpression(TextSpan span, ASTNode* parent, HXSLOperator op, std::unique_ptr<HXSLExpression> left, std::unique_ptr<HXSLExpression> right)
			: HXSLExpression(span, parent, HXSLNodeType_BinaryExpression),
			Operator(op),
			Left(std::move(left)),
			Right(std::move(right))
		{
			if (Left) Left->SetParent(this);
			if (Right) Right->SetParent(this);
		}

		void chain(std::unique_ptr<HXSLExpression> expression) override
		{
			Right = std::move(expression);
			Right->SetParent(this);
		}

		virtual const std::unique_ptr<HXSLExpression>& chainNext() override
		{
			return Right;
		}

		DEFINE_GETTER_SETTER(HXSLOperator, Operator)

			DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLExpression>, Left)

			DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLExpression>, Right)
	};

	class HXSLCastExpression : public HXSLExpression
	{
	private:
		std::unique_ptr<HXSLExpression> Left;
		std::unique_ptr<HXSLExpression> Right;
	public:
		HXSLCastExpression(TextSpan span, ASTNode* parent, std::unique_ptr<HXSLExpression> left, std::unique_ptr<HXSLExpression> right)
			: HXSLExpression(span, parent, HXSLNodeType_CastExpression),
			Left(std::move(left)),
			Right(std::move(right))
		{
			if (Left) Left->SetParent(this);
			if (Right) Right->SetParent(this);
		}

		DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLExpression>, Left)

			DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLExpression>, Right)
	};

	class HXSLTernaryExpression : public HXSLExpression
	{
	private:
		std::unique_ptr<HXSLExpression> Condition;
		std::unique_ptr<HXSLExpression> Left;
		std::unique_ptr<HXSLExpression> Right;
	public:
		HXSLTernaryExpression(TextSpan span, ASTNode* parent, std::unique_ptr<HXSLExpression> condition, std::unique_ptr<HXSLExpression> left, std::unique_ptr<HXSLExpression> right)
			: HXSLExpression(span, parent, HXSLNodeType_TernaryExpression),
			Condition(std::move(condition)),
			Left(std::move(left)),
			Right(std::move(right))
		{
			if (Left) Left->SetParent(this);
			if (Right) Right->SetParent(this);
		}

		DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLExpression>, Condition)

			DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLExpression>, Left)

			DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLExpression>, Right)
	};

	class HXSLEmptyExpression : public HXSLExpression
	{
	public:
		HXSLEmptyExpression(TextSpan span, ASTNode* parent)
			: HXSLExpression(span, parent, HXSLNodeType_EmptyExpression)
		{
		}
	};

	class HXSLLiteralExpression : public HXSLExpression
	{
	private:
		Token ExpressionToken;
	public:
		HXSLLiteralExpression(TextSpan span, ASTNode* parent, Token token)
			: HXSLExpression(span, parent, HXSLNodeType_LiteralExpression),
			ExpressionToken(token)
		{
		}

		DEFINE_GETTER_SETTER(Token, ExpressionToken)
	};

	class HXSLSymbolRefExpression : public HXSLExpression, public IHXSLHasSymbolRef
	{
	private:
		std::unique_ptr<HXSLSymbolRef> Symbol;
	public:
		HXSLSymbolRefExpression(TextSpan span, ASTNode* parent, std::unique_ptr<HXSLSymbolRef> symbol)
			: HXSLExpression(span, parent, HXSLNodeType_SymbolRefExpression),
			Symbol(std::move(symbol))
		{
		}

		std::unique_ptr<HXSLSymbolRef>& GetSymbolRef() override
		{
			return Symbol;
		}

		DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLSymbolRef>, Symbol)

			DEFINE_DETATCH(std::unique_ptr<HXSLSymbolRef>, Symbol)
	};

	class HXSLCallParameter : public ASTNode
	{
	private:
		std::unique_ptr<HXSLExpression> Expression;
	public:
		HXSLCallParameter(TextSpan span, ASTNode* parent, std::unique_ptr<HXSLExpression> expression)
			: ASTNode(span, parent, HXSLNodeType_FunctionCallParameter),
			Expression(std::move(expression))
		{
			if (Expression) Expression->SetParent(this);
		}

		DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLExpression>, Expression)
	};

	class HXSLFunctionCallExpression : public HXSLExpression, public IHXSLHasSymbolRef
	{
	private:
		std::unique_ptr<HXSLSymbolRef> Symbol;
		std::vector<std::unique_ptr<HXSLCallParameter>> Parameters;
	public:
		HXSLFunctionCallExpression(TextSpan span, ASTNode* parent, std::unique_ptr<HXSLSymbolRef> symbol, std::vector<std::unique_ptr<HXSLCallParameter>> parameters)
			: HXSLExpression(span, parent, HXSLNodeType_FunctionCallExpression),
			Symbol(std::move(symbol)),
			Parameters(std::move(parameters))
		{
		}

		HXSLFunctionCallExpression(TextSpan span, ASTNode* parent, std::unique_ptr<HXSLSymbolRef> symbol)
			: HXSLExpression(span, parent, HXSLNodeType_FunctionCallExpression),
			Symbol(std::move(symbol))
		{
		}

		std::string GetSignature()
		{
		}

		std::unique_ptr<HXSLSymbolRef>& GetSymbolRef() override
		{
			return Symbol;
		}

		DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLSymbolRef>, Symbol)

			DEFINE_GETTER_SETTER_MOVE(std::vector<std::unique_ptr<HXSLCallParameter>>, Parameters)
	};

	class HXSLMemberAccessExpression : public HXSLExpression, public IHXSLChainExpression, public IHXSLHasSymbolRef
	{
	private:
		std::unique_ptr<HXSLSymbolRef> Symbol;
		std::unique_ptr<HXSLExpression> Expression;
	public:
		HXSLMemberAccessExpression(TextSpan span, ASTNode* parent, std::unique_ptr<HXSLSymbolRef> symbol, std::unique_ptr<HXSLExpression> expression)
			: HXSLExpression(span, parent, HXSLNodeType_MemberAccessExpression),
			Symbol(std::move(symbol)),
			Expression(std::move(expression))
		{
		}

		std::unique_ptr<HXSLSymbolRef>& GetSymbolRef() override
		{
			return Symbol;
		}

		void chain(std::unique_ptr<HXSLExpression> expression) override
		{
			Expression = std::move(expression);
			Expression->SetParent(this);
		}

		virtual const std::unique_ptr<HXSLExpression>& chainNext() override
		{
			return Expression;
		}

		DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLSymbolRef>, Symbol)

			DEFINE_DETATCH(std::unique_ptr<HXSLSymbolRef>, Symbol)

			DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLExpression>, Expression)
	};

	class HXSLIndexerAccessExpression : public HXSLExpression
	{
	private:
		std::unique_ptr<HXSLSymbolRef> Symbol;
		std::vector<std::unique_ptr<HXSLExpression>> Indices;

	public:
		HXSLIndexerAccessExpression(TextSpan span, ASTNode* parent, std::unique_ptr<HXSLSymbolRef> symbol, std::vector<std::unique_ptr<HXSLExpression>> indices)
			: HXSLExpression(span, parent, HXSLNodeType_IndexerAccessExpression),
			Symbol(std::move(symbol)),
			Indices(std::move(indices))
		{
		}

		HXSLIndexerAccessExpression(TextSpan span, ASTNode* parent, std::unique_ptr<HXSLSymbolRef> symbol)
			: HXSLExpression(span, parent, HXSLNodeType_IndexerAccessExpression),
			Symbol(std::move(symbol))
		{
		}

		void AddIndex(std::unique_ptr<HXSLExpression> expression)
		{
			expression->SetParent(this);
			Indices.push_back(std::move(expression));
		}

		DEFINE_GETTER_SETTER_MOVE(std::vector<std::unique_ptr<HXSLExpression>>, Indices)

			DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLSymbolRef>, Symbol)
			DEFINE_DETATCH(std::unique_ptr<HXSLSymbolRef>, Symbol)
	};

	class HXSLAssignmentExpression : public HXSLExpression {
	private:
		std::unique_ptr<HXSLExpression> Target;
		std::unique_ptr<HXSLExpression> Expression;

	protected:
		HXSLAssignmentExpression(TextSpan span, ASTNode* parent, HXSLNodeType type, std::unique_ptr<HXSLExpression> target, std::unique_ptr<HXSLExpression> expression)
			: HXSLExpression(span, parent, type),
			Target(std::move(target)),
			Expression(std::move(expression))
		{
			if (Target) Target->SetParent(this);
			if (Expression) Expression->SetParent(this);
		}

	public:
		HXSLAssignmentExpression(TextSpan span, ASTNode* parent, std::unique_ptr<HXSLExpression> target, std::unique_ptr<HXSLExpression> expression)
			: HXSLExpression(span, parent, HXSLNodeType_AssignmentExpression),
			Target(std::move(target)),
			Expression(std::move(expression))
		{
			if (Target) Target->SetParent(this);
			if (Expression) Expression->SetParent(this);
		}

		DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLExpression>, Target)

			DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLExpression>, Expression)
	};

	class HXSLCompoundAssignmentExpression : public HXSLAssignmentExpression {
	private:
		HXSLOperator Operator;

	public:
		HXSLCompoundAssignmentExpression(TextSpan span, ASTNode* parent, HXSLOperator op, std::unique_ptr<HXSLExpression> target, std::unique_ptr<HXSLExpression> expression)
			: HXSLAssignmentExpression(span, parent, HXSLNodeType_CompoundAssignmentExpression, std::move(target), std::move(expression)),
			Operator(op)
		{
		}

		DEFINE_GETTER_SETTER(HXSLOperator, Operator)
	};

	class HXSLInitializationExpression : public HXSLExpression
	{
	private:
		std::vector<std::unique_ptr<HXSLExpression>> Parameters;

	public:
		HXSLInitializationExpression(TextSpan span, ASTNode* parent, std::vector<std::unique_ptr<HXSLExpression>> parameters)
			: HXSLExpression(span, parent, HXSLNodeType_InitializationExpression),
			Parameters(std::move(parameters))
		{
		}
		HXSLInitializationExpression(TextSpan span, ASTNode* parent)
			: HXSLExpression(span, parent, HXSLNodeType_InitializationExpression)
		{
		}

		void AddParameter(std::unique_ptr<HXSLExpression> parameter)
		{
			Parameters.push_back(std::move(parameter));
		}

		DEFINE_GETTER_SETTER_MOVE(std::vector<std::unique_ptr<HXSLExpression>>, Parameters)
	};

	class HXSLDeclarationStatement : public HXSLStatement, public HXSLSymbolDef, public IHXSLHasSymbolRef
	{
	private:
		std::unique_ptr<HXSLSymbolRef> Symbol;
		std::unique_ptr<HXSLExpression> Initializer;

	public:
		HXSLDeclarationStatement()
			: HXSLStatement(TextSpan(), nullptr, HXSLNodeType_DeclarationStatement),
			HXSLSymbolDef(TextSpan(), nullptr, HXSLNodeType_DeclarationStatement, TextSpan(), true),
			ASTNode(TextSpan(), nullptr, HXSLNodeType_DeclarationStatement, true)
		{
			if (Initializer) Initializer->SetParent(this);
		}
		HXSLDeclarationStatement(TextSpan span, ASTNode* parent, std::unique_ptr<HXSLSymbolRef> symbol, TextSpan name, std::unique_ptr<HXSLExpression> initializer)
			: HXSLStatement(span, parent, HXSLNodeType_DeclarationStatement),
			HXSLSymbolDef(span, parent, HXSLNodeType_DeclarationStatement, name),
			ASTNode(span, parent, HXSLNodeType_DeclarationStatement),
			Symbol(std::move(symbol)),
			Initializer(std::move(initializer))
		{
			if (Initializer) Initializer->SetParent(this);
		}

		std::unique_ptr<HXSLSymbolRef>& GetSymbolRef() override
		{
			return Symbol;
		}

		SymbolType GetSymbolType() const override
		{
			return HXSLSymbolType_Variable;
		}

		void Write(HXSLStream& stream) const override;

		void Read(HXSLStream& stream, StringPool& container) override;

		void Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<HXSLSymbolDef>>& nodes) override;

		DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLExpression>, Initializer)
	};

	class HXSLAssignmentStatement : public HXSLStatement {
	private:
		std::unique_ptr<HXSLExpression> Target;
		std::unique_ptr<HXSLExpression> Expression;

	protected:
		HXSLAssignmentStatement(TextSpan span, ASTNode* parent, HXSLNodeType type, std::unique_ptr<HXSLExpression> target, std::unique_ptr<HXSLExpression> expression)
			: HXSLStatement(span, parent, type),
			ASTNode(span, parent, type),
			Target(std::move(target)),
			Expression(std::move(expression))
		{
			if (Target) Target->SetParent(this);
			if (Expression) Expression->SetParent(this);
		}

	public:
		HXSLAssignmentStatement(TextSpan span, ASTNode* parent, std::unique_ptr<HXSLExpression> target, std::unique_ptr<HXSLExpression> expression)
			: HXSLStatement(span, parent, HXSLNodeType_AssignmentStatement),
			ASTNode(span, parent, HXSLNodeType_AssignmentStatement),
			Target(std::move(target)),
			Expression(std::move(expression))
		{
			if (Target) Target->SetParent(this);
			if (Expression) Expression->SetParent(this);
		}

		DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLExpression>, Target)

			DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLExpression>, Expression)
	};

	class HXSLCompoundAssignmentStatement : public HXSLAssignmentStatement {
	private:
		HXSLOperator Operator;

	public:
		HXSLCompoundAssignmentStatement(TextSpan span, ASTNode* parent, HXSLOperator op, std::unique_ptr<HXSLExpression> target, std::unique_ptr<HXSLExpression> expression)
			: HXSLAssignmentStatement(span, parent, HXSLNodeType_CompoundAssignmentStatement, std::move(target), std::move(expression)),
			ASTNode(span, parent, HXSLNodeType_CompoundAssignmentStatement),
			Operator(op)
		{
		}

		DEFINE_GETTER_SETTER(HXSLOperator, Operator)
	};

	class HXSLFunctionCallStatement : public HXSLStatement
	{
	private:
		std::unique_ptr<HXSLFunctionCallExpression> Expression;

	public:
		HXSLFunctionCallStatement(TextSpan span, ASTNode* parent, std::unique_ptr<HXSLFunctionCallExpression> expression)
			: HXSLStatement(span, parent, HXSLNodeType_FunctionCallStatement),
			ASTNode(span, parent, HXSLNodeType_FunctionCallStatement),
			Expression(std::move(expression))
		{
			if (Expression) Expression->SetParent(this);
		}

		DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLFunctionCallExpression>, Expression)
	};

	class HXSLReturnStatement : public HXSLStatement {
	private:
		std::unique_ptr<HXSLExpression> ReturnValueExpression;

	public:
		HXSLReturnStatement(TextSpan span, ASTNode* parent, std::unique_ptr<HXSLExpression> returnValueExpression)
			: HXSLStatement(span, parent, HXSLNodeType_ReturnStatement),
			ASTNode(span, parent, HXSLNodeType_ReturnStatement),
			ReturnValueExpression(std::move(returnValueExpression))
		{
			if (ReturnValueExpression) ReturnValueExpression->SetParent(this);
		}

		DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLExpression>, ReturnValueExpression)
	};

	class HXSLIfStatement : public HXSLStatement, public HXSLAttributeContainer {
	private:
		std::unique_ptr<HXSLExpression> Expression;
		std::unique_ptr<HXSLBlockStatement> Body;

	public:
		HXSLIfStatement(TextSpan span, ASTNode* parent, std::unique_ptr<HXSLExpression> expression, std::unique_ptr<HXSLBlockStatement> body)
			: HXSLStatement(span, parent, HXSLNodeType_IfStatement),
			ASTNode(span, parent, HXSLNodeType_IfStatement),
			Expression(std::move(expression)),
			Body(std::move(body))
		{
			if (Expression) Expression->SetParent(this);
			if (Body) Body->SetParent(this);
		}

		DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLExpression>, Expression)

			DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLBlockStatement>, Body)
	};

	class HXSLElseStatement : public HXSLStatement {
	private:
		std::unique_ptr<HXSLBlockStatement> Body;

	public:
		HXSLElseStatement(TextSpan span, ASTNode* parent, std::unique_ptr<HXSLBlockStatement> body)
			: HXSLStatement(span, parent, HXSLNodeType_ElseStatement),
			ASTNode(span, parent, HXSLNodeType_ElseStatement),
			Body(std::move(body))
		{
			if (Body) Body->SetParent(this);
		}

		DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLBlockStatement>, Body)
	};

	class HXSLElseIfStatement : public HXSLStatement {
	private:
		std::unique_ptr<HXSLExpression> Expression;
		std::unique_ptr<HXSLBlockStatement> Body;

	public:
		HXSLElseIfStatement(TextSpan span, ASTNode* parent, std::unique_ptr<HXSLExpression> expression, std::unique_ptr<HXSLBlockStatement> body)
			: HXSLStatement(span, parent, HXSLNodeType_ElseIfStatement),
			ASTNode(span, parent, HXSLNodeType_ElseIfStatement),
			Expression(std::move(expression)),
			Body(std::move(body))
		{
			if (Expression) Expression->SetParent(this);
			if (Body) Body->SetParent(this);
		}

		DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLExpression>, Expression)

			DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLBlockStatement>, Body)
	};

	class HXSLCaseStatement : public HXSLStatement, public HXSLStatementContainer
	{
	private:
		std::unique_ptr<HXSLExpression> Expression;
	public:
		HXSLCaseStatement(TextSpan span, ASTNode* parent, std::unique_ptr<HXSLExpression> expression)
			: HXSLStatement(span, parent, HXSLNodeType_CaseStatement),
			ASTNode(span, parent, HXSLNodeType_CaseStatement),
			Expression(std::move(expression))
		{
		}

		DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLExpression>, Expression)
	};

	class HXSLDefaultCaseStatement : public HXSLStatement, public HXSLStatementContainer
	{
	private:

	public:
		HXSLDefaultCaseStatement(TextSpan span, ASTNode* parent)
			: HXSLStatement(span, parent, HXSLNodeType_DefaultCaseStatement),
			ASTNode(span, parent, HXSLNodeType_DefaultCaseStatement)
		{
		}
	};

	class HXSLSwitchStatement : public HXSLStatement, public HXSLAttributeContainer
	{
	private:
		std::unique_ptr<HXSLExpression> Expression;
		std::vector<std::unique_ptr<HXSLCaseStatement>> Cases;
		std::unique_ptr<HXSLDefaultCaseStatement> DefaultCase;
	public:
		HXSLSwitchStatement(TextSpan span, ASTNode* parent, std::unique_ptr<HXSLExpression> expression, std::vector<std::unique_ptr<HXSLCaseStatement>> cases, std::unique_ptr<HXSLDefaultCaseStatement> defaultCase)
			: HXSLStatement(span, parent, HXSLNodeType_SwitchStatement),
			ASTNode(span, parent, HXSLNodeType_SwitchStatement),
			Expression(std::move(expression)),
			Cases(std::move(cases)),
			DefaultCase(std::move(defaultCase))
		{
		}
		HXSLSwitchStatement(TextSpan span, ASTNode* parent)
			: HXSLStatement(span, parent, HXSLNodeType_SwitchStatement),
			ASTNode(span, parent, HXSLNodeType_SwitchStatement)
		{
		}

		DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLExpression>, Expression)

			void AddCase(std::unique_ptr<HXSLCaseStatement> _case) { Cases.push_back(std::move(_case)); }

		DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLDefaultCaseStatement>, DefaultCase)
	};

	class HXSLForStatement : public HXSLStatement, public HXSLAttributeContainer
	{
	private:
		std::unique_ptr<HXSLStatement> Init;
		std::unique_ptr<HXSLExpression> Condition;
		std::unique_ptr<HXSLExpression> Iteration;
		std::unique_ptr<HXSLBlockStatement> Body;
	public:
		HXSLForStatement(TextSpan span, ASTNode* parent, std::unique_ptr<HXSLStatement> init, std::unique_ptr<HXSLExpression> condition, std::unique_ptr<HXSLBlockStatement> body)
			: HXSLStatement(span, parent, HXSLNodeType_ForStatement),
			ASTNode(span, parent, HXSLNodeType_ForStatement),
			Init(std::move(init)),
			Condition(std::move(condition)),
			Body(std::move(body))
		{
		}
		HXSLForStatement(TextSpan span, ASTNode* parent)
			: HXSLStatement(span, parent, HXSLNodeType_ForStatement),
			ASTNode(span, parent, HXSLNodeType_ForStatement)
		{
		}

		DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLStatement>, Init)
			DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLExpression>, Condition)
			DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLExpression>, Iteration)
			DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLBlockStatement>, Body)
	};

	class HXSLBreakStatement : public HXSLStatement {
	public:
		HXSLBreakStatement(TextSpan span, ASTNode* parent)
			: HXSLStatement(span, parent, HXSLNodeType_BreakStatement),
			ASTNode(span, parent, HXSLNodeType_BreakStatement)
		{
		}
	};

	class HXSLContinueStatement : public HXSLStatement {
	public:
		HXSLContinueStatement(TextSpan span, ASTNode* parent)
			: HXSLStatement(span, parent, HXSLNodeType_ContinueStatement),
			ASTNode(span, parent, HXSLNodeType_ContinueStatement)
		{
		}
	};

	class HXSLDiscardStatement : public HXSLStatement {
	public:
		HXSLDiscardStatement(TextSpan span, ASTNode* parent)
			: HXSLStatement(span, parent, HXSLNodeType_DiscardStatement),
			ASTNode(span, parent, HXSLNodeType_DiscardStatement)
		{
		}
	};

	class HXSLWhileStatement : public HXSLStatement {
	private:
		std::unique_ptr<HXSLExpression> Expression;
		std::unique_ptr<HXSLBlockStatement> Body;

	public:
		HXSLWhileStatement(TextSpan span, ASTNode* parent, std::unique_ptr<HXSLExpression> expression, std::unique_ptr<HXSLBlockStatement> body)
			: HXSLStatement(span, parent, HXSLNodeType_WhileStatement),
			ASTNode(span, parent, HXSLNodeType_WhileStatement),
			Expression(std::move(expression)),
			Body(std::move(body))
		{
			if (Expression) Expression->SetParent(this);
			if (Body) Body->SetParent(this);
		}

		DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLExpression>, Expression)

			DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLBlockStatement>, Body)
	};

	class HXSLFunction : public HXSLSymbolDef, public HXSLAttributeContainer
	{
	private:
		HXSLAccessModifier AccessModifiers;
		HXSLFunctionFlags Flags;
		std::unique_ptr<HXSLSymbolRef> ReturnSymbol;
		std::vector<std::unique_ptr<HXSLParameter>> Parameters;
		TextSpan Semantic;
		std::unique_ptr<HXSLBlockStatement> Body;

	protected:
		HXSLFunction(TextSpan span, ASTNode* parent, HXSLNodeType type, TextSpan name)
			: HXSLSymbolDef(span, parent, type, name),
			ASTNode(span, parent, type),
			Flags(HXSLFunctionFlags_None),
			AccessModifiers(HXSLAccessModifier_Private)
		{
		}

	public:
		HXSLFunction()
			: HXSLSymbolDef(TextSpan(), nullptr, HXSLNodeType_Function, TextSpan(), true),
			ASTNode(TextSpan(), nullptr, HXSLNodeType_Function, true),
			AccessModifiers(HXSLAccessModifier_Private),
			Flags(HXSLFunctionFlags_None),
			Semantic(TextSpan())
		{
		}
		HXSLFunction(TextSpan span, ASTNode* parent, HXSLAccessModifier accessModifiers, HXSLFunctionFlags flags, TextSpan name, std::unique_ptr<HXSLSymbolRef> returnSymbol, std::vector<std::unique_ptr<HXSLParameter>> parameters, TextSpan semantic)
			: HXSLSymbolDef(span, parent, HXSLNodeType_Function, name),
			ASTNode(span, parent, HXSLNodeType_Function),
			AccessModifiers(accessModifiers),
			Flags(flags),
			ReturnSymbol(std::move(returnSymbol)),
			Parameters(std::move(parameters)),
			Semantic(semantic)
		{
		}

		HXSLFunction(TextSpan span, ASTNode* parent, HXSLAccessModifier accessModifiers, HXSLFunctionFlags flags, TextSpan name, std::unique_ptr<HXSLSymbolRef> returnSymbol)
			: HXSLSymbolDef(span, parent, HXSLNodeType_Function, name),
			ASTNode(span, parent, HXSLNodeType_Function),
			AccessModifiers(accessModifiers),
			Flags(flags),
			ReturnSymbol(std::move(returnSymbol))
		{
		}

		void AddParameter(std::unique_ptr<HXSLParameter> parameter)
		{
			parameter->SetParent(this);
			Parameters.push_back(std::move(parameter));
		}

		std::unique_ptr<HXSLSymbolRef>& GetReturnSymbolRef()
		{
			return ReturnSymbol;
		}

		SymbolType GetSymbolType() const override
		{
			return HXSLSymbolType_Function;
		}

		void Write(HXSLStream& stream) const override;

		void Read(HXSLStream& stream, StringPool& container) override;

		void Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<HXSLSymbolDef>>& nodes) override;

		std::string DebugName() const override
		{
			return "[" + toString(type) + "] Name: " + Name.toString();
		}
		DEFINE_GETTER_SETTER(HXSLAccessModifier, AccessModifiers)
			DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLSymbolRef>, ReturnSymbol)
			DEFINE_GETTER_SETTER(TextSpan, Name)
			DEFINE_GETTER_SETTER_MOVE(std::vector<std::unique_ptr<HXSLParameter>>, Parameters)
			DEFINE_GETTER_SETTER(TextSpan, Semantic)
			DEFINE_GETTER_SETTER_MOVE(std::unique_ptr<HXSLBlockStatement>, Body)
	};

	class HXSLField : virtual public ASTNode, public HXSLSymbolDef, public IHXSLHasSymbolRef
	{
	private:
		HXSLAccessModifier AccessModifiers;
		HXSLFieldFlags Flags;
		std::unique_ptr<HXSLSymbolRef> Symbol;
		TextSpan Semantic;
	public:
		HXSLField()
			: HXSLSymbolDef(TextSpan(), nullptr, HXSLNodeType_Field, TextSpan(), true),
			ASTNode(TextSpan(), nullptr, HXSLNodeType_Field, true),
			AccessModifiers(HXSLAccessModifier_Private),
			Flags(HXSLFieldFlags_None),
			Semantic(TextSpan())
		{
		}
		HXSLField(TextSpan span, ASTNode* parent, HXSLAccessModifier access, HXSLFieldFlags flags, TextSpan name, std::unique_ptr<HXSLSymbolRef> symbol, TextSpan semantic)
			: HXSLSymbolDef(span, parent, HXSLNodeType_Field, name),
			ASTNode(span, parent, HXSLNodeType_Field),
			AccessModifiers(access),
			Flags(flags),
			Symbol(std::move(symbol)),
			Semantic(semantic)
		{
		}

		std::unique_ptr<HXSLSymbolRef>& GetSymbolRef() override
		{
			return Symbol;
		}

		SymbolType GetSymbolType() const override
		{
			return HXSLSymbolType_Field;
		}

		void Write(HXSLStream& stream) const override;

		void Read(HXSLStream& stream, StringPool& container) override;

		void Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<HXSLSymbolDef>>& nodes) override;

		DEFINE_GETTER_SETTER(HXSLAccessModifier, AccessModifiers)
	};

	class HXSLStruct : public HXSLType, public HXSLContainer
	{
	private:
		HXSLAccessModifier AccessModifiers;
	public:
		HXSLStruct()
			: HXSLType(TextSpan(), nullptr, HXSLNodeType_Struct, TextSpan(), true),
			ASTNode(TextSpan(), nullptr, HXSLNodeType_Struct, true),
			HXSLContainer(TextSpan(), nullptr, HXSLNodeType_Struct, true),
			AccessModifiers(HXSLAccessModifier_Private)
		{
		}
		HXSLStruct(TextSpan span, ASTNode* parent, HXSLAccessModifier access, TextSpan name)
			: HXSLType(span, parent, HXSLNodeType_Struct, name),
			ASTNode(span, parent, HXSLNodeType_Struct),
			HXSLContainer(span, parent, HXSLNodeType_Struct),
			AccessModifiers(access)
		{
		}

		std::string DebugName() const override
		{
			return "[" + toString(type) + "] Name: " + Name.toString();
		}

		SymbolType GetSymbolType() const override
		{
			return HXSLSymbolType_Struct;
		}

		void Write(HXSLStream& stream) const override;

		void Read(HXSLStream& stream, StringPool& container) override;

		void Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<HXSLSymbolDef>>& nodes) override;

		DEFINE_GETTER_SETTER(HXSLAccessModifier, AccessModifiers)
	};

	class HXSLClass : public HXSLType, public HXSLContainer
	{
	private:
		HXSLAccessModifier AccessModifiers;
	public:
		HXSLClass()
			: HXSLType(TextSpan(), nullptr, HXSLNodeType_Class, TextSpan(), true),
			ASTNode(TextSpan(), nullptr, HXSLNodeType_Class, true),
			HXSLContainer(TextSpan(), nullptr, HXSLNodeType_Class, true),
			AccessModifiers(HXSLAccessModifier_Private)
		{
		}
		HXSLClass(TextSpan span, ASTNode* parent, HXSLAccessModifier access, TextSpan name)
			: HXSLType(span, parent, HXSLNodeType_Class, name),
			ASTNode(span, parent, HXSLNodeType_Class),
			HXSLContainer(span, parent, HXSLNodeType_Class),
			AccessModifiers(access)
		{
		}

		std::string DebugName() const override
		{
			return "[" + toString(type) + "] Name: " + Name.toString();
		}

		SymbolType GetSymbolType() const override
		{
			return HXSLSymbolType_Struct;
		}

		void Write(HXSLStream& stream) const override;

		void Read(HXSLStream& stream, StringPool& container) override;

		void Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<HXSLSymbolDef>>& nodes) override;

		DEFINE_GETTER_SETTER(HXSLAccessModifier, AccessModifiers)
	};

	class HXSLArray : public HXSLType
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
		size_t LookupIndex;

		AssemblySymbolRef() = default;

		AssemblySymbolRef(Assembly* TargetAssembly, const size_t& LookupIndex)
			: TargetAssembly(TargetAssembly), LookupIndex(LookupIndex)
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

	class HXSLNamespace : virtual public ASTNode, public HXSLContainer, public HXSLSymbolDef
	{
	private:
		std::vector<UsingDeclaration> Usings;
		std::vector<AssemblySymbolRef> References;
	public:
		HXSLNamespace()
			: HXSLSymbolDef(TextSpan(), nullptr, HXSLNodeType_Namespace, TextSpan(), true),
			ASTNode(TextSpan(), nullptr, HXSLNodeType_Namespace, true),
			HXSLContainer(TextSpan(), nullptr, HXSLNodeType_Namespace, true)
		{
		}
		HXSLNamespace(ASTNode* parent, const NamespaceDeclaration& declaration)
			: HXSLSymbolDef(declaration.Span, parent, HXSLNodeType_Namespace, declaration.Name),
			ASTNode(declaration.Span, parent, HXSLNodeType_Namespace),
			HXSLContainer(declaration.Span, parent, HXSLNodeType_Namespace)
		{
		}

		void AddUsing(UsingDeclaration _using)
		{
			Usings.push_back(_using);
		}

		std::vector<UsingDeclaration>& GetUsings() { return Usings; }

		const std::vector<AssemblySymbolRef>& GetAssemblyReferences() { return References; }

		virtual SymbolType GetSymbolType() const { return HXSLSymbolType_Namespace; }

		void Write(HXSLStream& stream) const override;

		void Read(HXSLStream& stream, StringPool& container) override;

		void Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<HXSLSymbolDef>>& nodes) override;

		void Warmup(const AssemblyCollection& references);
	};

	static std::unique_ptr<HXSLSymbolDef> CreateInstance(HXSLNodeType type)
	{
		switch (type)
		{
		case HXSLNodeType_Namespace:
			return std::make_unique<HXSLNamespace>();
		case HXSLNodeType_Field:
			return std::make_unique<HXSLField>();
		case HXSLNodeType_Function:
			return std::make_unique<HXSLFunction>();
		case HXSLNodeType_Struct:
			return std::make_unique<HXSLStruct>();
		case HXSLNodeType_Parameter:
			return std::make_unique<HXSLParameter>();
		case HXSLNodeType_DeclarationStatement:
			return std::make_unique<HXSLDeclarationStatement>();
		default:
			return nullptr;
		}
	}
}

#endif