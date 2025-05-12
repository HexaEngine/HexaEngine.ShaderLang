#ifndef AST_BASE_HPP
#define AST_BASE_HPP

#include "config.h"
#include "lang/language.hpp"
#include "lexical/token.hpp"
#include "lexical/text_span.hpp"
#include "lexical/input_stream.hpp"
#include "utils/string_pool.hpp"
#include "io/source_file.hpp"
#include "io/stream.hpp"
#include "macros.hpp"

#include <memory>
#include <vector>
#include <string>
#include <functional>

namespace HXSL
{
	class ASTNodeAdapter;
	class Compilation;
	class FunctionOverload;
	class OperatorOverload;
	class Struct;
	class Class;
	class Field;
	class Array;
	class Primitive;
	class Expression;
	class Statement;

	class BlockStatement;

	struct SymbolRef;
	class SymbolTable;

	class Assembly;
	class AssemblyCollection;

	/// <summary>
	/// Parsing: All Done.
	/// Symbol Collection: All Done.
	/// Symbol Resolve: All Done.
	/// Type Check: WIP.
	/// </summary>
	enum NodeType : int
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
		NodeType_Array,
		NodeType_Field,
		NodeType_IntrinsicFunction, // Placeholder (Will be added in the future.)
		NodeType_FunctionOverload,
		NodeType_OperatorOverload,
		NodeType_Constructor, // Placeholder (Will be added in the future.)
		NodeType_Parameter,
		NodeType_SwizzleDefinition,
		NodeType_AttributeDeclaration,

		NodeType_BlockStatement, // type-check: yes
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

		NodeType_EmptyExpression,
		NodeType_BinaryExpression, // type-check: yes
		NodeType_LiteralExpression, // type-check: yes
		NodeType_MemberReferenceExpression, // type-check: yes
		NodeType_FunctionCallExpression, // type-check: yes
		NodeType_FunctionCallParameter, // type-check: yes
		NodeType_MemberAccessExpression, // type-check: yes
		NodeType_IndexerAccessExpression, // type-check: yes
		NodeType_CastExpression, // type-check: yes
		NodeType_TernaryExpression, // type-check: yes
		NodeType_UnaryExpression, // type-check: yes
		NodeType_PrefixExpression, // type-check: yes
		NodeType_PostfixExpression, // type-check: yes
		NodeType_AssignmentExpression, // type-check: yes
		NodeType_InitializationExpression,

		NodeType_Count,
	};

	constexpr int NodeType_FirstStatement = NodeType_BlockStatement;
	constexpr int NodeType_LastStatement = NodeType_DefaultCaseStatement;
	constexpr int NodeType_StatementCount = NodeType_LastStatement - NodeType_FirstStatement;

	constexpr int NodeType_FirstExpression = NodeType_EmptyExpression;
	constexpr int NodeType_LastExpression = NodeType_InitializationExpression;
	constexpr int NodeType_ExpressionCount = NodeType_LastExpression - NodeType_FirstExpression;

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
		case NodeType_Array: return "NodeType_Array";
		case NodeType_Field: return "NodeType_Field";
		case NodeType_IntrinsicFunction: return "NodeType_IntrinsicFunction";
		case NodeType_FunctionOverload: return "NodeType_Function";
		case NodeType_OperatorOverload: return "NodeType_Operator";
		case NodeType_Constructor: return "NodeType_Constructor";
		case NodeType_Parameter: return "NodeType_Parameter";
		case NodeType_AttributeDeclaration: return "NodeType_AttributeDeclaration";
		case NodeType_BinaryExpression: return "NodeType_BinaryExpression";
		case NodeType_EmptyExpression: return "NodeType_EmptyExpression";
		case NodeType_LiteralExpression: return "NodeType_ConstantExpression";
		case NodeType_MemberReferenceExpression: return "NodeType_MemberReferenceExpression";
		case NodeType_FunctionCallExpression: return "NodeType_FunctionCallExpression";
		case NodeType_FunctionCallParameter: return "NodeType_FunctionCallParameter";
		case NodeType_MemberAccessExpression: return "NodeType_MemberAccessExpression";
		case NodeType_IndexerAccessExpression: return "NodeType_IndexerAccessExpression";
		case NodeType_BlockStatement: return "NodeType_BlockStatement";
		case NodeType_DeclarationStatement: return "NodeType_DeclarationStatement";
		case NodeType_AssignmentStatement: return "NodeType_AssignmentStatement";
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
		case NodeType_InitializationExpression: return "NodeType_InitializationExpression";
		default: return "Unknown NodeType";
		}
	}

	class ASTNode
	{
		friend class ASTNodeAdapter;
	private:
		Compilation* compilation = nullptr;
		size_t id = 0;

		void AddChild(ASTNode* node)
		{
			if (node == nullptr) return;
			children.push_back(node);
			node->parent = this;
			node->AssignId();
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
		NodeType type;
		bool isExtern;

		void AssignId();

		void RegisterChild(ASTNode* child)
		{
			if (child == nullptr) return;
			child->SetParent(this);
		}

		template<class T>
		void RegisterChild(const std::unique_ptr<T>& child)
		{
			RegisterChild(child.get());
		}

		template<class T>
		void RegisterChildren(const std::vector<std::unique_ptr<T>>& children)
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
		void UnregisterChild(const std::unique_ptr<T>& child)
		{
			UnregisterChild(child.get());
		}

		template<class T>
		void UnregisterChildren(const std::vector<std::unique_ptr<T>>& children)
		{
			for (auto& child : children)
			{
				UnregisterChild(child.get());
			}
		}

	public:
		ASTNode(TextSpan span, NodeType type, bool isExtern = false) : span(span), parent(nullptr), type(type), children({}), isExtern(isExtern)
		{
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
			return dynamic_cast<T*>(node);
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
			oss << "[" << ToString(type) << "] ID: " << id << " Span: " + span.str();
			return oss.str();
		}

		template <typename T>
		T* As() { return dynamic_cast<T*>(this); };

		virtual std::unique_ptr<ASTNode> Clone() const noexcept
		{
			return {};
		}

		template <class T>
		std::unique_ptr<T> CloneNode(const std::unique_ptr<T>& ptr) const noexcept
		{
			return ptr ? std::unique_ptr<T>(static_cast<T*>(ptr->Clone().release())) : nullptr;
		}

		template <typename T>
		std::vector<std::unique_ptr<T>> CloneNodes(const std::vector<std::unique_ptr<T>>& ptr) const noexcept
		{
			std::vector<std::unique_ptr<T>> result;
			for (auto& pt : ptr)
			{
				result.push_back(std::unique_ptr<T>(static_cast<T*>(ptr->Clone().release())));
			}
			return result;
		}

		template <typename T>
		std::unique_ptr<T> CloneExtern(const std::unique_ptr<T>& ptr) const noexcept
		{
			return ptr ? std::unique_ptr<T>(static_cast<T*>(ptr->Clone())) : nullptr;
		}

		virtual ~ASTNode()
		{
		}
	};

	template <class Target, class Injector, class InsertFunc>
	static void	InjectNode(std::unique_ptr<Target>& target, std::unique_ptr<Injector> inject, InsertFunc func)
	{
		static_assert(std::is_base_of<ASTNode, Target>::value, "Target must derive from ASTNode");
		static_assert(std::is_base_of<Target, Injector>::value, "Injector must derive from Target");

		auto parent = target->GetParent();
		inject->SetParent(parent);
		target->SetParent(inject.get());

		(inject.get()->*func)(std::move(target));

		target = std::move(inject);
	}
}

#endif