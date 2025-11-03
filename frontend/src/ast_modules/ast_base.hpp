#ifndef AST_BASE_HPP
#define AST_BASE_HPP

#include "pch/std.hpp"

#include "utils/ast_allocator.hpp"

#include "config.h"
#include "lang/language.hpp"
#include "lexical/token.hpp"
#include "lexical/text_span.hpp"
#include "lexical/identifier_table.hpp"
#include "utils/iterator_range.hpp"
#include "utils/string_pool.hpp"
#include "utils/rtti_helper.hpp"
#include "utils/trailing_objects.hpp"
#include "io/stream.hpp"
#include "macros.hpp"

namespace HXSL
{
	class ASTNodeAdapter;
	class CompilationUnit;
	class FunctionOverload;
	class ConstructorOverload;
	class OperatorOverload;
	class Struct;
	class Class;
	class Field;
	class ArrayDecl;
	class Primitive;
	class Expression;
	class ASTNode;

	class BlockStatement;

	class ChainExpression;

	class SymbolTable;
	class SymbolDef;
	class SymbolRef;

	class TypeContainer;

	class Assembly;
	class AssemblyCollection;
	class ASTContext;

	/// <summary>
	/// Parsing: All Done.
	/// Symbol Collection: All Done.
	/// Symbol Resolve: All Done.
	/// Type Check: All Done.
	/// Code Gen: Partially.
	/// </summary>
	enum NodeType : uint8_t
	{
		/// <summary>
		/// Special node and does not exist as class.
		/// </summary>
		NodeType_Unknown,
		NodeType_CompilationUnit,
		NodeType_Namespace,
		NodeType_UsingDecl,
		NodeType_Enum, // Placeholder (Will be added in the future.)
		NodeType_Primitive,
		NodeType_Struct,
		NodeType_Class, // Needs parsing logic
		NodeType_Array,
		NodeType_Pointer,
		NodeType_Field,
		NodeType_IntrinsicFunction, // Placeholder (Will be added in the future.)
		NodeType_FunctionOverload,
		NodeType_OperatorOverload,
		NodeType_ConstructorOverload,
		NodeType_Parameter,
		NodeType_ThisDef, // Needs attention.
		NodeType_SwizzleDefinition,
		NodeType_AttributeDeclaration,

		NodeType_BlockStatement, // type-check: done
		NodeType_DeclarationStatement, // type-check: done
		NodeType_AssignmentStatement, // type-check: done
		NodeType_CompoundAssignmentStatement, // type-check: done
		NodeType_ExpressionStatement, // type-check: done
		NodeType_ReturnStatement, // type-check: done
		NodeType_IfStatement, // type-check: done
		NodeType_ElseStatement, // type-check: done
		NodeType_ElseIfStatement, // type-check: done
		NodeType_WhileStatement, // type-check: done
		NodeType_DoWhileStatement,
		NodeType_ForStatement, // type-check: done
		NodeType_BreakStatement, // type-check: done
		NodeType_ContinueStatement, // type-check: done
		NodeType_DiscardStatement, // type-check: done
		NodeType_SwitchStatement,
		NodeType_CaseStatement,
		NodeType_DefaultCaseStatement, // type-check: done

		NodeType_EmptyExpression, // type-check: done
		NodeType_BinaryExpression, // type-check: yes
		NodeType_LiteralExpression, // type-check: yes
		NodeType_MemberReferenceExpression, // type-check: yes
		NodeType_FunctionCallExpression, // type-check: yes
		NodeType_FunctionCallParameter, // type-check: yes
		NodeType_MemberAccessExpression, // type-check: yes
		NodeType_IndexerAccessExpression, // type-check: yes
		NodeType_CastExpression, // type-check: yes
		NodeType_TernaryExpression, // type-check: yes
		NodeType_PrefixExpression, // type-check: yes
		NodeType_PostfixExpression, // type-check: yes
		NodeType_AssignmentExpression, // type-check: yes
		NodeType_CompoundAssignmentExpression, // type-check: yes
		NodeType_InitializationExpression,

		NodeType_Count,
	};

	constexpr int NodeType_FirstStatement = NodeType_BlockStatement;
	constexpr int NodeType_LastStatement = NodeType_DefaultCaseStatement;
	constexpr int NodeType_StatementCount = NodeType_LastStatement - NodeType_FirstStatement + 1;

	constexpr int NodeType_FirstExpression = NodeType_EmptyExpression;
	constexpr int NodeType_LastExpression = NodeType_InitializationExpression;
	constexpr int NodeType_ExpressionCount = NodeType_LastExpression - NodeType_FirstExpression + 1;

	static bool IsDataType(NodeType nodeType)
	{
		switch (nodeType) {
		case NodeType_Primitive:
		case NodeType_Struct:
		case NodeType_Class:
		case NodeType_Enum:
			return true;
		default:
			return false;
		}
	}

