#ifndef STATEMENTS_HPP
#define STATEMENTS_HPP

#include "ast_base.hpp"
#include "symbol_base.hpp"
#include "interfaces.hpp"
#include "attributes.hpp"
#include "expressions.hpp"

namespace HXSL
{
	class Statement : virtual public ASTNode
	{
	protected:
		Statement(TextSpan span, NodeType type, bool isExtern = false)
			: ASTNode(span, type, isExtern)
		{
		}
	};

	class StatementContainer
	{
		ASTNode* self;
	protected:
		std::vector<ast_ptr<Statement>> statements;

	public:
		StatementContainer(ASTNode* self) :self(self) {}
		virtual ~StatementContainer() = default;

		ASTNode* GetSelf() const noexcept { return self; }

		void AddStatement(ast_ptr<Statement> statement)
		{
			statement->SetParent(self);
			statements.push_back(std::move(statement));
		}

		const std::vector<ast_ptr<Statement>>& GetStatements() const
		{
			return statements;
		}

		std::vector<ast_ptr<Statement>>& GetStatementsMut()
		{
			return statements;
		}
	};

	class BlockStatement : public Statement, public StatementContainer
	{
	public:
		BlockStatement(TextSpan span)
			: Statement(span, NodeType_BlockStatement),
			ASTNode(span, NodeType_BlockStatement),
			StatementContainer(this)
		{
		}

