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

		void Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<SymbolDef>>& nodes) override;

		DEFINE_GET_SET_MOVE(std::unique_ptr<SymbolRef>, Symbol, symbol)

			DEFINE_GET_SET_MOVE_REG_EXPR(std::unique_ptr<Expression>, Initializer, initializer)
	};

	class AssignmentStatement : public Statement, public IHasExpressions
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

	class ReturnStatement : public Statement, public IHasExpressions
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

		std::string DebugName() const override
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] ID: " << GetID() << " Header: " + init->GetSpan().merge(condition->GetSpan()).merge(iteration->GetSpan()).toString();
			return oss.str();
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
}

#endif