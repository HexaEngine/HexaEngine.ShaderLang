#ifndef STATEMENTS_HPP
#define STATEMENTS_HPP

#include "ast_base.hpp"
#include "symbol_base.hpp"
#include "interfaces.hpp"
#include "attributes.hpp"
#include "expressions.hpp"

namespace HXSL
{
	class StatementContainer
	{
		ASTNode* self;
	protected:
		std::vector<ast_ptr<ASTNode>> statements;

	public:
		StatementContainer(ASTNode* self) :self(self) {}
		virtual ~StatementContainer() = default;

		ASTNode* GetSelf() const noexcept { return self; }

		void AddStatement(ast_ptr<ASTNode> statement)
		{
			statement->SetParent(self);
			statements.push_back(std::move(statement));
		}

		const std::vector<ast_ptr<ASTNode>>& GetStatements() const
		{
			return statements;
		}

		std::vector<ast_ptr<ASTNode>>& GetStatementsMut()
		{
			return statements;
		}
	};

	class BlockStatement : public ASTNode, public StatementContainer
	{
	public:
		static constexpr NodeType ID = NodeType_BlockStatement;
		BlockStatement(TextSpan span)
			: ASTNode(span, NodeType_BlockStatement),
			StatementContainer(this)
		{
		}

		std::string DebugName() const override
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

		BodyStatement(TextSpan span, NodeType type, bool isExtern = false)
			: ASTNode(span, type, isExtern)
		{
		}

		BodyStatement(TextSpan span, NodeType type, ast_ptr<BlockStatement>&& body)
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

		ConditionalStatement(TextSpan span, NodeType type, bool isExtern = false)
			: BodyStatement(span, type, isExtern)
		{
		}

		ConditionalStatement(TextSpan span, NodeType type, ast_ptr<Expression>&& condition, ast_ptr<BlockStatement>&& body)
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
	private:
		StorageClass storageClass;
		ast_ptr<SymbolRef> symbol;
		ast_ptr<Expression> initializer;

	public:
		static constexpr NodeType ID = NodeType_DeclarationStatement;
		DeclarationStatement()
			: SymbolDef(TextSpan(), ID, TextSpan(), true),
			storageClass(StorageClass_None)
		{
		}

		DeclarationStatement(TextSpan span, ast_ptr<SymbolRef> symbol, StorageClass storageClass, TextSpan name, ast_ptr<Expression> initializer)
			: SymbolDef(span, ID, name),
			storageClass(storageClass),
			symbol(std::move(symbol)),
			initializer(std::move(initializer))
		{
			REGISTER_EXPR(initializer);
		}

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

		SymbolType GetSymbolType() const override
		{
			return SymbolType_Variable;
		}

		void Write(Stream& stream) const override;

		void Read(Stream& stream, StringPool& container) override;

		void Build(SymbolTable& table, size_t index, CompilationUnit* compilation, std::vector<ast_ptr<SymbolDef>>& nodes) override;

		DEFINE_GET_SET_MOVE(ast_ptr<SymbolRef>, Symbol, symbol)

			DEFINE_GET_SET_MOVE_REG_EXPR(ast_ptr<Expression>, Initializer, initializer)
	};

	class AssignmentStatement : public ASTNode, public IHasExpressions
	{
	private:

	protected:
		ast_ptr<AssignmentExpression> expr;

		AssignmentStatement(TextSpan span, NodeType type, ast_ptr<AssignmentExpression> expr)
			: ASTNode(span, type),
			expr(std::move(expr))
		{
			REGISTER_EXPR(expr);
		}

	public:
		static constexpr NodeType ID = NodeType_AssignmentStatement;
		AssignmentStatement(TextSpan span, ast_ptr<Expression> target, ast_ptr<Expression> expression)
			: ASTNode(span, ID),
			expr(make_ast_ptr<AssignmentExpression>(span, std::move(target), std::move(expression)))
		{
			REGISTER_EXPR(expr);
		}

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
	public:
		static constexpr NodeType ID = NodeType_CompoundAssignmentStatement;
		CompoundAssignmentStatement(TextSpan span, Operator op, ast_ptr<Expression> target, ast_ptr<Expression> expression)
			: AssignmentStatement(span, ID, make_ast_ptr<CompoundAssignmentExpression>(span, op, std::move(target), std::move(expression)))
		{
		}

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
	private:
		ast_ptr<Expression> expression;

