#include "pratt_parser.hpp"
#include "parser_helper.hpp"

namespace HXSL
{
#define IF_ERR_RET_FALSE_ASSERT(expr) \
	if (!expr) { \
	HXSL_ASSERT(false, "");	\
	 return false; \
	}
	const std::unordered_set<char> expressionDelimiters = { ';' ,')', '}', ']', ',', ':' };

	bool PrattParser::TryParseUnaryPrefixOperator(const Token& start, Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expression, bool& hadBrackets)
	{
		Operator unaryOp;
		if (stream.TryGetUnaryOperator(unaryOp))
		{
			std::unique_ptr<Expression> operand;
			IF_ERR_RET_FALSE(ParseOperand(parser, stream, parent, operand, hadBrackets));
			if (unaryOp == Operator_Increment || unaryOp == Operator_Decrement)
			{
				auto type = operand->GetType();
				if (type != NodeType_MemberReferenceExpression && type != NodeType_MemberAccessExpression)
				{
					parser.LogError("Prefix increment/decrement must target a variable.", operand->GetSpan());
					return false;
				}
			}
			expression = std::make_unique<PrefixExpression>(TextSpan(), parent, unaryOp, std::move(operand));
			expression->SetSpan(stream.MakeFromLast(start));
			hadBrackets = false;
		}
		else
		{
			IF_ERR_RET_FALSE(ParseOperand(parser, stream, parent, expression, hadBrackets));
		}

		return true;
	}

	bool PrattParser::TryParseUnaryPostfixOperator(const Token& start, Operator op, Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expression)
	{
		if (op != Operator_Increment && op != Operator_Decrement)
		{
			parser.LogError("Invalid postfix operator.", stream.LastToken());
			return false;
		}

		auto type = expression->GetType();
		if (type != NodeType_MemberReferenceExpression && type != NodeType_MemberAccessExpression)
		{
			parser.LogError("Postfix increment/decrement must target a variable.", expression->GetSpan());
			return false;
		}

		expression = std::make_unique<PostfixExpression>(TextSpan(), parent, op, std::move(expression));
		expression->SetSpan(stream.MakeFromLast(start));
		return true;
	}

	bool PrattParser::TryParseCastOperator(const Token& start, Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expression)
	{
		std::unique_ptr<SymbolRef> type;
		ParserHelper::MakeConcreteSymbolRef(expression.get(), SymbolRefType_Type, type);
		std::unique_ptr<Expression> rightCast;
		bool hadBrackets;
		IF_ERR_RET_FALSE(ParseOperand(parser, stream, parent, rightCast, hadBrackets));
		expression = std::make_unique<CastExpression>(TextSpan(), parent, std::move(type), std::move(rightCast));
		expression->SetSpan(stream.MakeFromLast(start));
		return true;
	}

