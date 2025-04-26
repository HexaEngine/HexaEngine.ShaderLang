#ifndef EXPRESSIONS_HPP
#define EXPRESSIONS_HPP

#include "ast_base.hpp"
#include "interfaces.hpp"

namespace HXSL
{
	enum ExpressionTraitFlags
	{
		ExpressionTraitFlags_None,
		ExpressionTraitFlags_Constant,
		ExpressionTraitFlags_Mutable,
	};

	struct ExpressionTraits
	{
		ExpressionTraitFlags flags = ExpressionTraitFlags_None;

		bool HasFlag(ExpressionTraitFlags wanted) const noexcept
		{
			return (flags & wanted) != 0;
		}

		bool IsConstant() const noexcept
		{
			return HasFlag(ExpressionTraitFlags_Constant);
		}

		bool IsMutable() const noexcept
		{
			return HasFlag(ExpressionTraitFlags_Mutable);
		}

		ExpressionTraits()
		{
		}

		ExpressionTraits(ExpressionTraitFlags flags) : flags(flags)
		{
		}
	};

	class Expression : public ASTNode
	{
	private:
		SymbolDef* inferredType;
		size_t lazyEvalState;
		ExpressionTraits traits;
	protected:
		Expression(TextSpan span, ASTNode* parent, NodeType type)
			: ASTNode(span, parent, type),
			lazyEvalState(0),
			inferredType(nullptr)
		{
		}

	public:

		const ExpressionTraits& GetTraits() const noexcept
		{
			return traits;
		}

		void SetTraits(const ExpressionTraits& traits) noexcept
		{
			this->traits = traits;
		}

		SymbolDef* GetInferredType() const noexcept
		{
			return inferredType;
		}

		void SetInferredType(SymbolDef* def) noexcept
		{
			inferredType = def;
			ResetLazyEvalState();
		}

		const size_t& GetLazyEvalState() const noexcept { return lazyEvalState; }

		void SetLazyEvalState(const size_t& value) noexcept { lazyEvalState = value; }

		void IncrementLazyEvalState() noexcept { lazyEvalState++; }

		void ResetLazyEvalState() noexcept { lazyEvalState = 0; }
	};

	class UnaryExpression : public Expression
	{
	private:
		std::unique_ptr<SymbolRef> operatorSymbol;
		Operator _operator;
		std::unique_ptr<Expression> operand;
	protected:
		UnaryExpression(TextSpan span, ASTNode* parent, NodeType type, Operator op, std::unique_ptr<Expression> operand)
			: Expression(span, parent, type),
			operatorSymbol(std::make_unique<SymbolRef>(TextSpan(), SymbolRefType_OperatorOverload, false)),
			_operator(op),
			operand(std::move(operand))
		{
			if (this->operand) this->operand->SetParent(this);
		}

	public:

		std::string BuildOverloadSignature() const
		{
			std::ostringstream oss;
			oss << "operator" << ToString(_operator) << "(" << operand->GetInferredType()->GetFullyQualifiedName() << ")";
			return oss.str();
		}

		std::unique_ptr<SymbolRef>& GetOperatorSymbolRef()
		{
			return operatorSymbol;
		}

		DEFINE_GETTER_SETTER(Operator, Operator, _operator)

			DEFINE_GET_SET_MOVE(std::unique_ptr<Expression>, Operand, operand)
	};

	class PrefixExpression : public UnaryExpression
	{
	public:
		PrefixExpression(TextSpan span, ASTNode* parent, Operator op, std::unique_ptr<Expression> operand)
			: UnaryExpression(span, parent, NodeType_PrefixExpression, op, std::move(operand))
		{
		}
	};

	class PostfixExpression : public UnaryExpression
	{
	public:
		PostfixExpression(TextSpan span, ASTNode* parent, Operator op, std::unique_ptr<Expression> operand)
			: UnaryExpression(span, parent, NodeType_PostfixExpression, op, std::move(operand))
		{
		}
	};

