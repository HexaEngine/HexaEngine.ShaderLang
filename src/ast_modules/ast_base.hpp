#ifndef AST_BASE_HPP
#define AST_BASE_HPP

#include "config.h"
#include "lang/language.h"
#include "lexical/token.h"
#include "utils/text_span.h"
#include "utils/string_pool.hpp"
#include "io/stream.h"
#include "macros.hpp"

#include <memory>
#include <vector>
#include <string>

namespace HXSL
{
	class Compilation;
	class FunctionOverload;
	class OperatorOverload;
	class Struct;
	class Class;
	class Field;
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
}

#endif