	bool PrattParser::ParseExpressionInner(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expression, int precedence)
	{
		auto start = stream.Current();

		bool hadBrackets = false;
		if (!TryParseUnaryPrefixOperator(start, parser, stream, parent, expression, hadBrackets))
		{
			return false;
		}

		while (true)
		{
			auto token = stream.Current();

			Operator op;
			if (!token.isOperator(op))
			{
				if (token.isDelimiterOf(expressionDelimiters))
				{
					return true;
				}

				auto type = expression->GetType();
				if (hadBrackets && (type == NodeType_MemberReferenceExpression || type == NodeType_MemberAccessExpression))
				{
					IF_ERR_RET_FALSE(TryParseCastOperator(start, parser, stream, parent, expression));
					continue;
				}

				ERR_RETURN_FALSE(parser, "Unexpected token in expression, expected an operator or ';'.");
			}

			int prec = Operators::GetOperatorPrecedence(op);

			if (prec <= precedence)
			{
				return true;
			}

			stream.TryAdvance();

			if (Operators::isUnaryOperator(op) && op != Operator_Subtract)
			{
				IF_ERR_RET_FALSE(TryParseUnaryPostfixOperator(start, op, parser, stream, parent, expression));
			}
			else if (Operators::isTernaryOperator(op))
			{
				auto ternary = std::make_unique<TernaryExpression>(TextSpan(), parent, std::move(expression), nullptr, nullptr);
				std::unique_ptr<Expression> right;
				IF_ERR_RET_FALSE(ParseExpressionInner(parser, stream, ternary.get(), right, hadBrackets));
				ternary->SetTrueBranch(std::move(right));
				stream.ExpectOperator(Operator_TernaryElse);
				std::unique_ptr<Expression> left;
				IF_ERR_RET_FALSE(ParseExpressionInner(parser, stream, ternary.get(), left, hadBrackets));
				ternary->SetFalseBranch(std::move(left));
				ternary->SetSpan(stream.MakeFromLast(start));
				expression = std::move(ternary);
			}
			else if (Operators::isAssignment(op))
			{
				auto chainExpr = dynamic_cast<ChainExpression*>(expression.get());
				if (chainExpr == nullptr)
				{
					parser.LogError("Expected member access expression on assignment expression.", expression->GetSpan());
					return false;
				}

				auto assignment = std::make_unique<AssignmentExpression>(TextSpan(), parent, op, std::move(expression), nullptr);
				std::unique_ptr<Expression> right;
				IF_ERR_RET_FALSE(ParseExpressionInner(parser, stream, assignment.get(), right, hadBrackets));

				if (Operators::isCompoundAssignment(op))
				{
					auto binaryOp = Operators::compoundToBinary(op);
					auto binary = std::make_unique<BinaryExpression>(TextSpan(), assignment.get(), binaryOp, nullptr, nullptr);
					binary->SetLeft(std::unique_ptr<Expression>(static_cast<Expression*>(chainExpr->Clone(parent).release())));
					binary->GetLeft()->SetParent(binary.get());
					binary->SetRight(std::move(right));
					binary->GetRight()->SetParent(binary.get());
					right = std::move(binary);
				}

				assignment->SetExpression(std::move(right));
				assignment->SetSpan(stream.MakeFromLast(start));
				expression = std::move(assignment);
			}
			else
			{
				auto binary = std::make_unique<BinaryExpression>(TextSpan(), parent, op, std::move(expression), nullptr);
				std::unique_ptr<Expression> right;
				if (!ParseExpressionInner(parser, stream, binary.get(), right, prec))
				{
					return false;
				}
				binary->SetRight(std::move(right));
				binary->SetSpan(stream.MakeFromLast(start));
				expression = std::move(binary);
			}
		}

		return true;
	}

	bool PrattParser::ParseOperand(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expression, bool& hadBrackets)
	{
		if (stream.TryGetDelimiter('('))
		{
			IF_ERR_RET_FALSE(ParseExpressionInner(parser, stream, parent, expression, 0));
			stream.ExpectDelimiter(')');
			hadBrackets = true;
		}
		else
		{
			IF_ERR_RET_FALSE(ExpressionParserRegistry::TryParse(parser, stream, parent, expression)); // member expressions, function calls, indexer and other complex grammar.
		}
		return true;
	}

#define HANDLE_RESULT(expr) \
	auto result = expr; \
	if (result == 1) continue; else if (result == -1) return false;

	using BinaryExpressionPtr = std::unique_ptr<BinaryExpression>;
	using OperatorPtr = std::unique_ptr<OperatorExpression>;

	template <typename T>
	static T popFromStack(std::stack<T>& stack)
	{
		T temp = std::move(stack.top());
		stack.pop();
		return std::move(temp);
	}

	static ExpressionPtr ParseUnaryPostfix(Parser& parser, TokenStream& stream, ASTNode* parent, Operator op, ExpressionPtr operand)
	{
		if (op != Operator_Increment && op != Operator_Decrement)
		{
			parser.LogError("Invalid postfix operator.", stream.LastToken());
			return false;
		}

		auto type = operand->GetType();
		if (type != NodeType_MemberReferenceExpression && type != NodeType_MemberAccessExpression)
		{
			parser.LogError("Postfix increment/decrement must target a variable.", operand->GetSpan());
			return false;
		}

		return std::make_unique<PostfixExpression>(stream.MakeFromLast(operand->GetSpan()), parent, op, std::move(operand));
	}