	class BinaryExpression : public Expression, public IChainExpression
	{
	private:
		Operator _operator;
		std::unique_ptr<SymbolRef> operatorSymbol;
		std::unique_ptr<Expression> left;
		std::unique_ptr<Expression> right;
	public:
		BinaryExpression(TextSpan span, ASTNode* parent, Operator op, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
			: Expression(span, parent, NodeType_BinaryExpression),
			_operator(op),
			left(std::move(left)),
			right(std::move(right)),
			operatorSymbol(std::make_unique<SymbolRef>(TextSpan(), SymbolRefType_OperatorOverload, false))
		{
			if (this->left) this->left->SetParent(this);
			if (this->right) this->right->SetParent(this);
		}

		std::string BuildOverloadSignature() const
		{
			std::ostringstream oss;
			oss << "operator" << ToString(_operator) << "(" << left->GetInferredType()->GetFullyQualifiedName() << "," << right->GetInferredType()->GetFullyQualifiedName() << ")";
			return oss.str();
		}

		std::unique_ptr<SymbolRef>& GetOperatorSymbolRef()
		{
			return operatorSymbol;
		}

		void chain(std::unique_ptr<Expression> expression) override
		{
			right = std::move(expression);
			right->SetParent(this);
		}

		virtual const std::unique_ptr<Expression>& chainNext() override
		{
			return right;
		}

		DEFINE_GETTER_SETTER(Operator, Operator, _operator)

			DEFINE_GET_SET_MOVE(std::unique_ptr<Expression>, Left, left)

			DEFINE_GET_SET_MOVE(std::unique_ptr<Expression>, Right, right)
	};

	class CastExpression : public Expression
	{
	private:
		std::unique_ptr<SymbolRef> operatorSymbol;
		std::unique_ptr<Expression> typeExpression;
		std::unique_ptr<Expression> operand;
	public:
		CastExpression(TextSpan span, ASTNode* parent, std::unique_ptr<Expression> typeExpression, std::unique_ptr<Expression> operand)
			: Expression(span, parent, NodeType_CastExpression),
			typeExpression(std::move(typeExpression)),
			operand(std::move(operand))
		{
			if (this->typeExpression) this->typeExpression->SetParent(this);
			if (this->operand) this->operand->SetParent(this);
		}

		std::string BuildOverloadSignature() const
		{
			std::ostringstream oss;
			oss << "operator" << "#" << typeExpression->GetInferredType()->GetFullyQualifiedName() << "(" << operand->GetInferredType()->GetFullyQualifiedName() << ")";
			return oss.str();
		}

		std::unique_ptr<SymbolRef>& GetOperatorSymbolRef()
		{
			return operatorSymbol;
		}

		DEFINE_GET_SET_MOVE(std::unique_ptr<Expression>, TypeExpression, typeExpression)

			DEFINE_GET_SET_MOVE(std::unique_ptr<Expression>, Operand, operand)
	};

	class TernaryExpression : public Expression
	{
	private:
		std::unique_ptr<Expression> condition;
		std::unique_ptr<Expression> trueBranch;
		std::unique_ptr<Expression> falseBranch;
	public:
		TernaryExpression(TextSpan span, ASTNode* parent, std::unique_ptr<Expression> condition, std::unique_ptr<Expression> trueBranch, std::unique_ptr<Expression> falseBranch)
			: Expression(span, parent, NodeType_TernaryExpression),
			condition(std::move(condition)),
			trueBranch(std::move(trueBranch)),
			falseBranch(std::move(falseBranch))
		{
			if (this->trueBranch) this->trueBranch->SetParent(this);
			if (this->falseBranch) this->falseBranch->SetParent(this);
		}

		DEFINE_GET_SET_MOVE(std::unique_ptr<Expression>, Condition, condition)

			DEFINE_GET_SET_MOVE(std::unique_ptr<Expression>, TrueBranch, trueBranch)

			DEFINE_GET_SET_MOVE(std::unique_ptr<Expression>, FalseBranch, falseBranch)
	};

	class EmptyExpression : public Expression
	{
	public:
		EmptyExpression(TextSpan span, ASTNode* parent)
			: Expression(span, parent, NodeType_EmptyExpression)
		{
		}
	};

	class LiteralExpression : public Expression
	{
	private:
		Token literal;
	public:
		LiteralExpression(TextSpan span, ASTNode* parent, Token token)
			: Expression(span, parent, NodeType_LiteralExpression),
			literal(token)
		{
		}

		DEFINE_GETTER_SETTER(Token, Literal, literal)
	};

