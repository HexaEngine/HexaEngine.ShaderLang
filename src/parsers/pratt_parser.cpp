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

#define HANDLE_RESULT(expr) \
	auto result = expr; \
	if (result == 0) return true; else if (result == -1) return false;

	using BinaryExpressionPtr = std::unique_ptr<BinaryExpression>;
	using OperatorPtr = std::unique_ptr<OperatorExpression>;

	template <typename T>
	static T popFromStack(std::stack<T>& stack)
	{
		T temp = std::move(stack.top());
		stack.pop();
		return std::move(temp);
	}

	enum TaskType
	{
		TaskType_Unknown,
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

		TaskFrame() : type(TaskType_Unknown), stackBoundary(0), parent(nullptr), wasOperator(true), hadBrackets(false)
		{
		}
	};

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

	struct ParseContext : public TokenStreamAdapter, public ParserAdapter
	{
		std::stack<TaskFrame> tasks;
		std::stack<ExpressionPtr> operandStack;
		std::stack<OperatorPtr> operatorStack;
		TaskFrame currentTask;
		std::stack<size_t> ternaryStack;

		ParseContext(Parser& parser, TokenStream& stream) : TokenStreamAdapter(stream), ParserAdapter(parser)
		{
		}

		bool NextTask()
		{
			if (tasks.empty())
			{
				return false;
			}
			if (currentTask.type == TaskType_TernaryTrue)
			{
				ternaryStack.pop();
			}
			if (currentTask.type == TaskType_TernaryFalse)
			{
				ternaryStack.pop();
			}
			currentTask = popFromStack(tasks);
			return true;
		}

		bool IsInTernary() const noexcept { return !ternaryStack.empty() && ternaryStack.top() == tasks.size() + 1; }

		void PushParseExpressionTask(ASTNode* parent)
		{
			tasks.push(TaskFrame(TaskType_ParseExpression, operatorStack.size(), parent, nullptr));
		}

		void PushBracketCloseTask(ASTNode* parent)
		{
			currentTask.hadBrackets = true;
			PushCurrentTask();
			tasks.push(TaskFrame(TaskType_BracketClose, operatorStack.size(), parent, nullptr, LastToken().Span));
			PushParseExpressionTask(parent);
		}

		void PushCurrentTask()
		{
			tasks.push(std::move(currentTask));
		}

		void PushSubExpressionTask(TaskType type, ExpressionPtr&& expr)
		{
			auto ptr = expr.get();
			PushCurrentTask();
			tasks.push(TaskFrame(type, operatorStack.size(), ptr, std::move(expr)));
			PushParseExpressionTask(ptr);
			if (type == TaskType_TernaryTrue)
			{
				ternaryStack.push(tasks.size());
			}
			if (type == TaskType_TernaryFalse)
			{
				ternaryStack.push(tasks.size());
			}
		}

		void PushSubTask(TaskType type, ExpressionPtr&& expr)
		{
			auto ptr = expr.get();
			tasks.push(TaskFrame(type, operatorStack.size(), ptr, std::move(expr)));
			PushParseExpressionTask(ptr);
			if (type == TaskType_TernaryTrue)
			{
				ternaryStack.push(tasks.size());
			}
			if (type == TaskType_TernaryFalse)
			{
				ternaryStack.push(tasks.size());
			}
		}

		template<typename ExpressionType, typename... Args>
		void PushSubExpressionTask(TaskType type, Args&&... args)
		{
			PushSubExpressionTask(type, std::make_unique<ExpressionType>(std::forward<Args>(args)...));
		}

		void InjectTask(TaskType type, ExpressionPtr&& expr)
		{
			auto ptr = expr.get();
			auto last = popFromStack(tasks);
			tasks.push(TaskFrame(type, operatorStack.size(), ptr, std::move(expr)));
			tasks.push(std::move(last));
		}

		void PushOperand(ExpressionPtr&& operand)
		{
			operandStack.push(std::move(operand));
		}

		void PushOperator(OperatorPtr&& _operator)
		{
			operatorStack.push(std::move(_operator));
		}

		template<typename ExpressionType, typename... Args>
		void PushOperator(Args&&... args)
		{
			PushOperator(std::make_unique<ExpressionType>(std::forward<Args>(args)...));
		}

		ExpressionPtr PopOperand()
		{
			return popFromStack(operandStack);
		}

		bool HasOperand() const
		{
			return !operandStack.empty();
		}

		const ExpressionPtr& TopOperand() const
		{
			return operandStack.top();
		}

		void TryReduceOperators(int precedence)
		{
			while (operatorStack.size() > currentTask.stackBoundary && Operators::GetOperatorPrecedence(operatorStack.top()->GetOperator()) >= precedence)
			{
				ReduceOperator(operatorStack, operandStack);
			}
		}

		void ReduceOperatorsFinal()
		{
			while (operatorStack.size() > currentTask.stackBoundary)
			{
				ReduceOperator(operatorStack, operandStack);
			}
		}

		bool GetResult(ExpressionPtr& exprOut, ASTNode* parent)
		{
			if (operandStack.size() == 1)
			{
				exprOut = std::move(operandStack.top());
				exprOut->SetParent(parent);
				return true;
			}
			return false;
		}
	};

	static ExpressionPtr ParseUnaryPostfix(ParseContext& context, ASTNode* parent, Operator op, ExpressionPtr operand)
	{
		if (op != Operator_Increment && op != Operator_Decrement)
		{
			context.LogError("Syntax error: invalid postfix operator.", context.LastToken());
			return false;
		}

		auto type = operand->GetType();
		if (type != NodeType_MemberReferenceExpression && type != NodeType_MemberAccessExpression)
		{
			context.LogError("Syntax error: postfix increment/decrement must target a variable.", operand->GetSpan());
			return false;
		}

		return std::make_unique<PostfixExpression>(context.MakeFromLast(operand->GetSpan()), parent, op, std::move(operand));
	}

	static int ParseOperandIter(ParseContext& context, ASTNode* parent)
	{
		if (context.TryGetDelimiter('('))
		{
			context.PushBracketCloseTask(parent);
			return 0;
		}
		else
		{
			context.PushCurrentTask();
			// member expressions, function calls, indexer and other complex grammar.
			ExpressionPtr expression;
			if (!ExpressionParserRegistry::TryParse(context.GetParser(), context.GetTokenStream(), parent, expression))
			{
				return -1;
			}
			context.PushOperand(std::move(expression));
			return 0;
		}
	}

	static int ParseUnaryPrefix(ParseContext& context, ASTNode* parent, Operator op)
	{
		auto start = context.LastToken().Span;
		auto unary = std::make_unique<PrefixExpression>(start, parent, op, nullptr);;

		auto result = ParseOperandIter(context, unary.get());
		if (result == -1)
		{
			return -1;
		}

		context.InjectTask(TaskType_UnaryPrefix, std::move(unary));
		return 0;
	}

	static bool PerformParse(ParseContext& context)
	{
		ASTNode*& parent = context.currentTask.parent;
		bool& wasOperator = context.currentTask.wasOperator;
		bool& hadBrackets = context.currentTask.hadBrackets;
		while (!context.IsEndOfTokens() && !context.HasCriticalErrors())
		{
			auto current = context.Current();

			if (current.isOperatorOf(Operator_TernaryElse))
			{
				if (!context.IsInTernary())
				{
					context.Advance();
					context.LogError("Syntax error: unexpected ':' outside of a ternary expression.", current);
					continue;
				}

				if (context.LastToken().isOperatorOf(Operator_TernaryElse))
				{
					context.Advance();
					context.LogError("Syntax error: unexpected ':' expected expression after ternary ':'.", current);
					continue;
				}

				break;
			}

			Operator op;
			if (context.TryGetAnyOperator(op))
			{
				if (Operators::isUnaryOperator(op) && (op != Operator_Subtract || (op == Operator_Subtract && wasOperator)))
				{
					if (wasOperator)
					{
						wasOperator = false;
						ExpressionPtr unary;
						HANDLE_RESULT(ParseUnaryPrefix(context, parent, op));
					}
					else
					{
						ExpressionPtr operand = context.PopOperand();
						context.PushOperand(ParseUnaryPostfix(context, parent, op, std::move(operand)));
					}

					continue;
				}

				if (wasOperator)
				{
					context.LogError("Syntax error: expected an operand after operator.", current);
					continue;
				}

				context.TryReduceOperators(Operators::GetOperatorPrecedence(op));

				if (Operators::isTernaryOperator(op))
				{
					context.PushSubExpressionTask<TernaryExpression>(TaskType_TernaryTrue, current.Span, parent, nullptr, nullptr, nullptr);
					return true;
				}
				else if (Operators::isAssignment(op))
				{
					context.PushSubExpressionTask<AssignmentExpression>(TaskType_Assignment, current.Span, parent, op, nullptr, nullptr);
					return true;
				}
				else if (Operators::isBinaryOperator(op))
				{
					context.PushOperator<BinaryExpression>(current.Span, parent, op, nullptr, nullptr);
					wasOperator = true;
				}
				else
				{
					context.LogError("Syntax error: unexpected token in expression.", current);
					continue;
				}

				hadBrackets = false;
			}
			else if (current.isDelimiterOf('(') || current.isLiteral() || current.isNumeric() || current.isIdentifier())
			{
				if (hadBrackets && context.HasOperand())
				{
					auto left = context.PopOperand();
					auto type = left->GetType();

					if (type != NodeType_MemberReferenceExpression && type != NodeType_MemberAccessExpression)
					{
						context.LogError("Expected an type in cast expression.", left->GetSpan());
						return false;
					}

					std::unique_ptr<SymbolRef> typeRef;
					ParserHelper::MakeConcreteSymbolRef(left.get(), SymbolRefType_Type, typeRef);

					auto cast = std::make_unique<CastExpression>(left->GetSpan(), parent, std::move(typeRef), nullptr);
					auto result = ParseOperandIter(context, cast.get());
					if (result == -1)
					{
						return false;
					}

					context.InjectTask(TaskType_Cast, std::move(cast));
					wasOperator = false;
					return true;
				}

				wasOperator = false;
				HANDLE_RESULT(ParseOperandIter(context, parent));
			}
			else if (current.isDelimiterOf(expressionDelimiters))
			{
				break;
			}
			else
			{
				if (context.IsEndOfTokens())
				{
					context.LogError("Unexpected end of tokens.", current);
				}
				else
				{
					context.LogError("Syntax error: unexpected token in expression.", current);
				}
				break;
			}
		}

		context.ReduceOperatorsFinal();
		return true;
	}

	bool PrattParser::ParseExpressionInnerIterGen2(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Expression>& expressionOut)
	{
		ParseContext context = ParseContext(parser, stream);
		context.PushParseExpressionTask(parent);

		while (context.NextTask())
		{
			TaskFrame& frame = context.currentTask;

			switch (frame.type)
			{
			case TaskType_ParseExpression:
				if (!PerformParse(context))
				{
					return false;
				}
				break;
			case TaskType_TernaryTrue:
			{
				auto expr = std::unique_ptr<TernaryExpression>(static_cast<TernaryExpression*>(frame.result.release()));
				expr->SetTrueBranch(context.PopOperand());
				if (!stream.ExpectOperator(Operator_TernaryElse, EXPECTED_COLON))
				{
					expr->SetSpan(expr->GetSpan().merge(expr->GetTrueBranch()->GetSpan()));
					expr->SetFalseBranch(std::make_unique<EmptyExpression>(TextSpan(), expr.get()));
					context.PushOperator(std::move(expr));
					break;
				}
				context.PushSubTask(TaskType_TernaryFalse, std::move(expr));
			}
			break;
			case TaskType_TernaryFalse:
			{
				auto expr = std::unique_ptr<TernaryExpression>(static_cast<TernaryExpression*>(frame.result.release()));
				expr->SetFalseBranch(context.PopOperand());
				expr->SetSpan(expr->GetSpan().merge(expr->GetFalseBranch()->GetSpan()));
				context.PushOperator(std::move(expr));
			}
			break;
			case TaskType_Assignment:
			{
				auto expr = std::unique_ptr<AssignmentExpression>(static_cast<AssignmentExpression*>(frame.result.release()));
				expr->SetExpression(context.PopOperand());
				expr->SetSpan(expr->GetSpan().merge(expr->GetExpression()->GetSpan()));
				context.PushOperator(std::move(expr));
			}
			break;
			case TaskType_BracketClose:
			{
				stream.ExpectDelimiter(')', EXPECTED_RIGHT_PAREN);
				if (context.HasOperand())
				{
					auto& top = context.TopOperand();
					top->SetSpan(stream.MakeFromLast(frame.start));
				}
			}
			break;
			case TaskType_Cast:
			{
				auto expr = std::unique_ptr<CastExpression>(static_cast<CastExpression*>(frame.result.release()));
				expr->SetOperand(context.PopOperand());
				expr->SetSpan(expr->GetSpan().merge(expr->GetOperand()->GetSpan()));
				context.PushOperand(std::move(expr));
			}
			break;
			case TaskType_UnaryPrefix:
			{
				auto expr = std::unique_ptr<PrefixExpression>(static_cast<PrefixExpression*>(frame.result.release()));
				auto operand = context.PopOperand();
				auto op = expr->GetOperator();

				if (op == Operator_Increment || op == Operator_Decrement)
				{
					auto type = operand->GetType();
					if (type != NodeType_MemberReferenceExpression && type != NodeType_MemberAccessExpression)
					{
						parser.Log(INC_DEC_MUST_TARGET_VAR, operand->GetSpan());
						return false;
					}
				}

				expr->SetOperand(std::move(operand));
				expr->SetSpan(stream.MakeFromLast(expr->GetSpan()));
				context.PushOperand(std::move(expr));
			}
			break;
			default:
				break;
			}
		}

		return context.GetResult(expressionOut, parent);
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