	static void ReduceOperator(std::stack<OperatorPtr>& operatorStack, std::stack<ExpressionPtr>& operandStack)
	{
		OperatorPtr expr = popFromStack(operatorStack);
		auto type = expr->GetType();

		switch (type)
		{
		case NodeType_BinaryExpression:
		{
			auto binary = static_cast<BinaryExpression*>(expr.get());
			ExpressionPtr right = popFromStack(operandStack);
			ExpressionPtr left = popFromStack(operandStack);

			auto span = left->GetSpan().merge(right->GetSpan());

			left->SetParent(binary);
			binary->SetLeft(std::move(left));
			right->SetParent(binary);
			binary->SetRight(std::move(right));
			binary->SetSpan(span);
		}
		break;
		case NodeType_TernaryExpression:
		{
			auto ternary = static_cast<TernaryExpression*>(expr.get());
			ExpressionPtr condition = popFromStack(operandStack);
			auto span = condition->GetSpan().merge(ternary->GetSpan());

			condition->SetParent(ternary);
			ternary->SetCondition(std::move(condition));

			{
				auto& trueBranch = ternary->GetTrueBranch();
				auto& falseBranch = ternary->GetFalseBranch();
				trueBranch->SetParent(nullptr);
				falseBranch->SetParent(nullptr);
				trueBranch->SetParent(ternary);
				falseBranch->SetParent(ternary);
			}

			ternary->SetSpan(span);
		}
		break;
		case NodeType_AssignmentExpression:
		{
			auto assignment = static_cast<AssignmentExpression*>(expr.get());
			ExpressionPtr target = popFromStack(operandStack);
			auto span = target->GetSpan().merge(assignment->GetSpan());

			auto assignmentOp = assignment->GetOperator();

			if (Operators::isCompoundAssignment(assignmentOp))
			{
				auto binaryOp = Operators::compoundToBinary(assignmentOp);
				auto binary = std::make_unique<BinaryExpression>(TextSpan(), assignment, binaryOp, nullptr, nullptr);
				binary->SetLeft(std::unique_ptr<Expression>(static_cast<Expression*>(target->Clone(binary.get()).release())));
				ExpressionPtr expression = std::move(assignment->GetExpressionMut());
				expression->SetParent(binary.get());
				binary->SetRight(std::move(expression));

				assignment->SetExpression(std::move(binary));
			}

			target->SetParent(assignment);
			assignment->SetTarget(std::move(target));
			assignment->SetSpan(span);
		}
		break;
		default:
			HXSL_ASSERT(false, "Missing operator type.");
			break;
		}

		operandStack.push(std::move(expr));
	}

	enum TaskType
	{
		TaskType_ParseExpression,
		TaskType_TernaryTrue,
		TaskType_TernaryFalse,
		TaskType_Assignment,
		TaskType_BracketClose,
		TaskType_Cast,
		TaskType_UnaryPrefix
	};

	struct TaskFrame
	{
		TaskType type;
		size_t stackBoundary;
		ASTNode* parent;
		ExpressionPtr result;
		bool wasOperator;
		bool hadBrackets;
		TextSpan start;

		TaskFrame(const TaskType& type, const size_t& stackLimit, ASTNode* parent, ExpressionPtr result)
			: type(type), stackBoundary(stackLimit), parent(parent), result(std::move(result)), wasOperator(true), hadBrackets(false)
		{
		}
		TaskFrame(const TaskType& type, const size_t& stackLimit, ASTNode* parent, ExpressionPtr result, TextSpan start)
			: type(type), stackBoundary(stackLimit), parent(parent), result(std::move(result)), wasOperator(true), hadBrackets(false), start(start)
		{
		}
	};

	static int ParseOperandIter(Parser& parser, TokenStream& stream, ASTNode* parent, std::stack<TaskFrame>& tasks, TaskFrame& task, size_t stackBoundary, ExpressionPtr& expressionOut)
	{
		if (stream.TryGetDelimiter('('))
		{
			task.hadBrackets = true;
			tasks.push(std::move(task));
			tasks.push(TaskFrame(TaskType_BracketClose, stackBoundary, parent, nullptr, stream.LastToken().Span));
			tasks.push(TaskFrame(TaskType_ParseExpression, stackBoundary, parent, nullptr));
			return 1;
		}
		else
		{
			ExpressionPtr expression;
			ExpressionParserRegistry::TryParse(parser, stream, parent, expression); // member expressions, function calls, indexer and other complex grammar.
			expressionOut = std::move(expression);
			return 0;
		}
	}