	static bool IsExpressionType(NodeType nodeType)
	{
		switch (nodeType) {
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
		case NodeType_ExpressionStatement:
		case NodeType_ReturnStatement:
		case NodeType_IfStatement:
		case NodeType_ElseStatement:
		case NodeType_ElseIfStatement:
		case NodeType_WhileStatement:
		case NodeType_DoWhileStatement:
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
		case NodeType_Unknown: return "Unknown";
		case NodeType_CompilationUnit: return "Compilation";
		case NodeType_Namespace: return "Namespace";
		case NodeType_Enum: return "Enum";
		case NodeType_Primitive: return "Primitive";
		case NodeType_Struct: return "Struct";
		case NodeType_Class: return "Class";
		case NodeType_Array: return "Array";
		case NodeType_Field: return "Field";
		case NodeType_IntrinsicFunction: return "Intrinsic";
		case NodeType_FunctionOverload: return "Function";
		case NodeType_OperatorOverload: return "Operator";
		case NodeType_ConstructorOverload: return "Constructor";
		case NodeType_Parameter: return "Parameter";
		case NodeType_AttributeDeclaration: return "AttributeDeclaration";
		case NodeType_BinaryExpression: return "BinaryExpression";
		case NodeType_EmptyExpression: return "EmptyExpression";
		case NodeType_LiteralExpression: return "ConstantExpression";
		case NodeType_MemberReferenceExpression: return "MemberReferenceExpression";
		case NodeType_FunctionCallExpression: return "FunctionCallExpression";
		case NodeType_FunctionCallParameter: return "FunctionCallParameter";
		case NodeType_MemberAccessExpression: return "MemberAccessExpression";
		case NodeType_IndexerAccessExpression: return "IndexerAccessExpression";
		case NodeType_BlockStatement: return "BlockStatement";
		case NodeType_DeclarationStatement: return "DeclarationStatement";
		case NodeType_AssignmentStatement: return "AssignmentStatement";
		case NodeType_CompoundAssignmentStatement: return "CompoundAssignmentStatement";
		case NodeType_ExpressionStatement: return "ExpressionStatement";
		case NodeType_ReturnStatement: return "ReturnStatement";
		case NodeType_IfStatement: return "IfStatement";
		case NodeType_ElseStatement: return "ElseStatement";
		case NodeType_ElseIfStatement: return "ElseIfStatement";
		case NodeType_WhileStatement: return "WhileStatement";
		case NodeType_DoWhileStatement: return "DoWhileStatement";
		case NodeType_ForStatement: return "ForStatement";
		case NodeType_BreakStatement: return "BreakStatement";
		case NodeType_ContinueStatement: return "ContinueStatement";
		case NodeType_DiscardStatement: return "DiscardStatement";
		case NodeType_SwitchStatement: return "SwitchStatement";
		case NodeType_CaseStatement: return "CaseStatement";
		case NodeType_DefaultCaseStatement: return "DefaultCaseStatement";
		case NodeType_CastExpression: return "CastExpression";
		case NodeType_TernaryExpression: return "TernaryExpression";
		case NodeType_PrefixExpression: return "PrefixExpression";
		case NodeType_PostfixExpression: return "PostfixExpression";
		case NodeType_AssignmentExpression: return "AssignmentExpression";
		case NodeType_CompoundAssignmentExpression: return "CompoundAssignmentExpression";
		case NodeType_InitializationExpression: return "InitializationExpression";
		default: return "Unknown NodeType";
		}
	}

	class ASTNode
	{
		friend class ASTNodeAdapter;
	private:
		void AddChild(ASTNode* node)
		{
			if (node == nullptr) return;
			children.push_back(node);
			node->parent = this;
		}

		void RemoveChild(ASTNode* node)
		{
			if (node == nullptr) return;
			children.erase(remove(children.begin(), children.end(), node), children.end());
			node->parent = nullptr;
		}

	protected:
		std::vector<ASTNode*> children;
		TextSpan span;
		ASTNode* parent;
		NodeType type : 6;
		bool isExtern : 1;

		void RegisterChild(ASTNode* child)
		{
			if (child == nullptr) return;
			child->SetParent(this);
		}

		template<class T>
		void RegisterChild(const ast_ptr<T>& child)
		{
			RegisterChild(child.get());
		}

		template<class T>
		void RegisterChildren(const std::vector<ast_ptr<T>>& children)
		{
			for (auto& child : children)
			{
				RegisterChild(child.get());
			}
		}

		void UnregisterChild(ASTNode* child)
		{
			if (child == nullptr) return;
			child->SetParent(nullptr);
		}

		template<class T>
		void UnregisterChild(const ast_ptr<T>& child)
		{
			UnregisterChild(child.get());
		}

