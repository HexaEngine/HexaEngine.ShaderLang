#ifndef STATEMENTS_HPP
#define STATEMENTS_HPP

#include "ast_base.hpp"
#include "symbol_base.hpp"
#include "interfaces.hpp"
#include "attributes.hpp"
#include "expressions.hpp"

namespace HXSL
{
	class StatementContainer : public ASTNode, public TrailingObjects<StatementContainer, ASTNode*>
	{
	protected:
		TrailingObjStorage<StatementContainer, uint32_t> storage;

		StatementContainer(const TextSpan& span, NodeType type, bool isExtern = false) : ASTNode(span, type, isExtern) {}

	public:
		DEFINE_TRAILING_OBJ_SPAN_GETTER(GetStatements, 0, storage);
	};

	class BlockStatement : public ASTNode, public TrailingObjects<StatementContainer, ASTNode*>
	{
		friend class ASTContext;
	private:
		TrailingObjStorage<BlockStatement, uint32_t> storage;

		BlockStatement(const TextSpan& span)
			: ASTNode(span, NodeType_BlockStatement)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_BlockStatement;
		static BlockStatement* Create(const TextSpan& span, const ArrayRef<ASTNode*>& statements);

		DEFINE_TRAILING_OBJ_SPAN_GETTER(GetStatements, 0, storage);