	static int ParseUnaryPrefix(Parser& parser, TokenStream& stream, ASTNode* parent, Operator op, std::stack<TaskFrame>& tasks, TaskFrame& task, size_t stackBoundary, ExpressionPtr& expressionOut)
	{
		auto start = stream.LastToken().Span;
		auto unary = std::make_unique<PrefixExpression>(start, parent, op, nullptr);;

		ExpressionPtr operand;
		auto result = ParseOperandIter(parser, stream, unary.get(), tasks, task, stackBoundary, operand);
		if (result == 1)
		{
			auto ptr = unary.get();
			auto last = popFromStack(tasks);
			tasks.push(TaskFrame(TaskType_UnaryPrefix, stackBoundary, ptr, std::move(unary)));
			tasks.push(std::move(last));
			return 1;
		}
		else if (result != 0)
		{
			return result;
		}

		if (op == Operator_Increment || op == Operator_Decrement)
		{
			auto type = operand->GetType();
			if (type != NodeType_MemberReferenceExpression && type != NodeType_MemberAccessExpression)
			{
				parser.LogError("Prefix increment/decrement must target a variable.", operand->GetSpan());
				return -1;
			}
		}

		unary->SetOperand(std::move(operand));
		unary->SetSpan(stream.MakeFromLast(start));

		expressionOut = std::move(unary);
		return 0;
	}

	static bool PerformParse(Parser& parser, TokenStream& stream, std::stack<TaskFrame>& tasks, TaskFrame& task, std::stack<ExpressionPtr>& operandStack, std::stack<OperatorPtr>& operatorStack)
	{
		ASTNode*& parent = task.parent;

		bool& wasOperator = task.wasOperator;
		bool& hadBrackets = task.hadBrackets;
		while (!stream.IsEndOfTokens() && !stream.HasCriticalErrors())
		{
			auto current = stream.Current();

			if (current.isOperatorOf(Operator_TernaryElse))
			{
				break;
			}

			Operator op;
			if (stream.TryGetAnyOperator(op))
			{
				if (Operators::isUnaryOperator(op) && (op != Operator_Subtract || (op == Operator_Subtract && wasOperator)))
				{
					if (wasOperator)
					{
						ExpressionPtr unary;
						auto result = ParseUnaryPrefix(parser, stream, parent, op, tasks, task, operatorStack.size(), unary);
						if (result == 1)
						{
							return true;
						}
						operandStack.push(std::move(unary));
					}
					else
					{
						ExpressionPtr operand = std::move(operandStack.top());
						operandStack.pop();
						operandStack.push(ParseUnaryPostfix(parser, stream, parent, op, std::move(operand)));
					}

					continue;
				}

				int precedence = Operators::GetOperatorPrecedence(op);
				while (operatorStack.size() > task.stackBoundary && Operators::GetOperatorPrecedence(operatorStack.top()->GetOperator()) >= precedence)
				{
					ReduceOperator(operatorStack, operandStack);
				}

				if (Operators::isTernaryOperator(op))
				{
					auto expr = std::make_unique<TernaryExpression>(current.Span, parent, nullptr, nullptr, nullptr);
					auto ptr = expr.get();
					tasks.push(std::move(task));
					tasks.push(TaskFrame(TaskType_TernaryTrue, operatorStack.size(), ptr, std::move(expr)));
					tasks.push(TaskFrame(TaskType_ParseExpression, operatorStack.size(), ptr, nullptr));
					return true;
				}
				else if (Operators::isAssignment(op))
				{
					auto expr = std::make_unique<AssignmentExpression>(current.Span, parent, op, nullptr, nullptr);
					auto ptr = expr.get();
					tasks.push(std::move(task));
					tasks.push(TaskFrame(TaskType_Assignment, operatorStack.size(), ptr, std::move(expr)));
					tasks.push(TaskFrame(TaskType_ParseExpression, operatorStack.size(), ptr, nullptr));
					return true;
				}
				else // binary
				{
					auto binary = std::make_unique<BinaryExpression>(current.Span, parent, op, nullptr, nullptr);
					parent = binary.get();
					operatorStack.push(std::move(binary));
					wasOperator = true;
				}

				hadBrackets = false;
			}
			else if (current.isDelimiterOf('(') || current.isLiteral() || current.isNumeric() || current.isIdentifier())
			{
				if (hadBrackets && !operandStack.empty())
				{
					auto left = popFromStack(operandStack);
					auto type = left->GetType();

					if (type != NodeType_MemberReferenceExpression && type != NodeType_MemberAccessExpression)
					{
						parser.LogError("Expected an type in cast expression.", left->GetSpan());
						return false;
					}

					std::unique_ptr<SymbolRef> typeRef;
					ParserHelper::MakeConcreteSymbolRef(left.get(), SymbolRefType_Type, typeRef);

					auto cast = std::make_unique<CastExpression>(left->GetSpan(), parent, std::move(typeRef), nullptr);

					ExpressionPtr right;
					auto result = ParseOperandIter(parser, stream, cast.get(), tasks, task, operandStack.size(), right);
					if (result == 1)
					{
						auto ptr = cast.get();
						auto last = popFromStack(tasks);
						tasks.push(TaskFrame(TaskType_Cast, operandStack.size(), ptr, std::move(cast)));
						tasks.push(std::move(last));
						return true;
					}

					cast->SetOperand(std::move(right));
					cast->SetSpan(stream.MakeFromLast(left->GetSpan()));

					operandStack.push(std::move(cast));
					wasOperator = false;
					continue;
				}

				ExpressionPtr operand;
				auto result = ParseOperandIter(parser, stream, parent, tasks, task, operandStack.size(), operand);
				if (result == 1)
				{
					return true;
				}
				operandStack.push(std::move(operand));
				wasOperator = false;
			}
			else if (current.isDelimiterOf(expressionDelimiters))
			{
				break;
			}
			else
			{
				return false; // TODO: Handle error.
			}
		}

		while (operatorStack.size() > task.stackBoundary)
		{
			ReduceOperator(operatorStack, operandStack);
		}
	}