	class MemberReferenceExpression : public Expression, public IHasSymbolRef
	{
	private:
		std::unique_ptr<SymbolRef> symbol;
	public:
		MemberReferenceExpression(TextSpan span, ASTNode* parent, std::unique_ptr<SymbolRef> symbol)
			: Expression(span, parent, NodeType_MemberReferenceExpression),
			symbol(std::move(symbol))
		{
		}

		std::unique_ptr<SymbolRef>& GetSymbolRef() override
		{
			return symbol;
		}

		DEFINE_GET_SET_MOVE(std::unique_ptr<SymbolRef>, Symbol, symbol)
	};

	class FunctionCallParameter : public ASTNode
	{
	private:
		std::unique_ptr<Expression> expression;
	public:
		FunctionCallParameter(TextSpan span, ASTNode* parent, std::unique_ptr<Expression> expression)
			: ASTNode(span, parent, NodeType_FunctionCallParameter),
			expression(std::move(expression))
		{
			if (this->expression) this->expression->SetParent(this);
		}

		DEFINE_GET_SET_MOVE(std::unique_ptr<Expression>, Expression, expression)
	};

	class FunctionCallExpression : public Expression, public IHasSymbolRef
	{
	private:
		std::unique_ptr<SymbolRef> symbol;
		std::vector<std::unique_ptr<FunctionCallParameter>> parameters;
	public:
		FunctionCallExpression(TextSpan span, ASTNode* parent, std::unique_ptr<SymbolRef> symbol, std::vector<std::unique_ptr<FunctionCallParameter>> parameters)
			: Expression(span, parent, NodeType_FunctionCallExpression),
			symbol(std::move(symbol)),
			parameters(std::move(parameters))
		{
		}

		FunctionCallExpression(TextSpan span, ASTNode* parent, std::unique_ptr<SymbolRef> symbol)
			: Expression(span, parent, NodeType_FunctionCallExpression),
			symbol(std::move(symbol))
		{
		}

		bool CanBuildOverloadSignature()
		{
			for (auto& param : parameters)
			{
				if (!param->GetExpression()->GetInferredType())
				{
					return false;
				}
			}
			return false;
		}

		std::string BuildOverloadSignature()
		{
			std::ostringstream oss;
			oss << symbol->GetName().toString() << "(";
			bool first = true;
			for (auto& param : parameters)
			{
				if (!first)
				{
					oss << ",";
				}
				first = false;
				oss << param->GetExpression()->GetInferredType()->GetFullyQualifiedName();
			}
			oss << ")";
			return oss.str();
		}

		std::unique_ptr<SymbolRef>& GetSymbolRef() override
		{
			return symbol;
		}

		DEFINE_GET_SET_MOVE(std::unique_ptr<SymbolRef>, Symbol, symbol)

			DEFINE_GET_SET_MOVE(std::vector<std::unique_ptr<FunctionCallParameter>>, Parameters, parameters)
	};

	class MemberAccessExpression : public Expression, public IChainExpression, public IHasSymbolRef
	{
	private:
		std::unique_ptr<SymbolRef> symbol;
		std::unique_ptr<Expression> expression;
	public:
		MemberAccessExpression(TextSpan span, ASTNode* parent, std::unique_ptr<SymbolRef> symbol, std::unique_ptr<Expression> expression)
			: Expression(span, parent, NodeType_MemberAccessExpression),
			symbol(std::move(symbol)),
			expression(std::move(expression))
		{
		}

		std::unique_ptr<SymbolRef>& GetSymbolRef() override
		{
			return symbol;
		}

		void chain(std::unique_ptr<Expression> expression) override
		{
			this->expression = std::move(expression);
			this->expression->SetParent(this);
		}

		virtual const std::unique_ptr<Expression>& chainNext() override
		{
			return expression;
		}

		DEFINE_GET_SET_MOVE(std::unique_ptr<SymbolRef>, Symbol, symbol)

			DEFINE_GET_SET_MOVE(std::unique_ptr<Expression>, Expression, expression)
	};

