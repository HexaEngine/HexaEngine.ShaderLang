#ifndef STATEMENTS_HPP
#define STATEMENTS_HPP

#include "ast_base.hpp"
#include "symbol_base.hpp"
#include "interfaces.hpp"
#include "attributes.hpp"
#include "expressions.hpp"

namespace HXSL
{
	class StatementContainer : public ASTNode, protected TrailingObjects<StatementContainer, ast_ptr<ASTNode>>
	{
	protected:
		uint32_t numStatements;

		StatementContainer(const TextSpan& span, NodeType type, bool isExtern = false) : ASTNode(span, type, isExtern) {}

	public:
		ArrayRef<ast_ptr<ASTNode>> GetStatements()
		{
			return { GetTrailingObjects<0>(numStatements), numStatements };
		}
	};

	class BlockStatement : public StatementContainer
	{
		friend class ASTContext;
	private:
		BlockStatement(const TextSpan& span)
			: StatementContainer(span, NodeType_BlockStatement)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_BlockStatement;
		static BlockStatement* Create(ASTContext* context, const TextSpan& span, ArrayRef<ast_ptr<ASTNode>>& statements);
		static BlockStatement* Create(ASTContext* context, const TextSpan& span, uint32_t numStatements);

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
		ast_ptr<BlockStatement> body;

		BodyStatement(const TextSpan& span, NodeType type, bool isExtern = false)
			: ASTNode(span, type, isExtern)
		{
		}

		BodyStatement(const TextSpan& span, NodeType type, ast_ptr<BlockStatement>&& body)
			: ASTNode(span, type, false),
			body(std::move(body))
		{
			REGISTER_CHILD(body);
		}

	public:
		const ast_ptr<BlockStatement>& GetBody() const noexcept
		{
			return body;
		}

		void SetBody(ast_ptr<BlockStatement>&& value) noexcept
		{
			UnregisterChild(body.get()); body = std::move(value); RegisterChild(body.get());
		}

		ast_ptr<BlockStatement>& GetBodyMut() noexcept
		{
			return body;
		};
	};

	class ConditionalStatement : public BodyStatement, public IHasExpressions
	{
	protected:
		ast_ptr<Expression> condition;

		ConditionalStatement(const TextSpan& span, NodeType type, bool isExtern = false)
			: BodyStatement(span, type, isExtern)
		{
		}

		ConditionalStatement(const TextSpan& span, NodeType type, ast_ptr<Expression>&& condition, ast_ptr<BlockStatement>&& body)
			: BodyStatement(span, type, std::move(body)),
			condition(std::move(condition))
		{
			REGISTER_EXPR(condition);
		}

	public:
		DEFINE_GET_SET_MOVE_REG_EXPR(ast_ptr<Expression>, Condition, condition);
	};

	class DeclarationStatement : public SymbolDef, public IHasExpressions
	{
		friend class ASTContext;
	private:
		StorageClass storageClass;
		ast_ptr<SymbolRef> symbol;
		ast_ptr<Expression> initializer;

		DeclarationStatement(const TextSpan& span, IdentifierInfo* name, ast_ptr<SymbolRef>&& symbol, StorageClass storageClass, ast_ptr<Expression>&& initializer)
			: SymbolDef(span, ID, name),
			storageClass(storageClass),
			symbol(std::move(symbol)),
			initializer(std::move(initializer))
		{
			REGISTER_EXPR(initializer);
		}

	public:
		static constexpr NodeType ID = NodeType_DeclarationStatement;
		static DeclarationStatement* Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, ast_ptr<SymbolRef>&& symbol, StorageClass storageClass, ast_ptr<Expression>&& initializer);

		ast_ptr<SymbolRef>& GetSymbolRef()
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

		bool IsConstant() const override
		{
			return (storageClass & StorageClass_Const) != 0;
		}

		DEFINE_GET_SET_MOVE(ast_ptr<SymbolRef>, Symbol, symbol)

