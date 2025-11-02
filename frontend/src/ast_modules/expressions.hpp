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
		uint32_t lazyEvalState;
		ExpressionTraits traits;
	protected:
		Expression(const TextSpan& span, NodeType type)
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

		const uint32_t& GetLazyEvalState() const noexcept { return lazyEvalState; }

		void SetLazyEvalState(const uint32_t& value) noexcept { lazyEvalState = value; }

		void IncrementLazyEvalState() noexcept { lazyEvalState++; }

		void ResetLazyEvalState() noexcept { lazyEvalState = 0; }
	};

	class OperatorExpression : public Expression
	{
	protected:
		OperatorExpression(const TextSpan& span, NodeType type)
			: Expression(span, type)
		{
		}

	public:
		virtual const Operator& GetOperator() const noexcept = 0;
		virtual ~OperatorExpression() = default;
	};

	class ChainExpression : public Expression
	{
	protected:
		ChainExpression* next;

		ChainExpression(const TextSpan& span, NodeType type, ChainExpression* next = nullptr)
			: Expression(span, type),
			next(next)
		{
			REGISTER_CHILD(next);
		}

	public:
		virtual ~ChainExpression() = default;

		ChainExpression* GetNextExpression() const noexcept
		{
			return next;
		}

		void SetNextExpression(ChainExpression* value) noexcept
		{
			UnregisterChild(next);
			next = value;
			RegisterChild(next);
		}
	};

	class UnaryExpression : public OperatorExpression
	{
	private:
		SymbolRef* operatorSymbol;
		Operator _operator;
		Expression* operand;
	protected:
		UnaryExpression(const TextSpan& span, NodeType type, Operator op, Expression* operand, SymbolRef* operatorSymbol)
			: OperatorExpression(span, type),
			operatorSymbol(operatorSymbol),
			_operator(op),
			operand(operand)
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

		SymbolRef*& GetOperatorSymbolRef()
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

		DEFINE_GET_SET_MOVE_CHILD(Expression*, Operand, operand)
	};

	class PrefixExpression : public UnaryExpression
	{
		friend class ASTContext;
	private:
		PrefixExpression(const TextSpan& span, Operator op, Expression* operand, SymbolRef* operatorSymbol)
			: UnaryExpression(span, ID, op, operand, operatorSymbol)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_PrefixExpression;
		static PrefixExpression* Create(const TextSpan& span, Operator op, Expression* operand);
	};

	class PostfixExpression : public UnaryExpression
	{
		friend class ASTContext;
	private:
		PostfixExpression(const TextSpan& span, Operator op, Expression* operand, SymbolRef* operatorSymbol)
			: UnaryExpression(span, ID, op, operand, operatorSymbol)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_PostfixExpression;
		static PostfixExpression* Create(const TextSpan& span, Operator op, Expression* operand);
	};

	class BinaryExpression : public OperatorExpression
	{
		friend class ASTContext;
	private:
		Operator _operator;
		SymbolRef* operatorSymbol;
		Expression* left;
		Expression* right;

		BinaryExpression(const TextSpan& span, Operator op, Expression* left, Expression* right, SymbolRef* operatorSymbol)
			: OperatorExpression(span, NodeType_BinaryExpression),
			_operator(op),
			left(left),
			right(right),
			operatorSymbol(operatorSymbol)
		{
			REGISTER_CHILD(left);
			REGISTER_CHILD(right);
		}

	public:
		static constexpr NodeType ID = NodeType_BinaryExpression;
		static BinaryExpression* Create(const TextSpan& span, Operator op, Expression* left, Expression* right);

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
			str.append(fqnLeft.view());
			str.push_back(',');
			str.append(fqnRight.view());
			str.push_back(')');
		}

		std::string BuildOverloadSignature() const
		{
			std::string str;
			PrepareOverloadSignature(str, _operator);
			BuildOverloadSignature(str, left->GetInferredType(), right->GetInferredType());
			return str;
		}

		SymbolRef*& GetOperatorSymbolRef()
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

		DEFINE_GET_SET_MOVE_CHILD(Expression*, Left, left)

			DEFINE_GET_SET_MOVE_CHILD(Expression*, Right, right)
	};

	class CastExpression : public Expression
	{
		friend class ASTContext;
	private:
		SymbolRef* operatorSymbol;
		SymbolRef* typeSymbol;
		Expression* operand;

		CastExpression(const TextSpan& span, SymbolRef* operatorSymbol, SymbolRef* typeSymbol, Expression* operand)
			: Expression(span, ID),
			operatorSymbol(operatorSymbol),
			typeSymbol(typeSymbol),
			operand(operand)
		{
			REGISTER_CHILD(operand);
		}

	public:
		static constexpr NodeType ID = NodeType_CastExpression;
		static CastExpression* Create(const TextSpan& span, SymbolRef* operatorSymbol, SymbolRef* typeSymbol, Expression* operand);

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
			str.append(retFqn.view());
			str.push_back('(');
			str.append(fqn.view());
			str.push_back(')');

			return str;
		}

		std::string BuildOverloadSignature() const
		{
			return BuildOverloadSignature(typeSymbol->GetDeclaration(), operand->GetInferredType());
		}

		SymbolRef*& GetOperatorSymbolRef()
		{
			return operatorSymbol;
		}

		DEFINE_GETTER_SETTER_PTR(SymbolRef*, TypeSymbol, typeSymbol)

			DEFINE_GET_SET_MOVE_CHILD(Expression*, Operand, operand)
	};

	class TernaryExpression : public OperatorExpression
	{
		friend class ASTContext;
	private:
		Expression* condition;
		Expression* trueBranch;
		Expression* falseBranch;

		TernaryExpression(const TextSpan& span, Expression* condition, Expression* trueBranch, Expression* falseBranch)
			: OperatorExpression(span, ID),
			condition(condition),
			trueBranch(trueBranch),
			falseBranch(falseBranch)
		{
			REGISTER_CHILD(condition);
			REGISTER_CHILD(trueBranch);
			REGISTER_CHILD(falseBranch);
		}

	public:
		static constexpr NodeType ID = NodeType_TernaryExpression;
		static TernaryExpression* Create(const TextSpan& span, Expression* condition, Expression* trueBranch, Expression* falseBranch);

		const Operator& GetOperator() const noexcept override
		{
			static const Operator _operator = Operator_Ternary;
			return _operator;
		}

		DEFINE_GET_SET_MOVE_CHILD(Expression*, Condition, condition)

			DEFINE_GET_SET_MOVE_CHILD(Expression*, TrueBranch, trueBranch)

			DEFINE_GET_SET_MOVE_CHILD(Expression*, FalseBranch, falseBranch)
	};

	class EmptyExpression : public Expression
	{
		friend class ASTContext;
	private:
		EmptyExpression(const TextSpan& span)
			: Expression(span, ID)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_EmptyExpression;
		static EmptyExpression* Create(const TextSpan& span);
	};

	class LiteralExpression : public Expression
	{
		friend class ASTContext;
	private:
		Token literal;

		LiteralExpression(const TextSpan& span, const Token& token)
			: Expression(span, ID),
			literal(token)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_LiteralExpression;
		static LiteralExpression* Create(const TextSpan& span, const Token& token);

		Token& GetLiteral() noexcept
		{
			return literal;
		}

		void SetLiteral(const Token& value) noexcept
		{
			literal = value;
		}
	};

	class MemberReferenceExpression : public ChainExpression
	{
		friend class ASTContext;
	private:
		SymbolRef* symbol;

		MemberReferenceExpression(const TextSpan& span, SymbolRef* symbol)
			: ChainExpression(span, ID),
			symbol(symbol)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_MemberReferenceExpression;
		static MemberReferenceExpression* Create(const TextSpan& span, SymbolRef* symbol);

		SymbolRef* GetSymbolRef()
		{
			return symbol;
		}

		DEFINE_GETTER_SETTER_PTR(SymbolRef*, Symbol, symbol);
	};

	class FunctionCallParameter : public ASTNode
	{
		friend class ASTContext;
	private:
		Expression* expression;

		FunctionCallParameter(const TextSpan& span, Expression* expression)
			: ASTNode(span, ID),
			expression(expression)
		{
			REGISTER_CHILD(expression);
		}

	public:
		static constexpr NodeType ID = NodeType_FunctionCallParameter;
		static FunctionCallParameter* Create(const TextSpan& span, Expression* expression);

		DEFINE_GET_SET_MOVE_CHILD(Expression*, Expression, expression);
	};

	enum class FunctionCallExpressionFlags : uint32_t
	{
		None = 0,
		ConstructorCall = 1 << 0
	};

	DEFINE_FLAGS_OPERATORS(FunctionCallExpressionFlags, uint32_t);

	class FunctionCallExpression : public ChainExpression, public TrailingObjects<FunctionCallExpression, FunctionCallParameter*>
	{
		friend class ASTContext;
		friend class TrailingObjects;
	private:
		FunctionCallExpressionFlags flags = FunctionCallExpressionFlags::None;
		SymbolRef* symbol;
		TrailingObjStorage<FunctionCallExpression, uint32_t> storage;

		FunctionCallExpression(const TextSpan& span, SymbolRef* symbol)
			: ChainExpression(span, ID),
			symbol(symbol)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_FunctionCallExpression;
		static FunctionCallExpression* Create(const TextSpan& span, SymbolRef* symbol, const ArrayRef<FunctionCallParameter*>& parameters);
		static FunctionCallExpression* Create(const TextSpan& span, SymbolRef* symbol, uint32_t numParameters);

		DEFINE_FLAGS_MEMBERS(FunctionCallExpressionFlags, FunctionCall, flags);

		DEFINE_TRAILING_OBJ_SPAN_GETTER(GetParameters, 0, storage);

		bool IsConstructorCall() const noexcept { return HasFunctionCallFlag(FunctionCallExpressionFlags::ConstructorCall); }

		bool CanBuildOverloadSignature()
		{
			for (auto& param : GetParameters())
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
			for (auto& param : GetParameters())
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
			for (auto& param : GetParameters())
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

		SymbolRef* GetSymbolRef()
		{
			return symbol;
		}

		DEFINE_GETTER_SETTER_PTR(SymbolRef*, Symbol, symbol)
	};

	class MemberAccessExpression : public ChainExpression
	{
		friend class ASTContext;
	private:
		SymbolRef* symbol;

		MemberAccessExpression(const TextSpan& span, SymbolRef* symbol, ChainExpression* expression)
			: ChainExpression(span, ID, expression),
			symbol(symbol)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_MemberAccessExpression;
		static MemberAccessExpression* Create(const TextSpan& span, SymbolRef* symbol, ChainExpression* expression);

		SymbolRef* GetSymbolRef()
		{
			return symbol;
		}

		DEFINE_GETTER_SETTER_PTR(SymbolRef*, Symbol, symbol);
	};

	class IndexerAccessExpression : public ChainExpression
	{
		friend class ASTContext;
	private:
		SymbolRef* symbol;
		Expression* indexExpression;

		IndexerAccessExpression(const TextSpan& span, SymbolRef* symbol, Expression* indexExpression)
			: ChainExpression(span, ID),
			symbol(symbol),
			indexExpression(indexExpression)
		{
			REGISTER_CHILD(indexExpression);
		}

	public:
		static constexpr NodeType ID = NodeType_IndexerAccessExpression;
		static IndexerAccessExpression* Create(const TextSpan& span, SymbolRef* symbol, Expression* indexExpression);

		SymbolRef* GetSymbolRef()
		{
			return symbol;
		}

		DEFINE_GET_SET_MOVE_CHILD(Expression*, IndexExpression, indexExpression);

		DEFINE_GETTER_SETTER_PTR(SymbolRef*, Symbol, symbol);
	};

	class AssignmentExpression : public OperatorExpression
	{
		friend class ASTContext;
	private:
		const Operator op = Operator_Assign;
	protected:
		Expression* target;
		Expression* expression;

		AssignmentExpression(const TextSpan& span, NodeType type, Expression* target, Expression* expression)
			: OperatorExpression(span, type),
			target(target),
			expression(expression)
		{
			REGISTER_CHILD(target);
			REGISTER_CHILD(expression);
		}

	public:
		static constexpr NodeType ID = NodeType_AssignmentExpression;
		static AssignmentExpression* Create(const TextSpan& span, Expression* target, Expression* expression);

		const Operator& GetOperator() const noexcept override
		{
			return op;
		}

		virtual void SetOperator(const Operator& value)
		{
			throw std::runtime_error("setting the operator is not supported on assignment nodes.");
		}

		DEFINE_GET_SET_MOVE_CHILD(Expression*, Target, target);

		DEFINE_GET_SET_MOVE_CHILD(Expression*, Expression, expression);
	};

	class CompoundAssignmentExpression : public AssignmentExpression
	{
		friend class ASTContext;
	private:
		Operator _operator;
		SymbolRef* operatorSymbol;

		CompoundAssignmentExpression(const TextSpan& span, Operator _operator, Expression* target, Expression* expression, SymbolRef* operatorSymbol)
			: AssignmentExpression(span, ID, target, expression),
			_operator(Operators::compoundToBinary(_operator)),
			operatorSymbol(operatorSymbol)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_CompoundAssignmentExpression;
		static CompoundAssignmentExpression* Create(const TextSpan& span, Operator _operator, Expression* target, Expression* expression);

		std::string BuildOverloadSignature() const
		{
			std::string str;
			BinaryExpression::PrepareOverloadSignature(str, _operator);
			BinaryExpression::BuildOverloadSignature(str, target->GetInferredType(), expression->GetInferredType());
			return str;
		}

		SymbolRef*& GetOperatorSymbolRef()
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

	class InitializationExpression : public Expression, public TrailingObjects<InitializationExpression, Expression*>
	{
		friend class ASTContext;
	private:
		TrailingObjStorage<InitializationExpression, uint32_t> storage;

	public:
		static constexpr NodeType ID = NodeType_InitializationExpression;
		InitializationExpression(const TextSpan& span)
			: Expression(span, ID)
		{
		}

		static InitializationExpression* Create(const TextSpan& span, const ArrayRef<Expression*>& parameters);
		static InitializationExpression* Create(const TextSpan& span, uint32_t numParameters);

		DEFINE_TRAILING_OBJ_SPAN_GETTER(GetParameters, 0, storage);
	};
}

#endif