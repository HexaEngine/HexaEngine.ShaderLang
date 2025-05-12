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
		std::vector<std::unique_ptr<Statement>> statements;

	public:
		StatementContainer(ASTNode* self) :self(self) {}
		virtual ~StatementContainer() = default;

		ASTNode* GetSelf() const noexcept { return self; }

		void AddStatement(std::unique_ptr<Statement> statement)
		{
			statement->SetParent(self);
			statements.push_back(std::move(statement));
		}

		const std::vector<std::unique_ptr<Statement>>& GetStatements() const
		{
			return statements;
		}
	};

	class ConditionalStatement : public Statement, public IHasExpressions
	{
	protected:
		std::unique_ptr<Expression> condition;
	public:
		ConditionalStatement(TextSpan span, NodeType type, bool isExtern = false)
			: Statement(span, type, isExtern),
			ASTNode(span, type, isExtern)
		{
		}

		ConditionalStatement(TextSpan span, NodeType type, bool isExtern, std::unique_ptr<Expression>&& condition)
			: Statement(span, type, isExtern),
			ASTNode(span, type, isExtern),
			condition(std::move(condition))
		{
			REGISTER_EXPR(condition);
		}

		DEFINE_GET_SET_MOVE_REG_EXPR(std::unique_ptr<Expression>, Condition, condition)
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

	class DeclarationStatement : public Statement, public SymbolDef, public IHasSymbolRef, public IHasExpressions
	{
	private:
		StorageClass storageClass;
		std::unique_ptr<SymbolRef> symbol;
		std::unique_ptr<Expression> initializer;

	public:
		DeclarationStatement()
			: Statement(TextSpan(), NodeType_DeclarationStatement, true),
			SymbolDef(TextSpan(), NodeType_DeclarationStatement, TextSpan(), true),
			ASTNode(TextSpan(), NodeType_DeclarationStatement, true),
			storageClass(StorageClass_None)
		{
		}

		DeclarationStatement(TextSpan span, std::unique_ptr<SymbolRef> symbol, StorageClass storageClass, TextSpan name, std::unique_ptr<Expression> initializer)
			: Statement(span, NodeType_DeclarationStatement),
			SymbolDef(span, NodeType_DeclarationStatement, name),
			ASTNode(span, NodeType_DeclarationStatement),
			storageClass(storageClass),
			symbol(std::move(symbol)),
			initializer(std::move(initializer))
		{
			REGISTER_EXPR(initializer);
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
		AssignmentStatement(TextSpan span, NodeType type, std::unique_ptr<Expression> target, std::unique_ptr<Expression> expression)
			: Statement(span, type),
			ASTNode(span, type),
			target(std::move(target)),
			expression(std::move(expression))
		{
			REGISTER_EXPR(target);
			REGISTER_EXPR(expression);
		}

	public:
		AssignmentStatement(TextSpan span, std::unique_ptr<Expression> target, std::unique_ptr<Expression> expression)
			: Statement(span, NodeType_AssignmentStatement),
			ASTNode(span, NodeType_AssignmentStatement),
			target(std::move(target)),
			expression(std::move(expression))
		{
			REGISTER_EXPR(target);
			REGISTER_EXPR(expression);
		}

		DEFINE_GET_SET_MOVE_REG_EXPR(std::unique_ptr<Expression>, Target, target)

			DEFINE_GET_SET_MOVE_REG_EXPR(std::unique_ptr<Expression>, Expression, expression)
	};

	class CompoundAssignmentStatement : public AssignmentStatement {
	private:
		Operator _operator;

	public:
		CompoundAssignmentStatement(TextSpan span, Operator op, std::unique_ptr<Expression> target, std::unique_ptr<Expression> expression)
			: AssignmentStatement(span, NodeType_CompoundAssignmentStatement, std::move(target), std::move(expression)),
			ASTNode(span, NodeType_CompoundAssignmentStatement),
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
		FunctionCallStatement(TextSpan span, std::unique_ptr<Expression> expression)
			: Statement(span, NodeType_FunctionCallStatement),
			ASTNode(span, NodeType_FunctionCallStatement),
			expression(std::move(expression))
		{
			REGISTER_EXPR(expression);
		}

		DEFINE_GET_SET_MOVE_REG_EXPR(std::unique_ptr<Expression>, Expression, expression)
	};

	class ReturnStatement : public Statement, public IHasExpressions
	{
	private:
		std::unique_ptr<Expression> returnValueExpression;

	public:
		ReturnStatement(TextSpan span, std::unique_ptr<Expression> returnValueExpression)
			: Statement(span, NodeType_ReturnStatement),
			ASTNode(span, NodeType_ReturnStatement),
			returnValueExpression(std::move(returnValueExpression))
		{
			REGISTER_EXPR(returnValueExpression);
		}

		DEFINE_GET_SET_MOVE_REG_EXPR(std::unique_ptr<Expression>, ReturnValueExpression, returnValueExpression)
	};

	class IfStatement : public ConditionalStatement, public AttributeContainer
	{
	private:
		std::unique_ptr<BlockStatement> body;

	public:
		IfStatement(TextSpan span, std::unique_ptr<Expression> condition, std::unique_ptr<BlockStatement> body)
			: ConditionalStatement(span, NodeType_IfStatement, false, std::move(condition)),
			ASTNode(span, NodeType_IfStatement),
			AttributeContainer(this),
			body(std::move(body))
		{
			REGISTER_CHILD(body);
		}

		DEFINE_GET_SET_MOVE_CHILD(std::unique_ptr<BlockStatement>, Body, body)
	};

	class ElseStatement : public Statement
	{
	private:
		std::unique_ptr<BlockStatement> body;

	public:
		ElseStatement(TextSpan span, std::unique_ptr<BlockStatement> body)
			: Statement(span, NodeType_ElseStatement),
			ASTNode(span, NodeType_ElseStatement),
			body(std::move(body))
		{
			REGISTER_CHILD(body);
		}

		DEFINE_GET_SET_MOVE_CHILD(std::unique_ptr<BlockStatement>, Body, body)
	};

	class ElseIfStatement : public ConditionalStatement
	{
	private:
		std::unique_ptr<BlockStatement> body;

	public:
		ElseIfStatement(TextSpan span, std::unique_ptr<Expression> condition, std::unique_ptr<BlockStatement> body)
			: ConditionalStatement(span, NodeType_ElseIfStatement, false, std::move(condition)),
			ASTNode(span, NodeType_ElseIfStatement),
			body(std::move(body))
		{
			REGISTER_CHILD(body);
		}

		DEFINE_GET_SET_MOVE_CHILD(std::unique_ptr<BlockStatement>, Body, body)
	};

	class CaseStatement : public Statement, public StatementContainer, public IHasExpressions
	{
	private:
		std::unique_ptr<Expression> expression;
	public:
		CaseStatement(TextSpan span, std::unique_ptr<Expression> expression)
			: Statement(span, NodeType_CaseStatement),
			ASTNode(span, NodeType_CaseStatement),
			StatementContainer(this),
			expression(std::move(expression))
		{
			REGISTER_EXPR(expression);
		}

		DEFINE_GET_SET_MOVE_REG_EXPR(std::unique_ptr<Expression>, Expression, expression)
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
		std::unique_ptr<Expression> expression;
		std::vector<std::unique_ptr<CaseStatement>> cases;
		std::unique_ptr<DefaultCaseStatement> defaultCase;
	public:
		SwitchStatement(TextSpan span, std::unique_ptr<Expression> expression, std::vector<std::unique_ptr<CaseStatement>> cases, std::unique_ptr<DefaultCaseStatement> defaultCase)
			: Statement(span, NodeType_SwitchStatement),
			ASTNode(span, NodeType_SwitchStatement),
			AttributeContainer(this),
			expression(std::move(expression)),
			cases(std::move(cases)),
			defaultCase(std::move(defaultCase))
		{
			REGISTER_EXPR(expression);
			REGISTER_CHILD(defaultCase);
		}

		SwitchStatement(TextSpan span)
			: Statement(span, NodeType_SwitchStatement),
			ASTNode(span, NodeType_SwitchStatement),
			AttributeContainer(this)
		{
		}

		DEFINE_GET_SET_MOVE_REG_EXPR(std::unique_ptr<Expression>, Expression, expression)

			void AddCase(std::unique_ptr<CaseStatement> _case) { cases.push_back(std::move(_case)); }

		DEFINE_GET_SET_MOVE_CHILD(std::unique_ptr<DefaultCaseStatement>, DefaultCase, defaultCase)
	};

	class ForStatement : public ConditionalStatement, public AttributeContainer
	{
	private:
		std::unique_ptr<Statement> init;
		std::unique_ptr<Expression> iteration;
		std::unique_ptr<BlockStatement> body;
	public:
		ForStatement(TextSpan span, std::unique_ptr<Statement> init, std::unique_ptr<Expression> condition, std::unique_ptr<Expression> iteration, std::unique_ptr<BlockStatement> body)
			: ConditionalStatement(span, NodeType_ForStatement, false, std::move(condition)),
			ASTNode(span, NodeType_ForStatement),
			AttributeContainer(this),
			init(std::move(init)),
			iteration(std::move(iteration)),
			body(std::move(body))
		{
			REGISTER_CHILD(init);
			REGISTER_EXPR(iteration);
			REGISTER_CHILD(body);
		}

		ForStatement(TextSpan span)
			: ConditionalStatement(span, NodeType_ForStatement),
			ASTNode(span, NodeType_ForStatement),
			AttributeContainer(this)
		{
		}

		std::string DebugName() const override
		{
			std::ostringstream oss;
			oss << "[" << ToString(type) << "] ID: " << GetID() << " Header: " + init->GetSpan().merge(condition->GetSpan()).merge(iteration->GetSpan()).str();
			return oss.str();
		}

		DEFINE_GET_SET_MOVE_CHILD(std::unique_ptr<Statement>, Init, init)
			DEFINE_GET_SET_MOVE_REG_EXPR(std::unique_ptr<Expression>, Iteration, iteration)
			DEFINE_GET_SET_MOVE_CHILD(std::unique_ptr<BlockStatement>, Body, body)
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
	private:
		std::unique_ptr<BlockStatement> body;

	public:
		WhileStatement(TextSpan span, std::unique_ptr<Expression> condition, std::unique_ptr<BlockStatement> body)
			: ConditionalStatement(span, NodeType_WhileStatement, false, std::move(condition)),
			ASTNode(span, NodeType_WhileStatement),
			AttributeContainer(this),
			body(std::move(body))
		{
			REGISTER_CHILD(body);
		}

		WhileStatement(TextSpan span)
			: ConditionalStatement(span, NodeType_WhileStatement),
			ASTNode(span, NodeType_WhileStatement),
			AttributeContainer(this)
		{
		}

		DEFINE_GET_SET_MOVE_CHILD(std::unique_ptr<BlockStatement>, Body, body)
	};
}

#endif