			DEFINE_GET_SET_MOVE_REG_EXPR(ast_ptr<Expression>, Initializer, initializer)
	};

	class AssignmentStatement : public ASTNode, public IHasExpressions
	{
		friend class ASTContext;
	private:

	protected:
		ast_ptr<AssignmentExpression> expr;

		AssignmentStatement(const TextSpan& span, NodeType type, ast_ptr<AssignmentExpression>&& expr)
			: ASTNode(span, type),
			expr(std::move(expr))
		{
			REGISTER_EXPR(expr);
		}

	public:
		static constexpr NodeType ID = NodeType_AssignmentStatement;
		static AssignmentStatement* Create(ASTContext* context, const TextSpan& span, ast_ptr<Expression>&& target, ast_ptr<Expression>&& expression);

		const ast_ptr<AssignmentExpression>& GetAssignmentExpression() const noexcept
		{
			return expr;
		}

		const ast_ptr<Expression>& GetTarget() const noexcept
		{
			return expr->GetTarget();
		}

		void SetTarget(ast_ptr<Expression>&& value) noexcept
		{
			expr->SetTarget(std::move(value));
		}

		const ast_ptr<Expression>& GetExpression() const noexcept
		{
			return expr->GetExpression();
		}

		void SetExpression(ast_ptr<Expression>&& value) noexcept
		{
			expr->SetExpression(std::move(value));
		}
	};

	class CompoundAssignmentStatement : public AssignmentStatement
	{
		friend class ASTContext;
	private:
		CompoundAssignmentStatement(const TextSpan& span, Operator op, ast_ptr<CompoundAssignmentExpression>&& expression)
			: AssignmentStatement(span, ID, std::move(expression))
		{
		}

	public:
		static constexpr NodeType ID = NodeType_CompoundAssignmentStatement;
		static CompoundAssignmentStatement* Create(ASTContext* context, const TextSpan& span, Operator op, ast_ptr<Expression>&& target, ast_ptr<Expression>&& expression);

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
		ast_ptr<Expression> expression;

		ExpressionStatement(const TextSpan& span, ast_ptr<Expression>&& expression)
			: ASTNode(span, ID),
			expression(std::move(expression))
		{
			REGISTER_EXPR(expression);
		}

	public:
		static constexpr NodeType ID = NodeType_ExpressionStatement;
		static ExpressionStatement* Create(ASTContext* context, const TextSpan& span, ast_ptr<Expression>&& expression);

		DEFINE_GET_SET_MOVE_REG_EXPR(ast_ptr<Expression>, Expression, expression);
	};

	class ReturnStatement : public ASTNode, public IHasExpressions
	{
		friend class ASTContext;
	private:
		ast_ptr<Expression> returnValueExpression;

		ReturnStatement(const TextSpan& span, ast_ptr<Expression>&& returnValueExpression)
			: ASTNode(span, ID),
			returnValueExpression(std::move(returnValueExpression))
		{
			REGISTER_EXPR(returnValueExpression);
		}

	public:
		static constexpr NodeType ID = NodeType_ReturnStatement;
		static ReturnStatement* Create(ASTContext* context, const TextSpan& span, ast_ptr<Expression>&& returnValueExpression);

		DEFINE_GET_SET_MOVE_REG_EXPR(ast_ptr<Expression>, ReturnValueExpression, returnValueExpression)
	};

	class ElseStatement : public BodyStatement
	{
		friend class ASTContext;
	private:
		ElseStatement(const TextSpan& span, ast_ptr<BlockStatement>&& body)
			: BodyStatement(span, ID, std::move(body))
		{
		}

	public:
		static constexpr NodeType ID = NodeType_ElseStatement;
		static ElseStatement* Create(ASTContext* context, const TextSpan& span, ast_ptr<BlockStatement>&& body);

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
		ElseIfStatement(const TextSpan& span, ast_ptr<Expression>&& condition, ast_ptr<BlockStatement>&& body)
			: ConditionalStatement(span, ID, std::move(condition), std::move(body))
		{
		}

	public:
		static constexpr NodeType ID = NodeType_ElseIfStatement;
		static ElseIfStatement* Create(ASTContext* context, const TextSpan& span, ast_ptr<Expression>&& condition, ast_ptr<BlockStatement>&& body);

		std::string DebugName() const
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] Condition: " + condition->GetSpan().str();
			return oss.str();
		}
	};

	class IfStatement : public ConditionalStatement, public AttributeContainer, TrailingObjects<IfStatement, ast_ptr<ElseIfStatement>>
	{
		friend class ASTContext;
	private:
		ast_ptr<ElseStatement> elseStatement;
		uint32_t numElseIfStatements;

		IfStatement(const TextSpan& span, ast_ptr<Expression>&& condition, ast_ptr<BlockStatement>&& body, ast_ptr<ElseStatement>&& elseStatement)
			: ConditionalStatement(span, ID, std::move(condition), std::move(body)),
			AttributeContainer(this)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_IfStatement;
		static IfStatement* Create(ASTContext* context, const TextSpan& span, ast_ptr<Expression>&& condition, ast_ptr<BlockStatement>&& body, ArrayRef<ast_ptr<ElseIfStatement>>& elseIfStatements, ast_ptr<ElseStatement>&& elseStatement);
		static IfStatement* Create(ASTContext* context, const TextSpan& span, ast_ptr<Expression>&& condition, ast_ptr<BlockStatement>&& body, uint32_t numElseIfStatements, ast_ptr<ElseStatement>&& elseStatement);

		ArrayRef<ast_ptr<ElseIfStatement>> GetElseIfStatements() noexcept
		{
			return { GetTrailingObjects<0>(numElseIfStatements), numElseIfStatements };
		}

		DEFINE_GET_SET_MOVE_CHILD(ast_ptr<ElseStatement>, ElseStatement, elseStatement);

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
		ast_ptr<Expression> expression;

		CaseStatement(const TextSpan& span, ast_ptr<Expression>&& expression)
			: StatementContainer(span, ID),
			expression(std::move(expression))
		{
			REGISTER_EXPR(expression);
		}

	public:
		static constexpr NodeType ID = NodeType_CaseStatement;
		static CaseStatement* Create(ASTContext* context, const TextSpan& span, ast_ptr<Expression>&& expression, ArrayRef<ast_ptr<ASTNode>>& statements);
		static CaseStatement* Create(ASTContext* context, const TextSpan& span, ast_ptr<Expression>&& expression, uint32_t numStatements);

		DEFINE_GET_SET_MOVE_REG_EXPR(ast_ptr<Expression>, Expression, expression);

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
		static DefaultCaseStatement* Create(ASTContext* context, const TextSpan& span, ArrayRef<ast_ptr<ASTNode>>& statements);
		static DefaultCaseStatement* Create(ASTContext* context, const TextSpan& span, uint32_t numStatements);
	};

	class SwitchStatement : public ASTNode, public AttributeContainer, public IHasExpressions, TrailingObjects<SwitchStatement, ast_ptr<CaseStatement>>
	{
		friend class ASTContext;
	private:
		ast_ptr<Expression> expression;
		ast_ptr<DefaultCaseStatement> defaultCase;
		uint32_t numCases;

		SwitchStatement(const TextSpan& span, ast_ptr<Expression>&& expression, ast_ptr<DefaultCaseStatement>&& defaultCase)
			: ASTNode(span, ID),
			AttributeContainer(this),
			expression(std::move(expression)),
			defaultCase(std::move(defaultCase))
		{
			REGISTER_EXPR(expression);
			REGISTER_CHILD(defaultCase);
		}

	public:
		static constexpr NodeType ID = NodeType_SwitchStatement;
		static SwitchStatement* Create(ASTContext* context, const TextSpan& span, ast_ptr<Expression>&& expression, ArrayRef<ast_ptr<CaseStatement>>& cases, ast_ptr<DefaultCaseStatement>&& defaultCase);
		static SwitchStatement* Create(ASTContext* context, const TextSpan& span, ast_ptr<Expression>&& expression, uint32_t numCases, ast_ptr<DefaultCaseStatement>&& defaultCase);

		ArrayRef<ast_ptr<CaseStatement>> GetCases()
		{
			return { GetTrailingObjects<0>(numCases), numCases };
		}

		DEFINE_GET_SET_MOVE_REG_EXPR(ast_ptr<Expression>, Expression, expression);

		DEFINE_GET_SET_MOVE_CHILD(ast_ptr<DefaultCaseStatement>, DefaultCase, defaultCase);

		std::string DebugName() const
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] Header: " + expression->GetSpan().str();
			return oss.str();
		}
	};

	class ForStatement : public ConditionalStatement, public AttributeContainer
	{
		friend class ASTContext;
	private:
		ast_ptr<ASTNode> init;
		ast_ptr<Expression> iteration;

		ForStatement(const TextSpan& span, ast_ptr<ASTNode>&& init, ast_ptr<Expression>&& condition, ast_ptr<Expression>&& iteration, ast_ptr<BlockStatement>&& body)
			: ConditionalStatement(span, ID, std::move(condition), std::move(body)),
			AttributeContainer(this),
			init(std::move(init)),
			iteration(std::move(iteration))
		{
			REGISTER_CHILD(init);
			REGISTER_EXPR(iteration);
		}

	public:
		static constexpr NodeType ID = NodeType_ForStatement;
		static ForStatement* Create(ASTContext* context, const TextSpan& span, ast_ptr<ASTNode>&& init, ast_ptr<Expression>&& condition, ast_ptr<Expression>&& iteration, ast_ptr<BlockStatement>&& body);

		DEFINE_GET_SET_MOVE_CHILD(ast_ptr<ASTNode>, Init, init);
		DEFINE_GET_SET_MOVE_REG_EXPR(ast_ptr<Expression>, Iteration, iteration);

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
		static BreakStatement* Create(ASTContext* context, const TextSpan& span);
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
		static ContinueStatement* Create(ASTContext* context, const TextSpan& span);
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
		static DiscardStatement* Create(ASTContext* context, const TextSpan& span);
	};

	class WhileStatement : public ConditionalStatement, public AttributeContainer
	{
		friend class ASTContext;
	private:
		WhileStatement(const TextSpan& span, ast_ptr<Expression>&& condition, ast_ptr<BlockStatement>&& body)
			: ConditionalStatement(span, ID, std::move(condition), std::move(body)),
			AttributeContainer(this)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_WhileStatement;
		static WhileStatement* Create(ASTContext* context, const TextSpan& span, ast_ptr<Expression>&& condition, ast_ptr<BlockStatement>&& body);

		std::string DebugName() const
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] Condition: " + condition->GetSpan().str();
			return oss.str();
		}
	};

	class DoWhileStatement : public ConditionalStatement, public AttributeContainer
	{
		friend class ASTContext;
	private:
		DoWhileStatement(const TextSpan& span, ast_ptr<Expression>&& condition, ast_ptr<BlockStatement>&& body)
			: ConditionalStatement(span, ID, std::move(condition), std::move(body)),
			AttributeContainer(this)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_DoWhileStatement;
		static DoWhileStatement* Create(ASTContext* context, const TextSpan& span, ast_ptr<Expression>&& condition, ast_ptr<BlockStatement>&& body);

		std::string DebugName() const
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] Condition: " + condition->GetSpan().str();
			return oss.str();
		}
	};
}

#endif