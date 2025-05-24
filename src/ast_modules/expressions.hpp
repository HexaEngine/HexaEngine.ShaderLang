#ifndef EXPRESSIONS_HPP
#define EXPRESSIONS_HPP

#include "ast_base.hpp"
#include "symbol_base.hpp"
#include "interfaces.hpp"
#include "macros.hpp"

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
		Expression(TextSpan span, NodeType type)
			: ASTNode(span, type),
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

		bool IsVoidType() const noexcept;

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

	class OperatorExpression : public Expression
	{
	protected:
		OperatorExpression(TextSpan span, NodeType type)
			: Expression(span, type)
		{
		}

	public:
		virtual const Operator& GetOperator() const noexcept = 0;
		virtual ~OperatorExpression() = default;
	};

	class ChainExpression : public Expression, public IHasSymbolRef
	{
	protected:
		ast_ptr<ChainExpression> next;

		ChainExpression(TextSpan span, NodeType type, ast_ptr<ChainExpression> next = {})
			: Expression(span, type),
			next(std::move(next))
		{
			REGISTER_CHILD(next);
		}

	public:
		virtual ~ChainExpression() = default;

		const ast_ptr<ChainExpression>& GetNextExpression() const noexcept
		{
			return next;
		}

		void SetNextExpression(ast_ptr<ChainExpression> value) noexcept
		{
			UnregisterChild(next);
			next = std::move(value);
			RegisterChild(next);
		}
	};

	class UnaryExpression : public OperatorExpression
	{
	private:
		ast_ptr<SymbolRef> operatorSymbol;
		Operator _operator;
		ast_ptr<Expression> operand;
	protected:
		UnaryExpression(TextSpan span, NodeType type, Operator op, ast_ptr<Expression> operand)
			: OperatorExpression(span, type),
			operatorSymbol(make_ast_ptr<SymbolRef>(TextSpan(), SymbolRefType_OperatorOverload, false)),
			_operator(op),
			operand(std::move(operand))
		{
			REGISTER_CHILD(operand);
		}

	public:

		std::string BuildOverloadSignature() const
		{
			auto& fqn = operand->GetInferredType()->GetFullyQualifiedName();

			std::string str;
			str.resize(2 + fqn.size() + 1);

			auto data = str.data();
			data[0] = ToLookupChar(_operator);
			data[1] = '(';
			std::copy(fqn.begin(), fqn.end(), data + 2);
			data[2 + fqn.size()] = ')';

			return str;
		}

		ast_ptr<SymbolRef>& GetOperatorSymbolRef()
		{
			return operatorSymbol;
		}

		const Operator& GetOperator() const noexcept override
		{
			return _operator;
		}

		void SetOperator(const Operator& value) noexcept
		{
			_operator = value;
		}

		DEFINE_GET_SET_MOVE_CHILD(ast_ptr<Expression>, Operand, operand)
	};

	class PrefixExpression : public UnaryExpression
	{
	public:
		PrefixExpression(TextSpan span, Operator op, ast_ptr<Expression> operand)
			: UnaryExpression(span, NodeType_PrefixExpression, op, std::move(operand))
		{
		}
	};

	class PostfixExpression : public UnaryExpression
	{
	public:
		PostfixExpression(TextSpan span, Operator op, ast_ptr<Expression> operand)
			: UnaryExpression(span, NodeType_PostfixExpression, op, std::move(operand))
		{
		}
	};

	class BinaryExpression : public OperatorExpression
	{
	private:
		Operator _operator;
		ast_ptr<SymbolRef> operatorSymbol;
		ast_ptr<Expression> left;
		ast_ptr<Expression> right;
	public:
		BinaryExpression(TextSpan span, Operator op, ast_ptr<Expression> left, ast_ptr<Expression> right)
			: OperatorExpression(span, NodeType_BinaryExpression),
			_operator(op),
			left(std::move(left)),
			right(std::move(right)),
			operatorSymbol(make_ast_ptr<SymbolRef>(TextSpan(), SymbolRefType_OperatorOverload, false))
		{
			REGISTER_CHILD(left);
			REGISTER_CHILD(right);
		}

		static void PrepareOverloadSignature(std::string& str, Operator _operator)
		{
			str.resize(2);
			auto data = str.data();
			data[0] = ToLookupChar(_operator);
			data[1] = '(';
		}

		static void BuildOverloadSignature(std::string& str, SymbolDef* left, SymbolDef* right)
		{
			auto& fqnLeft = left->GetFullyQualifiedName();
			auto& fqnRight = right->GetFullyQualifiedName();

			size_t requiredSize = 2 + fqnLeft.size() + 1 + fqnRight.size() + 1;
			str.resize(2);
			str.reserve(requiredSize);
			str.append(fqnLeft);
			str.push_back(',');
			str.append(fqnRight);
			str.push_back(')');
		}

		std::string BuildOverloadSignature() const
		{
			std::string str;
			PrepareOverloadSignature(str, _operator);
			BuildOverloadSignature(str, left->GetInferredType(), right->GetInferredType());
			return str;
		}

		ast_ptr<SymbolRef>& GetOperatorSymbolRef()
		{
			return operatorSymbol;
		}

		SymbolDef* GetOperatorDeclaration() const
		{
			return operatorSymbol->GetDeclaration();
		}

		const Operator& GetOperator() const noexcept override
		{
			return _operator;
		}

		void SetOperator(const Operator& value) noexcept
		{
			_operator = value;
		}

		DEFINE_GET_SET_MOVE_CHILD(ast_ptr<Expression>, Left, left)

			DEFINE_GET_SET_MOVE_CHILD(ast_ptr<Expression>, Right, right)
	};

	class CastExpression : public Expression
	{
	private:
		ast_ptr<SymbolRef> operatorSymbol;
		ast_ptr<SymbolRef> typeSymbol;
		ast_ptr<Expression> operand;
	public:
		CastExpression(TextSpan span, ast_ptr<SymbolRef> typeSymbol, ast_ptr<Expression> operand)
			: Expression(span, NodeType_CastExpression),
			typeSymbol(std::move(typeSymbol)),
			operand(std::move(operand))
		{
			operatorSymbol = make_ast_ptr<SymbolRef>("", SymbolRefType_OperatorOverload, false);
			REGISTER_CHILD(operand);
		}

		CastExpression(TextSpan span, ast_ptr<SymbolRef> operatorSymbol, ast_ptr<SymbolRef> typeSymbol, ast_ptr<Expression> operand)
			: Expression(span, NodeType_CastExpression),
			operatorSymbol(std::move(operatorSymbol)),
			typeSymbol(std::move(typeSymbol)),
			operand(std::move(operand))
		{
			REGISTER_CHILD(operand);
		}

		static std::string BuildOverloadSignature(const SymbolDef* targetType, const SymbolDef* sourceType)
		{
			auto& retFqn = targetType->GetFullyQualifiedName();
			auto& fqn = sourceType->GetFullyQualifiedName();

			std::string str;
			str.reserve(2 + retFqn.size() + 1 + fqn.size() + 1);
			str.resize(2);
			auto data = str.data();
			data[0] = ToLookupChar(Operator_Cast);
			data[1] = '#';
			str.append(retFqn);
			str.push_back('(');
			str.append(fqn);
			str.push_back(')');

			return str;
		}

		std::string BuildOverloadSignature() const
		{
			return BuildOverloadSignature(typeSymbol->GetDeclaration(), operand->GetInferredType());
		}

		ast_ptr<SymbolRef>& GetOperatorSymbolRef()
		{
			return operatorSymbol;
		}

		DEFINE_GET_SET_MOVE(ast_ptr<SymbolRef>, TypeSymbol, typeSymbol)

			DEFINE_GET_SET_MOVE_CHILD(ast_ptr<Expression>, Operand, operand)
	};

	class TernaryExpression : public OperatorExpression
	{
	private:
		ast_ptr<Expression> condition;
		ast_ptr<Expression> trueBranch;
		ast_ptr<Expression> falseBranch;
	public:
		TernaryExpression(TextSpan span, ast_ptr<Expression> condition, ast_ptr<Expression> trueBranch, ast_ptr<Expression> falseBranch)
			: OperatorExpression(span, NodeType_TernaryExpression),
			condition(std::move(condition)),
			trueBranch(std::move(trueBranch)),
			falseBranch(std::move(falseBranch))
		{
			REGISTER_CHILD(condition);
			REGISTER_CHILD(trueBranch);
			REGISTER_CHILD(falseBranch);
		}

		const Operator& GetOperator() const noexcept override
		{
			static const Operator _operator = Operator_Ternary;
			return _operator;
		}

		DEFINE_GET_SET_MOVE_CHILD(ast_ptr<Expression>, Condition, condition)

			DEFINE_GET_SET_MOVE_CHILD(ast_ptr<Expression>, TrueBranch, trueBranch)

			DEFINE_GET_SET_MOVE_CHILD(ast_ptr<Expression>, FalseBranch, falseBranch)
	};

	class EmptyExpression : public Expression
	{
	public:
		EmptyExpression(TextSpan span)
			: Expression(span, NodeType_EmptyExpression)
		{
		}
	};

	class LiteralExpression : public Expression
	{
	private:
		Token literal;
	public:
		LiteralExpression(TextSpan span, Token token)
			: Expression(span, NodeType_LiteralExpression),
			literal(token)
		{
		}

		Token& GetLiteral() noexcept {
			return literal;
		} void SetLiteral(const Token& value) noexcept {
			literal = value;
		}
	};

	class MemberReferenceExpression : public ChainExpression
	{
	private:
		ast_ptr<SymbolRef> symbol;
	public:
		MemberReferenceExpression(TextSpan span, ast_ptr<SymbolRef> symbol)
			: ChainExpression(span, NodeType_MemberReferenceExpression),
			symbol(std::move(symbol))
		{
		}

		ast_ptr<SymbolRef>& GetSymbolRef() override
		{
			return symbol;
		}

		DEFINE_GET_SET_MOVE(ast_ptr<SymbolRef>, Symbol, symbol);

		ast_ptr<ASTNode> Clone() const noexcept override
		{
			auto result = make_ast_ptr<MemberReferenceExpression>(span, symbol->Clone());
			return result;
		}
	};

	class FunctionCallParameter : public ASTNode
	{
	private:
		ast_ptr<Expression> expression;
	public:
		FunctionCallParameter(TextSpan span, ast_ptr<Expression> expression)
			: ASTNode(span, NodeType_FunctionCallParameter),
			expression(std::move(expression))
		{
			REGISTER_CHILD(expression);
		}

		DEFINE_GET_SET_MOVE_CHILD(ast_ptr<Expression>, Expression, expression);
	};

	class FunctionCallExpression : public ChainExpression
	{
	private:
		ast_ptr<SymbolRef> symbol;
		std::vector<ast_ptr<FunctionCallParameter>> parameters;
	public:
		FunctionCallExpression(TextSpan span, ast_ptr<SymbolRef> symbol, std::vector<ast_ptr<FunctionCallParameter>> parameters)
			: ChainExpression(span, NodeType_FunctionCallExpression),
			symbol(std::move(symbol)),
			parameters(std::move(parameters))
		{
			REGISTER_CHILDREN(parameters);
		}

		FunctionCallExpression(TextSpan span, ast_ptr<SymbolRef> symbol)
			: ChainExpression(span, NodeType_FunctionCallExpression),
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
			return true;
		}

		std::string BuildOverloadSignature()
		{
			std::ostringstream oss;
			oss << symbol->GetName() << "(";
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

		std::string BuildConstructorOverloadSignature()
		{
			std::ostringstream oss;
			oss << "#ctor(";
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

		void AddParameter(ast_ptr<FunctionCallParameter> param) noexcept
		{
			RegisterChild(param);
			parameters.push_back(std::move(param));
		}

		ast_ptr<SymbolRef>& GetSymbolRef() override
		{
			return symbol;
		}

		DEFINE_GET_SET_MOVE(ast_ptr<SymbolRef>, Symbol, symbol)

			DEFINE_GET_SET_MOVE_CHILDREN(std::vector<ast_ptr<FunctionCallParameter>>, Parameters, parameters)
	};

	class MemberAccessExpression : public ChainExpression
	{
	private:
		ast_ptr<SymbolRef> symbol;
	public:
		MemberAccessExpression(TextSpan span, ast_ptr<SymbolRef> symbol, ast_ptr<ChainExpression> expression)
			: ChainExpression(span, NodeType_MemberAccessExpression, std::move(expression)),
			symbol(std::move(symbol))
		{
		}

		ast_ptr<SymbolRef>& GetSymbolRef() override
		{
			return symbol;
		}

		DEFINE_GET_SET_MOVE(ast_ptr<SymbolRef>, Symbol, symbol);

		ast_ptr<ASTNode> Clone() const noexcept override
		{
			auto result = make_ast_ptr<MemberAccessExpression>(span, symbol->Clone(), nullptr);
			result->SetNextExpression(CloneNode(next));
			return result;
		}
	};

	class IndexerAccessExpression : public ChainExpression
	{
	private:
		ast_ptr<SymbolRef> symbol;
		ast_ptr<Expression> indexExpression;
	public:
		IndexerAccessExpression(TextSpan span, ast_ptr<SymbolRef> symbol, ast_ptr<Expression> indexExpression)
			: ChainExpression(span, NodeType_IndexerAccessExpression),
			symbol(std::move(symbol)),
			indexExpression(std::move(indexExpression))
		{
			REGISTER_CHILD(indexExpression);
		}

		IndexerAccessExpression(TextSpan span, ast_ptr<SymbolRef> symbol)
			: ChainExpression(span, NodeType_IndexerAccessExpression),
			symbol(std::move(symbol))
		{
		}

		ast_ptr<SymbolRef>& GetSymbolRef() override
		{
			return symbol;
		}

		DEFINE_GET_SET_MOVE_CHILD(ast_ptr<Expression>, IndexExpression, indexExpression);

		DEFINE_GET_SET_MOVE(ast_ptr<SymbolRef>, Symbol, symbol);
	};

	class AssignmentExpression : public OperatorExpression
	{
	private:
		const Operator op = Operator_Assign;
	protected:
		ast_ptr<Expression> target;
		ast_ptr<Expression> expression;

		AssignmentExpression(TextSpan span, NodeType type, ast_ptr<Expression> target, ast_ptr<Expression> expression)
			: OperatorExpression(span, type),
			target(std::move(target)),
			expression(std::move(expression))
		{
			REGISTER_CHILD(target);
			REGISTER_CHILD(expression);
		}

	public:
		AssignmentExpression(TextSpan span, ast_ptr<Expression> target, ast_ptr<Expression> expression)
			: OperatorExpression(span, NodeType_AssignmentExpression),
			target(std::move(target)),
			expression(std::move(expression))
		{
			REGISTER_CHILD(target);
			REGISTER_CHILD(expression);
		}

		const Operator& GetOperator() const noexcept override
		{
			return op;
		}

		virtual void SetOperator(const Operator& value)
		{
			throw std::runtime_error("setting the operator is not supported on assignment nodes.");
		}

		DEFINE_GET_SET_MOVE_CHILD(ast_ptr<Expression>, Target, target);

		DEFINE_GET_SET_MOVE_CHILD(ast_ptr<Expression>, Expression, expression);
	};

	class CompoundAssignmentExpression : public AssignmentExpression
	{
	private:
		Operator _operator;
		ast_ptr<SymbolRef> operatorSymbol;
	public:
		CompoundAssignmentExpression(TextSpan span, Operator _operator, ast_ptr<Expression> target, ast_ptr<Expression> expression)
			: AssignmentExpression(span, NodeType_CompoundAssignmentExpression, std::move(target), std::move(expression)),
			_operator(Operators::compoundToBinary(_operator)),
			operatorSymbol(make_ast_ptr<SymbolRef>(TextSpan(), SymbolRefType_OperatorOverload, false))
		{
		}

		std::string BuildOverloadSignature() const
		{
			std::string str;
			BinaryExpression::PrepareOverloadSignature(str, _operator);
			BinaryExpression::BuildOverloadSignature(str, target->GetInferredType(), expression->GetInferredType());
			return str;
		}

		ast_ptr<SymbolRef>& GetOperatorSymbolRef()
		{
			return operatorSymbol;
		}

		const Operator& GetOperator() const noexcept override
		{
			return _operator;
		}

		void SetOperator(const Operator& value) noexcept override
		{
			_operator = value;
		}
	};

	class InitializationExpression : public Expression
	{
	private:
		std::vector<ast_ptr<Expression>> parameters;

	public:
		InitializationExpression(TextSpan span, std::vector<ast_ptr<Expression>> parameters)
			: Expression(span, NodeType_InitializationExpression),
			parameters(std::move(parameters))
		{
			REGISTER_CHILDREN(parameters);
		}
		InitializationExpression(TextSpan span)
			: Expression(span, NodeType_InitializationExpression)
		{
		}

		void AddParameter(ast_ptr<Expression> parameter)
		{
			RegisterChild(parameter);
			parameters.push_back(std::move(parameter));
		}

		DEFINE_GET_SET_MOVE_CHILDREN(std::vector<ast_ptr<Expression>>, Parameters, parameters)
	};
}

#endif