		std::string DebugName() const override
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] ID: " << GetID();
			return oss.str();
		}
	};

	class BodyStatement : public Statement, public IHasBody
	{
	protected:
		ast_ptr<BlockStatement> body;

		BodyStatement(TextSpan span, NodeType type, bool isExtern = false)
			: Statement(span, type, isExtern),
			ASTNode(span, type, isExtern)
		{
		}

		BodyStatement(TextSpan span, NodeType type, ast_ptr<BlockStatement>&& body)
			: Statement(span, type, false),
			ASTNode(span, type, false),
			body(std::move(body))
		{
			REGISTER_CHILD(body);
		}

	public:
		const ast_ptr<BlockStatement>& GetBody() const noexcept override
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
			: BodyStatement(span, type, isExtern),
			ASTNode(span, type, isExtern)
		{
		}

		ConditionalStatement(TextSpan span, NodeType type, ast_ptr<Expression>&& condition, ast_ptr<BlockStatement>&& body)
			: BodyStatement(span, type, std::move(body)),
			ASTNode(span, type, isExtern),
			condition(std::move(condition))
		{
			REGISTER_EXPR(condition);
		}

	public:
		DEFINE_GET_SET_MOVE_REG_EXPR(ast_ptr<Expression>, Condition, condition);
	};

	class DeclarationStatement : public Statement, public SymbolDef, public IHasSymbolRef, public IHasExpressions
	{
	private:
		StorageClass storageClass;
		ast_ptr<SymbolRef> symbol;
		ast_ptr<Expression> initializer;

	public:
		DeclarationStatement()
			: Statement(TextSpan(), NodeType_DeclarationStatement, true),
			SymbolDef(TextSpan(), NodeType_DeclarationStatement, TextSpan(), true),
			ASTNode(TextSpan(), NodeType_DeclarationStatement, true),
			storageClass(StorageClass_None)
		{
		}

		DeclarationStatement(TextSpan span, ast_ptr<SymbolRef> symbol, StorageClass storageClass, TextSpan name, ast_ptr<Expression> initializer)
			: Statement(span, NodeType_DeclarationStatement),
			SymbolDef(span, NodeType_DeclarationStatement, name),
			ASTNode(span, NodeType_DeclarationStatement),
			storageClass(storageClass),
			symbol(std::move(symbol)),
			initializer(std::move(initializer))
		{
			REGISTER_EXPR(initializer);
		}

		ast_ptr<SymbolRef>& GetSymbolRef() override
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

	class AssignmentStatement : public Statement, public IHasExpressions
	{
	private:

	protected:
		ast_ptr<AssignmentExpression> expr;

		AssignmentStatement(TextSpan span, NodeType type, ast_ptr<AssignmentExpression> expr)
			: Statement(span, type),
			ASTNode(span, type),
			expr(std::move(expr))
		{
			REGISTER_EXPR(expr);
		}

	public:
		AssignmentStatement(TextSpan span, ast_ptr<Expression> target, ast_ptr<Expression> expression)
			: Statement(span, NodeType_AssignmentStatement),
			ASTNode(span, NodeType_AssignmentStatement),
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
		CompoundAssignmentStatement(TextSpan span, Operator op, ast_ptr<Expression> target, ast_ptr<Expression> expression)
			: AssignmentStatement(span, NodeType_CompoundAssignmentStatement, make_ast_ptr<CompoundAssignmentExpression>(span, op, std::move(target), std::move(expression))),
			ASTNode(span, NodeType_CompoundAssignmentStatement)
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

	class ExpressionStatement : public Statement, public IHasExpressions
	{
	private:
		ast_ptr<Expression> expression;

	public:
		ExpressionStatement(TextSpan span, ast_ptr<Expression> expression)
			: Statement(span, NodeType_ExpressionStatement),
			ASTNode(span, NodeType_ExpressionStatement),
			expression(std::move(expression))
		{
			REGISTER_EXPR(expression);
		}

		DEFINE_GET_SET_MOVE_REG_EXPR(ast_ptr<Expression>, Expression, expression);
	};

	class ReturnStatement : public Statement, public IHasExpressions
	{
	private:
		ast_ptr<Expression> returnValueExpression;

	public:
		ReturnStatement(TextSpan span, ast_ptr<Expression> returnValueExpression)
			: Statement(span, NodeType_ReturnStatement),
			ASTNode(span, NodeType_ReturnStatement),
			returnValueExpression(std::move(returnValueExpression))
		{
			REGISTER_EXPR(returnValueExpression);
		}

		DEFINE_GET_SET_MOVE_REG_EXPR(ast_ptr<Expression>, ReturnValueExpression, returnValueExpression)
	};

	class ElseStatement : public BodyStatement
	{
	public:
		ElseStatement(TextSpan span, ast_ptr<BlockStatement>&& body)
			: BodyStatement(span, NodeType_ElseStatement, std::move(body)),
			ASTNode(span, NodeType_ElseStatement)
		{
		}

		std::string DebugName() const override
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] ID: " << GetID();
			return oss.str();
		}
	};

	class ElseIfStatement : public ConditionalStatement
	{
	public:
		ElseIfStatement(TextSpan span, ast_ptr<Expression>&& condition, ast_ptr<BlockStatement>&& body)
			: ConditionalStatement(span, NodeType_ElseIfStatement, std::move(condition), std::move(body)),
			ASTNode(span, NodeType_ElseIfStatement)
		{
		}

		std::string DebugName() const override
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] ID: " << GetID() << " Condition: " + condition->GetSpan().str();
			return oss.str();
		}
	};

	class IfStatement : public ConditionalStatement, public AttributeContainer
	{
	private:
		std::vector<ast_ptr<ElseIfStatement>> elseIfStatements;
		ast_ptr<ElseStatement> elseStatement;
	public:
		IfStatement(TextSpan span, ast_ptr<Expression>&& condition, ast_ptr<BlockStatement>&& body)
			: ConditionalStatement(span, NodeType_IfStatement, std::move(condition), std::move(body)),
			ASTNode(span, NodeType_IfStatement),
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
			oss << "[" << ToString(type) << "] ID: " << GetID() << " Condition: " + condition->GetSpan().str();
			return oss.str();
		}
	};

	class CaseStatement : public Statement, public StatementContainer, public IHasExpressions
	{
	private:
		ast_ptr<Expression> expression;
	public:
		CaseStatement(TextSpan span, ast_ptr<Expression> expression)
			: Statement(span, NodeType_CaseStatement),
			ASTNode(span, NodeType_CaseStatement),
			StatementContainer(this),
			expression(std::move(expression))
		{
			REGISTER_EXPR(expression);
		}

		DEFINE_GET_SET_MOVE_REG_EXPR(ast_ptr<Expression>, Expression, expression);

		std::string DebugName() const override
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] ID: " << GetID() << " Header: " + expression->GetSpan().str();
			return oss.str();
		}
	};

	class DefaultCaseStatement : public Statement, public StatementContainer
	{
	private:

	public:
		DefaultCaseStatement(TextSpan span)
			: Statement(span, NodeType_DefaultCaseStatement),
			ASTNode(span, NodeType_DefaultCaseStatement),
			StatementContainer(this)
		{
		}
	};

	class SwitchStatement : public Statement, public AttributeContainer, public IHasExpressions
	{
	private:
		ast_ptr<Expression> expression;
		std::vector<ast_ptr<CaseStatement>> cases;
		ast_ptr<DefaultCaseStatement> defaultCase;
	public:
		SwitchStatement(TextSpan span, ast_ptr<Expression> expression, std::vector<ast_ptr<CaseStatement>> cases, ast_ptr<DefaultCaseStatement> defaultCase)
			: Statement(span, NodeType_SwitchStatement),
			ASTNode(span, NodeType_SwitchStatement),
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
			: Statement(span, NodeType_SwitchStatement),
			ASTNode(span, NodeType_SwitchStatement),
			AttributeContainer(this)
		{
		}

		DEFINE_GET_SET_MOVE_REG_EXPR(ast_ptr<Expression>, Expression, expression);

		void AddCase(ast_ptr<CaseStatement> _case) { RegisterChild(_case); cases.push_back(std::move(_case)); }

		DEFINE_GET_SET_MOVE_CHILD(ast_ptr<DefaultCaseStatement>, DefaultCase, defaultCase);

		std::string DebugName() const override
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] ID: " << GetID() << " Header: " + expression->GetSpan().str();
			return oss.str();
		}
	};

	class ForStatement : public ConditionalStatement, public AttributeContainer
	{
	private:
		ast_ptr<Statement> init;
		ast_ptr<Expression> iteration;
	public:
		ForStatement(TextSpan span, ast_ptr<Statement> init, ast_ptr<Expression> condition, ast_ptr<Expression> iteration, ast_ptr<BlockStatement> body)
			: ConditionalStatement(span, NodeType_ForStatement, std::move(condition), std::move(body)),
			ASTNode(span, NodeType_ForStatement),
			AttributeContainer(this),
			init(std::move(init)),
			iteration(std::move(iteration))
		{
			REGISTER_CHILD(init);
			REGISTER_EXPR(iteration);
		}

		ForStatement(TextSpan span)
			: ConditionalStatement(span, NodeType_ForStatement),
			ASTNode(span, NodeType_ForStatement),
			AttributeContainer(this)
		{
		}

		DEFINE_GET_SET_MOVE_CHILD(ast_ptr<Statement>, Init, init);
		DEFINE_GET_SET_MOVE_REG_EXPR(ast_ptr<Expression>, Iteration, iteration);

		std::string DebugName() const override
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] ID: " << GetID() << " Header: " + init->GetSpan().merge(condition->GetSpan()).merge(iteration->GetSpan()).str();
			return oss.str();
		}
	};

	class BreakStatement : public Statement
	{
	public:
		BreakStatement(TextSpan span)
			: Statement(span, NodeType_BreakStatement),
			ASTNode(span, NodeType_BreakStatement)
		{
		}
	};

	class ContinueStatement : public Statement
	{
	public:
		ContinueStatement(TextSpan span)
			: Statement(span, NodeType_ContinueStatement),
			ASTNode(span, NodeType_ContinueStatement)
		{
		}
	};

	class DiscardStatement : public Statement
	{
	public:
		DiscardStatement(TextSpan span)
			: Statement(span, NodeType_DiscardStatement),
			ASTNode(span, NodeType_DiscardStatement)
		{
		}
	};

	class WhileStatement : public ConditionalStatement, public AttributeContainer
	{
	public:
		WhileStatement(TextSpan span, ast_ptr<Expression> condition, ast_ptr<BlockStatement> body)
			: ConditionalStatement(span, NodeType_WhileStatement, std::move(condition), std::move(body)),
			ASTNode(span, NodeType_WhileStatement),
			AttributeContainer(this)
		{
		}

		WhileStatement(TextSpan span)
			: ConditionalStatement(span, NodeType_WhileStatement),
			ASTNode(span, NodeType_WhileStatement),
			AttributeContainer(this)
		{
		}

		std::string DebugName() const override
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] ID: " << GetID() << " Condition: " + condition->GetSpan().str();
			return oss.str();
		}
	};

	class DoWhileStatement : public ConditionalStatement, public AttributeContainer
	{
	public:
		DoWhileStatement(TextSpan span, ast_ptr<Expression> condition, ast_ptr<BlockStatement> body)
			: ConditionalStatement(span, NodeType_DoWhileStatement, std::move(condition), std::move(body)),
			ASTNode(span, NodeType_DoWhileStatement),
			AttributeContainer(this)
		{
		}

		DoWhileStatement(TextSpan span)
			: ConditionalStatement(span, NodeType_DoWhileStatement),
			ASTNode(span, NodeType_DoWhileStatement),
			AttributeContainer(this)
		{
		}

		std::string DebugName() const override
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] ID: " << GetID() << " Condition: " + condition->GetSpan().str();
			return oss.str();
		}
	};
}

#endif