	bool PrattParser::ParseExpressionInnerIterGen2(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expressionOut)
	{
		std::stack<TaskFrame> tasks;
		std::stack<ExpressionPtr> operandStack;
		std::stack<OperatorPtr> operatorStack;
		tasks.push(TaskFrame(TaskType_ParseExpression, 0, parent, nullptr));

		while (!tasks.empty())
		{
			TaskFrame frame = popFromStack(tasks);

			switch (frame.type)
			{
			case TaskType_ParseExpression:
				PerformParse(parser, stream, tasks, frame, operandStack, operatorStack);
				break;
			case TaskType_TernaryTrue:
			{
				auto expr = static_cast<TernaryExpression*>(frame.result.get());
				expr->SetTrueBranch(popFromStack(operandStack));
				stream.ExpectOperator(Operator_TernaryElse);
				tasks.push(TaskFrame(TaskType_TernaryFalse, operatorStack.size(), expr, std::move(frame.result)));
				tasks.push(TaskFrame(TaskType_ParseExpression, operatorStack.size(), expr, nullptr));
			}
			break;
			case TaskType_TernaryFalse:
			{
				auto expr = std::unique_ptr<TernaryExpression>(static_cast<TernaryExpression*>(frame.result.release()));
				expr->SetFalseBranch(popFromStack(operandStack));
				expr->SetSpan(expr->GetSpan().merge(expr->GetFalseBranch()->GetSpan()));
				operatorStack.push(std::move(expr));
			}
			break;
			case TaskType_Assignment:
			{
				auto expr = std::unique_ptr<AssignmentExpression>(static_cast<AssignmentExpression*>(frame.result.release()));
				expr->SetExpression(std::move(popFromStack(operandStack)));
				expr->SetSpan(expr->GetSpan().merge(expr->GetExpression()->GetSpan()));
				operatorStack.push(std::move(expr));
			}
			break;
			case TaskType_BracketClose:
			{
				stream.ExpectDelimiter(')');
				auto& top = operandStack.top();
				top->SetSpan(stream.MakeFromLast(frame.start));
			}
			break;
			case TaskType_Cast:
			{
				auto expr = std::unique_ptr<CastExpression>(static_cast<CastExpression*>(frame.result.release()));
				expr->SetOperand(std::move(popFromStack(operandStack)));
				expr->SetSpan(expr->GetSpan().merge(expr->GetOperand()->GetSpan()));
				operandStack.push(std::move(expr));
			}
			break;
			case TaskType_UnaryPrefix:
			{
				auto expr = std::unique_ptr<PrefixExpression>(static_cast<PrefixExpression*>(frame.result.release()));
				expr->SetOperand(std::move(popFromStack(operandStack)));
				expr->SetSpan(stream.MakeFromLast(expr->GetSpan()));
				operandStack.push(std::move(expr));
			}
			break;
			default:
				break;
			}
		}

		if (operandStack.size() == 1)
		{
			expressionOut = std::move(operandStack.top());
			expressionOut->SetParent(parent);
			return true;
		}

		return false;
	}

	bool PrattParser::ParseExpression(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expression)
	{
		HXSL_ASSERT(parent, "Parent cannot be null.");
		auto start = stream.Current();
		stream.PushState();
		if (!ParseExpressionInnerIterGen2(parser, stream, parent, expression))
		{
		}
		if (!expression.get())
		{
			stream.PopState();
			return false;
		}
		stream.PopState(false);
		expression->SetParent(parent);

		return true;
	}
}