		template<class T>
		void UnregisterChildren(const std::vector<ast_ptr<T>>& children)
		{
			for (auto& child : children)
			{
				UnregisterChild(child.get());
			}
		}

	public:
		using child_iterator = ASTNode**;
		using child_range = iterator_range<child_iterator>;

		ASTNode(const TextSpan& span, NodeType type, bool isExtern = false) : span(span), parent(nullptr), type(type), children({}), isExtern(isExtern)
		{
		}

		child_range GetChildrenIt();

		bool IsExtern() const
		{
			return isExtern;
		}

		ASTNode* GetParent() const noexcept { return parent; }
		void SetParent(ASTNode* newParent) noexcept
		{
			if (parent == newParent) return;
			if (newParent == this)
			{
#if HXSL_DEBUG
				HXSL_ASSERT(false, "This should never happen.");
#endif
				return;
			}

			if (parent)
			{
				parent->RemoveChild(this);
			}
			parent = newParent;
			if (newParent)
			{
				newParent->AddChild(this);
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

		ASTNode* FindAncestor(const std::unordered_set<NodeType>& types, size_t maxDepth = std::numeric_limits<size_t>::max()) const noexcept
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

		template<typename T>
		T* FindAncestor(const std::unordered_set<NodeType>& types, size_t maxDepth = std::numeric_limits<size_t>::max()) const noexcept
		{
			auto node = FindAncestor(types, maxDepth);
			if (!node)
			{
				return nullptr;
			}
			return dyn_cast<T>(node);
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
			return dyn_cast<T>(node);
		}

		std::string DebugName() const;

		virtual ~ASTNode()
		{
		}
	};

	template <class Target, class Injector, class InsertFunc>
	static void	InjectNode(Target& target, Injector inject, InsertFunc func)
	{
		using TargetType = std::remove_pointer_t<Target>;
		using InjectorType = std::remove_pointer_t<Injector>;
		static_assert(std::is_base_of<ASTNode, TargetType>::value, "Target must derive from ASTNode");
		static_assert(std::is_base_of<TargetType, InjectorType>::value, "Injector must derive from Target");

		auto parent = target->GetParent();
		inject->SetParent(parent);
		target->SetParent(inject);

		(inject->*func)(target);

		target = inject;
	}

	template<typename T>
	inline static bool isa(const ASTNode* node) { return node && node->GetType() == T::ID; }

	using symbol_def_checker = rtti_type_equals_checker<
		NodeType_Namespace,
		NodeType_Enum,
		NodeType_Primitive,
		NodeType_Struct,
		NodeType_Class,
		NodeType_Array,
		NodeType_Pointer,
		NodeType_Field,
		NodeType_IntrinsicFunction,
		NodeType_FunctionOverload,
		NodeType_OperatorOverload,
		NodeType_ConstructorOverload,
		NodeType_ThisDef,
		NodeType_SwizzleDefinition,
		NodeType_DeclarationStatement,
		NodeType_Parameter>;

	template<>
	inline static bool isa<SymbolDef>(const ASTNode* node) { return node && symbol_def_checker::check(node->GetType()); }

	using chain_expr_checker = rtti_type_equals_checker<
		NodeType_MemberAccessExpression,
		NodeType_MemberReferenceExpression,
		NodeType_FunctionCallExpression,
		NodeType_IndexerAccessExpression
	>;

	template<>
	inline static bool isa<ChainExpression>(const ASTNode* node) { return node && chain_expr_checker::check(node->GetType()); }

	using function_ovl_checker = rtti_type_equals_checker<
		NodeType_FunctionOverload,
		NodeType_ConstructorOverload,
		NodeType_OperatorOverload
	>;

	template<>
	inline static bool isa<FunctionOverload>(const ASTNode* node) { return node && function_ovl_checker::check(node->GetType()); }

	using type_container_checker = rtti_type_equals_checker<
		NodeType_Struct,
		NodeType_Class
	>;

	template<>
	inline static bool isa<TypeContainer>(const ASTNode* node) { return node && type_container_checker::check(node->GetType()); }

	template<typename T>
	inline static T* dyn_cast(ASTNode* node) { return isa<T>(node) ? static_cast<T*>(node) : nullptr; }

	template<typename T>
	inline static const T* dyn_cast(const ASTNode* node) { return isa<T>(node) ? static_cast<const T*>(node) : nullptr; }

	template <typename T>
	inline static T* cast(ASTNode* N) { assert(isa<T>(N) && "cast<T>() argument is not of type T!"); return static_cast<T*>(N); }

	template <typename T>
	inline static const T* cast(const ASTNode* N) { assert(isa<T>(N) && "cast<T>() argument is not of type T!"); return static_cast<const T*>(N); }
}

#endif