	public:
		static constexpr NodeType ID = NodeType_ExpressionStatement;
		ExpressionStatement(TextSpan span, ast_ptr<Expression> expression)
			: ASTNode(span, ID),
			expression(std::move(expression))
		{
			REGISTER_EXPR(expression);
		}

		DEFINE_GET_SET_MOVE_REG_EXPR(ast_ptr<Expression>, Expression, expression);
	};

	class ReturnStatement : public ASTNode, public IHasExpressions
	{
	private:
		ast_ptr<Expression> returnValueExpression;

	public:
		static constexpr NodeType ID = NodeType_ReturnStatement;
		ReturnStatement(TextSpan span, ast_ptr<Expression> returnValueExpression)
			: ASTNode(span, ID),
			returnValueExpression(std::move(returnValueExpression))
		{
			REGISTER_EXPR(returnValueExpression);
		}

		DEFINE_GET_SET_MOVE_REG_EXPR(ast_ptr<Expression>, ReturnValueExpression, returnValueExpression)
	};

	class ElseStatement : public BodyStatement
	{
	public:
		static constexpr NodeType ID = NodeType_ElseStatement;
		ElseStatement(TextSpan span, ast_ptr<BlockStatement>&& body)
			: BodyStatement(span, ID, std::move(body))
		{
		}