		std::string DebugName() const
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "]";
			return oss.str();
		}
	};

	class BodyStatement : public ASTNode
	{
	protected:
		BlockStatement* body;

		BodyStatement(const TextSpan& span, NodeType type, bool isExtern = false)
			: ASTNode(span, type, isExtern), body(nullptr)
		{
		}

		BodyStatement(const TextSpan& span, NodeType type, BlockStatement* body)
			: ASTNode(span, type, false),
			body(body)
		{
			REGISTER_CHILD(body);
		}

	public:
		BlockStatement* GetBody() const noexcept
		{
			return body;
		}

		void SetBody(BlockStatement* value) noexcept
		{
			UnregisterChild(body); body = value; RegisterChild(body);
		}

		BlockStatement*& GetBodyMut() noexcept
		{
			return body;
		};
	};

	class ConditionalStatement : public BodyStatement, public IHasExpressions
	{
	protected:
		Expression* condition;

		ConditionalStatement(const TextSpan& span, NodeType type, bool isExtern = false)
			: BodyStatement(span, type, isExtern), condition(nullptr)
		{
		}

		ConditionalStatement(const TextSpan& span, NodeType type, Expression* condition, BlockStatement* body)
			: BodyStatement(span, type, body),
			condition(condition)
		{
			REGISTER_EXPR(condition);
		}

	public:
		DEFINE_GET_SET_MOVE_REG_EXPR(Expression*, Condition, condition);
	};

	class DeclarationStatement : public SymbolDef, public IHasExpressions
	{
		friend class ASTContext;
	private:
		StorageClass storageClass;
		SymbolRef* symbol;
		Expression* initializer;

		DeclarationStatement(const TextSpan& span, IdentifierInfo* name, SymbolRef* symbol, StorageClass storageClass, Expression* initializer)
			: SymbolDef(span, ID, name),
			storageClass(storageClass),
			symbol(symbol),
			initializer(initializer)
		{
			REGISTER_EXPR(initializer);
		}

	public:
		static constexpr NodeType ID = NodeType_DeclarationStatement;
		static DeclarationStatement* Create(const TextSpan& span, IdentifierInfo* name, SymbolRef* symbol, StorageClass storageClass, Expression* initializer);

		SymbolRef* GetSymbolRef()
		{
			return symbol;
		}

		SymbolDef* GetDeclaredType() const noexcept
		{
			return symbol->GetDeclaration();
		}

		const StorageClass& GetStorageClass() const noexcept
		{
			return storageClass;
		}

		DEFINE_GETTER_SETTER_PTR(SymbolRef*, Symbol, symbol)

			DEFINE_GET_SET_MOVE_REG_EXPR(Expression*, Initializer, initializer)
	};

	class AssignmentStatement : public ASTNode, public IHasExpressions
	{
		friend class ASTContext;
	private:

	protected:
		AssignmentExpression* expr;

		AssignmentStatement(const TextSpan& span, NodeType type, AssignmentExpression* expr)
			: ASTNode(span, type),
			expr(expr)
		{
			REGISTER_EXPR(expr);
		}

	public:
		static constexpr NodeType ID = NodeType_AssignmentStatement;
		static AssignmentStatement* Create(const TextSpan& span, Expression* target, Expression* expression);

		AssignmentExpression* GetAssignmentExpression() const noexcept
		{
			return expr;
		}

		Expression* GetTarget() const noexcept
		{
			return expr->GetTarget();
		}

		void SetTarget(Expression* value) noexcept
		{
			expr->SetTarget(value);
		}

		Expression* GetExpression() const noexcept
		{
			return expr->GetExpression();
		}

		void SetExpression(Expression* value) noexcept
		{
			expr->SetExpression(value);
		}
	};

	class CompoundAssignmentStatement : public AssignmentStatement
	{
		friend class ASTContext;
	private:
		CompoundAssignmentStatement(const TextSpan& span, Operator op, CompoundAssignmentExpression* expression)
			: AssignmentStatement(span, ID, expression)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_CompoundAssignmentStatement;
		static CompoundAssignmentStatement* Create(const TextSpan& span, Operator op, Expression* target, Expression* expression);

		const Operator& GetOperator() const noexcept
		{
			return expr->GetOperator();
		}

		void SetOperator(const Operator& value) noexcept
		{
			expr->SetOperator(value);
		}
	};

	class ExpressionStatement : public ASTNode, public IHasExpressions
	{
		friend class ASTContext;
	private:
		Expression* expression;

		ExpressionStatement(const TextSpan& span, Expression* expression)
			: ASTNode(span, ID),
			expression(expression)
		{
			REGISTER_EXPR(expression);
		}

	public:
		static constexpr NodeType ID = NodeType_ExpressionStatement;
		static ExpressionStatement* Create(const TextSpan& span, Expression* expression);

		DEFINE_GET_SET_MOVE_REG_EXPR(Expression*, Expression, expression);
	};

	class ReturnStatement : public ASTNode, public IHasExpressions
	{
		friend class ASTContext;
	private:
		Expression* returnValueExpression;

		ReturnStatement(const TextSpan& span, Expression* returnValueExpression)
			: ASTNode(span, ID),
			returnValueExpression(returnValueExpression)
		{
			REGISTER_EXPR(returnValueExpression);
		}

	public:
		static constexpr NodeType ID = NodeType_ReturnStatement;
		static ReturnStatement* Create(const TextSpan& span, Expression* returnValueExpression);

		DEFINE_GET_SET_MOVE_REG_EXPR(Expression*, ReturnValueExpression, returnValueExpression)
	};

	class ElseStatement : public BodyStatement
	{
		friend class ASTContext;
	private:
		ElseStatement(const TextSpan& span, BlockStatement* body)
			: BodyStatement(span, ID, body)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_ElseStatement;
		static ElseStatement* Create(const TextSpan& span, BlockStatement* body);

		std::string DebugName() const
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "]";
			return oss.str();
		}
	};

	class ElseIfStatement : public ConditionalStatement
	{
		friend class ASTContext;
	private:
		ElseIfStatement(const TextSpan& span, Expression* condition, BlockStatement* body)
			: ConditionalStatement(span, ID, condition, body)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_ElseIfStatement;
		static ElseIfStatement* Create(const TextSpan& span, Expression* condition, BlockStatement* body);

		std::string DebugName() const
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] Condition: " + condition->GetSpan().str();
			return oss.str();
		}
	};

	class IfStatement : public ConditionalStatement, public TrailingObjects<IfStatement, ElseIfStatement*, AttributeDecl*>
	{
		friend class ASTContext;
	private:
		ElseStatement* elseStatement;
		TrailingObjStorage<IfStatement, uint32_t> storage;

		IfStatement(const TextSpan& span, Expression* condition, BlockStatement* body, ElseStatement* elseStatement)
			: ConditionalStatement(span, ID, condition, body)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_IfStatement;
		static IfStatement* Create(const TextSpan& span, Expression* condition, BlockStatement* body, const ArrayRef<ElseIfStatement*>& elseIfStatements, ElseStatement* elseStatement, const ArrayRef<AttributeDecl*>& attributes);
		static IfStatement* Create(const TextSpan& span, Expression* condition, BlockStatement* body, uint32_t numElseIfStatements, ElseStatement* elseStatement, uint32_t numAttributes);

		DEFINE_TRAILING_OBJ_SPAN_GETTER(GetElseIfStatements, 0, storage);

		DEFINE_GET_SET_MOVE_CHILD(ElseStatement*, ElseStatement, elseStatement);

		std::string DebugName() const
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] Condition: " + condition->GetSpan().str();
			return oss.str();
		}
	};

	class CaseStatement : public StatementContainer, public IHasExpressions
	{
		friend class ASTContext;
	private:
		Expression* expression;

		CaseStatement(const TextSpan& span, Expression* expression)
			: StatementContainer(span, ID),
			expression(expression)
		{
			REGISTER_EXPR(expression);
		}

	public:
		static constexpr NodeType ID = NodeType_CaseStatement;
		static CaseStatement* Create(const TextSpan& span, Expression* expression, const ArrayRef<ASTNode*>& statements);
		static CaseStatement* Create(const TextSpan& span, Expression* expression, uint32_t numStatements);

		DEFINE_GET_SET_MOVE_REG_EXPR(Expression*, Expression, expression);

		std::string DebugName() const
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] Header: " + expression->GetSpan().str();
			return oss.str();
		}
	};

	class DefaultCaseStatement : public StatementContainer
	{
		friend class ASTContext;
	private:
		DefaultCaseStatement(const TextSpan& span)
			: StatementContainer(span, ID)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_DefaultCaseStatement;
		static DefaultCaseStatement* Create(const TextSpan& span, const ArrayRef<ASTNode*>& statements);
		static DefaultCaseStatement* Create(const TextSpan& span, uint32_t numStatements);
	};

	class SwitchStatement : public ASTNode, public IHasExpressions, public TrailingObjects<SwitchStatement, CaseStatement*, AttributeDecl*>
	{
		friend class ASTContext;
	private:
		Expression* expression;
		DefaultCaseStatement* defaultCase;
		TrailingObjStorage<SwitchStatement, uint32_t> storage;

		SwitchStatement(const TextSpan& span, Expression* expression, DefaultCaseStatement* defaultCase)
			: ASTNode(span, ID),
			expression(expression),
			defaultCase(defaultCase)
		{
			REGISTER_EXPR(expression);
			REGISTER_CHILD(defaultCase);
		}

	public:
		static constexpr NodeType ID = NodeType_SwitchStatement;
		static SwitchStatement* Create(const TextSpan& span, Expression* expression, const ArrayRef<CaseStatement*>& cases, DefaultCaseStatement* defaultCase, const ArrayRef<AttributeDecl*>& attributes);
		static SwitchStatement* Create(const TextSpan& span, Expression* expression, uint32_t numCases, DefaultCaseStatement* defaultCase, uint32_t numAttributes);

		DEFINE_TRAILING_OBJ_SPAN_GETTER(GetCases, 0, storage);

		DEFINE_GET_SET_MOVE_REG_EXPR(Expression*, Expression, expression);

		DEFINE_GET_SET_MOVE_CHILD(DefaultCaseStatement*, DefaultCase, defaultCase);

		std::string DebugName() const
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] Header: " + expression->GetSpan().str();
			return oss.str();
		}
	};

	class ForStatement : public ConditionalStatement, public TrailingObjects<ForStatement, AttributeDecl*>
	{
		friend class ASTContext;
	private:
		ASTNode* init;
		Expression* iteration;
		TrailingObjStorage<ForStatement, uint32_t> storage;

		ForStatement(const TextSpan& span, ASTNode* init, Expression* condition, Expression* iteration, BlockStatement* body)
			: ConditionalStatement(span, ID, condition, body),
			init(init),
			iteration(iteration)
		{
			REGISTER_CHILD(init);
			REGISTER_EXPR(iteration);
		}

	public:
		static constexpr NodeType ID = NodeType_ForStatement;
		static ForStatement* Create(const TextSpan& span, ASTNode* init, Expression* condition, Expression* iteration, BlockStatement* body, const ArrayRef<AttributeDecl*>& attributes);
		static ForStatement* Create(const TextSpan& span, ASTNode* init, Expression* condition, Expression* iteration, BlockStatement* body, uint32_t numAttributes);

		DEFINE_TRAILING_OBJ_SPAN_GETTER(GetAttributes, 0, storage);
		DEFINE_GET_SET_MOVE_CHILD(ASTNode*, Init, init);
		DEFINE_GET_SET_MOVE_REG_EXPR(Expression*, Iteration, iteration);

		std::string DebugName() const
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] Header: " + init->GetSpan().merge(condition->GetSpan()).merge(iteration->GetSpan()).str();
			return oss.str();
		}
	};

	class BreakStatement : public ASTNode
	{
		friend class ASTContext;
	private:
		BreakStatement(const TextSpan& span)
			: ASTNode(span, ID)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_BreakStatement;
		static BreakStatement* Create(const TextSpan& span);
	};

	class ContinueStatement : public ASTNode
	{
		friend class ASTContext;
	private:
		ContinueStatement(const TextSpan& span)
			: ASTNode(span, ID)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_ContinueStatement;
		static ContinueStatement* Create(const TextSpan& span);
	};

	class DiscardStatement : public ASTNode
	{
		friend class ASTContext;
	private:
		DiscardStatement(const TextSpan& span)
			: ASTNode(span, ID)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_DiscardStatement;
		static DiscardStatement* Create(const TextSpan& span);
	};

	class WhileStatement : public ConditionalStatement, public TrailingObjects<WhileStatement, AttributeDecl*>
	{
		friend class ASTContext;
	private:
		TrailingObjStorage<WhileStatement, uint32_t> storage;

		WhileStatement(const TextSpan& span, Expression* condition, BlockStatement* body)
			: ConditionalStatement(span, ID, condition, body)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_WhileStatement;
		static WhileStatement* Create(const TextSpan& span, Expression* condition, BlockStatement* body, const ArrayRef<AttributeDecl*>& attributes);
		static WhileStatement* Create(const TextSpan& span, Expression* condition, BlockStatement* body, uint32_t numAttributes);

		std::string DebugName() const
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] Condition: " + condition->GetSpan().str();
			return oss.str();
		}
	};

	class DoWhileStatement : public ConditionalStatement, public TrailingObjects<WhileStatement, AttributeDecl*>
	{
		friend class ASTContext;
	private:
		TrailingObjStorage<DoWhileStatement, uint32_t> storage;

		DoWhileStatement(const TextSpan& span, Expression* condition, BlockStatement* body)
			: ConditionalStatement(span, ID, condition, body)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_DoWhileStatement;
		static DoWhileStatement* Create(const TextSpan& span, Expression* condition, BlockStatement* body, const ArrayRef<AttributeDecl*>& atttributes);
		static DoWhileStatement* Create(const TextSpan& span, Expression* condition, BlockStatement* body, uint32_t numAttributes);

		std::string DebugName() const
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] Condition: " + condition->GetSpan().str();
			return oss.str();
		}
	};
}

#endif