	class ComplexMemberAccessExpression : public Expression, public IChainExpression, public IHasSymbolRef
	{
	private:
		std::unique_ptr<SymbolRef> symbol;
		std::unique_ptr<Expression> left;
		std::unique_ptr<Expression> right;
	public:
		ComplexMemberAccessExpression(TextSpan span, ASTNode* parent, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
			: Expression(span, parent, NodeType_ComplexMemberAccessExpression),
			symbol(std::make_unique<SymbolRef>()),
			left(std::move(left)),
			right(std::move(right))
		{
		}

		std::unique_ptr<SymbolRef>& GetSymbolRef() override
		{
			return symbol;
		}

		void chain(std::unique_ptr<Expression> expression) override
		{
			this->right = std::move(expression);
			this->right->SetParent(this);
			if (left && right)
			{
				span = left->GetSpan().merge(right->GetSpan());
			}
		}

		virtual const std::unique_ptr<Expression>& chainNext() override
		{
			return right;
		}

		DEFINE_GET_SET_MOVE(std::unique_ptr<SymbolRef>, Symbol, symbol)

			DEFINE_GET_SET_MOVE(std::unique_ptr<Expression>, Left, left)

			DEFINE_GET_SET_MOVE(std::unique_ptr<Expression>, Right, right)
	};

	class IndexerAccessExpression : public Expression, public IHasSymbolRef
	{
	private:
		std::unique_ptr<SymbolRef> symbol;
		std::unique_ptr<Expression> indexExpression;

	public:
		IndexerAccessExpression(TextSpan span, ASTNode* parent, std::unique_ptr<SymbolRef> symbol, std::unique_ptr<Expression> indexExpression)
			: Expression(span, parent, NodeType_IndexerAccessExpression),
			symbol(std::move(symbol)),
			indexExpression(std::move(indexExpression))
		{
		}

		IndexerAccessExpression(TextSpan span, ASTNode* parent, std::unique_ptr<SymbolRef> symbol)
			: Expression(span, parent, NodeType_IndexerAccessExpression),
			symbol(std::move(symbol))
		{
		}

		std::unique_ptr<SymbolRef>& GetSymbolRef() override
		{
			return symbol;
		}

		DEFINE_GET_SET_MOVE(std::unique_ptr<Expression>, IndexExpression, indexExpression)

			DEFINE_GET_SET_MOVE(std::unique_ptr<SymbolRef>, Symbol, symbol)
	};

	class AssignmentExpression : public Expression {
	private:
		std::unique_ptr<Expression> target;
		std::unique_ptr<Expression> expression;

	protected:
		AssignmentExpression(TextSpan span, ASTNode* parent, NodeType type, std::unique_ptr<Expression> target, std::unique_ptr<Expression> expression)
			: Expression(span, parent, type),
			target(std::move(target)),
			expression(std::move(expression))
		{
			if (this->target) this->target->SetParent(this);
			if (this->expression) this->expression->SetParent(this);
		}

	public:
		AssignmentExpression(TextSpan span, ASTNode* parent, std::unique_ptr<Expression> target, std::unique_ptr<Expression> expression)
			: Expression(span, parent, NodeType_AssignmentExpression),
			target(std::move(target)),
			expression(std::move(expression))
		{
			if (this->target) this->target->SetParent(this);
			if (this->expression) this->expression->SetParent(this);
		}

		DEFINE_GET_SET_MOVE(std::unique_ptr<Expression>, Target, target)

			DEFINE_GET_SET_MOVE(std::unique_ptr<Expression>, Expression, expression)
	};

	class CompoundAssignmentExpression : public AssignmentExpression {
	private:
		Operator _operator;

	public:
		CompoundAssignmentExpression(TextSpan span, ASTNode* parent, Operator op, std::unique_ptr<Expression> target, std::unique_ptr<Expression> expression)
			: AssignmentExpression(span, parent, NodeType_CompoundAssignmentExpression, std::move(target), std::move(expression)),
			_operator(op)
		{
		}

		DEFINE_GETTER_SETTER(Operator, Operator, _operator)
	};

	class InitializationExpression : public Expression
	{
	private:
		std::vector<std::unique_ptr<Expression>> parameters;

	public:
		InitializationExpression(TextSpan span, ASTNode* parent, std::vector<std::unique_ptr<Expression>> parameters)
			: Expression(span, parent, NodeType_InitializationExpression),
			parameters(std::move(parameters))
		{
		}
		InitializationExpression(TextSpan span, ASTNode* parent)
			: Expression(span, parent, NodeType_InitializationExpression)
		{
		}

		void AddParameter(std::unique_ptr<Expression> parameter)
		{
			parameters.push_back(std::move(parameter));
		}

		DEFINE_GET_SET_MOVE(std::vector<std::unique_ptr<Expression>>, Parameters, parameters)
	};
}

#endif