		std::string DebugName() const override
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "]";
			return oss.str();
		}
	};

	class ElseIfStatement : public ConditionalStatement
	{
	public:
		static constexpr NodeType ID = NodeType_ElseIfStatement;
		ElseIfStatement(TextSpan span, ast_ptr<Expression>&& condition, ast_ptr<BlockStatement>&& body)
			: ConditionalStatement(span, ID, std::move(condition), std::move(body))
		{
		}

		std::string DebugName() const override
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] Condition: " + condition->GetSpan().str();
			return oss.str();
		}
	};

	class IfStatement : public ConditionalStatement, public AttributeContainer
	{
	private:
		std::vector<ast_ptr<ElseIfStatement>> elseIfStatements;
		ast_ptr<ElseStatement> elseStatement;
	public:
		static constexpr NodeType ID = NodeType_IfStatement;
		IfStatement(TextSpan span, ast_ptr<Expression>&& condition, ast_ptr<BlockStatement>&& body)
			: ConditionalStatement(span, ID, std::move(condition), std::move(body)),
			AttributeContainer(this)
		{
		}

		void AddElseIf(ast_ptr<ElseIfStatement>&& value) noexcept
		{
			RegisterChild(value.get());
			elseIfStatements.push_back(std::move(value));
		}

		const std::vector<ast_ptr<ElseIfStatement>>& GetElseIfStatements() const noexcept
		{
			return elseIfStatements;
		}

		DEFINE_GET_SET_MOVE_CHILD(ast_ptr<ElseStatement>, ElseStatement, elseStatement);

		std::string DebugName() const override
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] Condition: " + condition->GetSpan().str();
			return oss.str();
		}
	};

	class CaseStatement : public ASTNode, public StatementContainer, public IHasExpressions
	{
	private:
		ast_ptr<Expression> expression;
	public:
		static constexpr NodeType ID = NodeType_CaseStatement;
		CaseStatement(TextSpan span, ast_ptr<Expression> expression)
			: ASTNode(span, ID),
			StatementContainer(this),
			expression(std::move(expression))
		{
			REGISTER_EXPR(expression);
		}

		DEFINE_GET_SET_MOVE_REG_EXPR(ast_ptr<Expression>, Expression, expression);

		std::string DebugName() const override
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] Header: " + expression->GetSpan().str();
			return oss.str();
		}
	};

	class DefaultCaseStatement : public ASTNode, public StatementContainer
	{
	private:

	public:
		static constexpr NodeType ID = NodeType_DefaultCaseStatement;
		DefaultCaseStatement(TextSpan span)
			: ASTNode(span, ID),
			StatementContainer(this)
		{
		}
	};

	class SwitchStatement : public ASTNode, public AttributeContainer, public IHasExpressions
	{
	private:
		ast_ptr<Expression> expression;
		std::vector<ast_ptr<CaseStatement>> cases;
		ast_ptr<DefaultCaseStatement> defaultCase;
	public:
		static constexpr NodeType ID = NodeType_SwitchStatement;
		SwitchStatement(TextSpan span, ast_ptr<Expression> expression, std::vector<ast_ptr<CaseStatement>> cases, ast_ptr<DefaultCaseStatement> defaultCase)
			: ASTNode(span, ID),
			AttributeContainer(this),
			expression(std::move(expression)),
			cases(std::move(cases)),
			defaultCase(std::move(defaultCase))
		{
			REGISTER_EXPR(expression);
			REGISTER_CHILDREN(cases);
			REGISTER_CHILD(defaultCase);
		}

		SwitchStatement(TextSpan span)
			: ASTNode(span, ID),
			AttributeContainer(this)
		{
		}

		DEFINE_GET_SET_MOVE_REG_EXPR(ast_ptr<Expression>, Expression, expression);

		void AddCase(ast_ptr<CaseStatement> _case) { RegisterChild(_case); cases.push_back(std::move(_case)); }

		DEFINE_GET_SET_MOVE_CHILD(ast_ptr<DefaultCaseStatement>, DefaultCase, defaultCase);

		std::string DebugName() const override
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] Header: " + expression->GetSpan().str();
			return oss.str();
		}
	};

	class ForStatement : public ConditionalStatement, public AttributeContainer
	{
	private:
		ast_ptr<ASTNode> init;
		ast_ptr<Expression> iteration;
	public:
		static constexpr NodeType ID = NodeType_ForStatement;
		ForStatement(TextSpan span, ast_ptr<ASTNode> init, ast_ptr<Expression> condition, ast_ptr<Expression> iteration, ast_ptr<BlockStatement> body)
			: ConditionalStatement(span, ID, std::move(condition), std::move(body)),
			AttributeContainer(this),
			init(std::move(init)),
			iteration(std::move(iteration))
		{
			REGISTER_CHILD(init);
			REGISTER_EXPR(iteration);
		}

		ForStatement(TextSpan span)
			: ConditionalStatement(span, ID),
			AttributeContainer(this)
		{
		}

		DEFINE_GET_SET_MOVE_CHILD(ast_ptr<ASTNode>, Init, init);
		DEFINE_GET_SET_MOVE_REG_EXPR(ast_ptr<Expression>, Iteration, iteration);

		std::string DebugName() const override
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] Header: " + init->GetSpan().merge(condition->GetSpan()).merge(iteration->GetSpan()).str();
			return oss.str();
		}
	};

	class BreakStatement : public ASTNode
	{
	public:
		static constexpr NodeType ID = NodeType_BreakStatement;
		BreakStatement(TextSpan span)
			: ASTNode(span, ID)
		{
		}
	};

	class ContinueStatement : public ASTNode
	{
	public:
		static constexpr NodeType ID = NodeType_ContinueStatement;
		ContinueStatement(TextSpan span)
			: ASTNode(span, ID)
		{
		}
	};

	class DiscardStatement : public ASTNode
	{
	public:
		static constexpr NodeType ID = NodeType_DiscardStatement;
		DiscardStatement(TextSpan span)
			: ASTNode(span, ID)
		{
		}
	};

	class WhileStatement : public ConditionalStatement, public AttributeContainer
	{
	public:
		static constexpr NodeType ID = NodeType_WhileStatement;
		WhileStatement(TextSpan span, ast_ptr<Expression> condition, ast_ptr<BlockStatement> body)
			: ConditionalStatement(span, ID, std::move(condition), std::move(body)),
			AttributeContainer(this)
		{
		}

		WhileStatement(TextSpan span)
			: ConditionalStatement(span, ID),
			AttributeContainer(this)
		{
		}

		std::string DebugName() const override
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] Condition: " + condition->GetSpan().str();
			return oss.str();
		}
	};

	class DoWhileStatement : public ConditionalStatement, public AttributeContainer
	{
	public:
		static constexpr NodeType ID = NodeType_DoWhileStatement;
		DoWhileStatement(TextSpan span, ast_ptr<Expression> condition, ast_ptr<BlockStatement> body)
			: ConditionalStatement(span, ID, std::move(condition), std::move(body)),
			AttributeContainer(this)
		{
		}

		DoWhileStatement(TextSpan span)
			: ConditionalStatement(span, ID),
			AttributeContainer(this)
		{
		}

		std::string DebugName() const override
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] Condition: " + condition->GetSpan().str();
			return oss.str();
		}
	};
}

#endif