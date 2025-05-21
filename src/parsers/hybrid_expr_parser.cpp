#include "hybrid_expr_parser.hpp"
#include "parser_helper.hpp"
#include "sub_parser_registry.hpp"

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

	using ExpressionPtr = std::unique_ptr<Expression>;
	using BinaryExpressionPtr = std::unique_ptr<BinaryExpression>;
	using OperatorPtr = std::unique_ptr<OperatorExpression>;

	template <typename T>
	static T popFromStack(std::stack<T>& stack)
	{
		T temp = std::move(stack.top());
		stack.pop();
		return std::move(temp);
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

	bool ParseContext::NextTask()
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

	void ParseContext::InjectTask(TaskType type, ExpressionPtr&& expr)
	{
		if (!canInject)
		{
			PushTask(TaskFrame(type, operatorStack.size(), std::move(expr)));
			return;
		}
		auto last = popFromStack(tasks);
		PushTask(TaskFrame(type, operatorStack.size(), std::move(expr)));
		PushTask(std::move(last));
	}

	ExpressionPtr ParseContext::PopOperand()
	{
		return popFromStack(operandStack);
	}

	void ParseContext::TryReduceOperators(int precedence)
	{
		while (operatorStack.size() > currentTask.stackBoundary && Operators::GetOperatorPrecedence(operatorStack.top()->GetOperator()) >= precedence)
		{
			ReduceOperator(operatorStack, operandStack);
		}
	}

	void ParseContext::ReduceOperatorsFinal()
	{
		while (operatorStack.size() > currentTask.stackBoundary)
		{
			ReduceOperator(operatorStack, operandStack);
		}
	}

	static ExpressionPtr ParseUnaryPostfix(ParseContext& context, Operator op, ExpressionPtr operand)
	{
		if (op != Operator_Increment && op != Operator_Decrement)
		{
			context.Log(INVALID_POSTFIX_OP, context.LastToken());
			return nullptr;
		}

		auto type = operand->GetType();
		if (type != NodeType_MemberReferenceExpression && type != NodeType_MemberAccessExpression)
		{
			context.Log(INC_DEC_MUST_TARGET_VAR, operand->GetSpan());
			return nullptr;
		}

		return std::make_unique<PostfixExpression>(context.MakeFromLast(operand->GetSpan()), op, std::move(operand));
	}

	static int ParseOperandIter(ParseContext& context)
	{
		if (context.TryGetDelimiter('('))
		{
			context.PushBracketCloseTask();
			return 0;
		}
		else
		{
			context.PushCurrentTask();
			// member expressions, function calls, indexer and other complex grammar.
			ExpressionPtr expression;
			if (!ExpressionParserRegistry::TryParse(context.GetParser(), context.GetTokenStream(), expression))
			{
				return -1;
			}
			context.PushOperand(std::move(expression));
			return 0;
		}
	}

	static int ParseUnaryPrefix(ParseContext& context, Operator op)
	{
		auto start = context.LastToken().Span;
		auto unary = std::make_unique<PrefixExpression>(start, op, nullptr);;

		auto result = ParseOperandIter(context);
		if (result == -1)
		{
			return -1;
		}

		context.InjectTask(TaskType_UnaryPrefix, std::move(unary));
		return 0;
	}

	static bool PerformParse(ParseContext& context)
	{
		bool& wasOperator = context.currentTask.wasOperator;
		bool& hadBrackets = context.currentTask.hadBrackets;
		while (!context.IsEndOfTokens() && !context.HasCriticalErrors())
		{
			auto current = context.Current();

			if (current.isOperatorOf(Operator_TernaryElse))
			{
				if (!context.IsInTernary())
				{
					if ((context.flags & ExpressionParserFlags_SwitchCase) != 0)
					{
						break;
					}
					context.Advance();
					context.Log(UNEXPECTED_COLON_OUTSIDE_TERNARY, current);
					continue;
				}

				if (context.LastToken().isOperatorOf(Operator_TernaryElse))
				{
					context.Advance();
					context.Log(EXPECTED_EXPR_AFTER_TERNARY, current);
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
						HANDLE_RESULT(ParseUnaryPrefix(context, op));
					}
					else
					{
						ExpressionPtr operand = context.PopOperand();
						context.PushOperand(ParseUnaryPostfix(context, op, std::move(operand)));
					}

					continue;
				}

				if (wasOperator)
				{
					context.Log(EXPECTED_OPERAND_AFTER_OP, current);
					continue;
				}

				context.TryReduceOperators(Operators::GetOperatorPrecedence(op));

				if (Operators::isTernaryOperator(op))
				{
					context.PushSubExpressionTask<TernaryExpression>(TaskType_TernaryTrue, current.Span, nullptr, nullptr, nullptr);
					return true;
				}
				else if (Operators::isAssignment(op))
				{
					if (Operators::isCompoundAssignment(op))
					{
						context.PushSubExpressionTask<CompoundAssignmentExpression>(TaskType_Assignment, current.Span, Operators::compoundToBinary(op), nullptr, nullptr);
					}
					else
					{
						context.PushSubExpressionTask<AssignmentExpression>(TaskType_Assignment, current.Span, nullptr, nullptr);
					}
					return true;
				}
				else if (Operators::isBinaryOperator(op))
				{
					context.PushOperator<BinaryExpression>(current.Span, op, nullptr, nullptr);
					wasOperator = true;
				}
				else
				{
					context.Log(UNEXPECTED_TOKEN, current);
					continue;
				}

				hadBrackets = false;
			}
			else if (current.isDelimiterOf('(') || current.isLiteral() || current.isNumeric() || current.isIdentifier() || current.isKeywordOf(BuiltInTypes) || current.isKeywordOf(KeywordLiterals))
			{
				if (hadBrackets && context.HasOperand())
				{
					auto left = context.PopOperand();
					auto type = left->GetType();

					if (type != NodeType_MemberReferenceExpression && type != NodeType_MemberAccessExpression)
					{
						context.Log(EXPECTED_TYPE_EXPR_CAST, left->GetSpan());
						//return false;
					}

					std::unique_ptr<SymbolRef> typeRef;
					ParserHelper::MakeConcreteSymbolRef(left.get(), SymbolRefType_Type, typeRef);

					if (!typeRef)
					{
						typeRef = std::make_unique<SymbolRef>(left->GetSpan(), SymbolRefType_Type, false);
					}

					typeRef->TrimCastType();

					auto cast = std::make_unique<CastExpression>(left->GetSpan(), std::move(typeRef), nullptr);
					auto result = ParseOperandIter(context);
					if (result == -1)
					{
						return false;
					}

					context.InjectTask(TaskType_Cast, std::move(cast));
					wasOperator = false;
					return true;
				}
				hadBrackets = current.isDelimiterOf('(');

				wasOperator = false;
				HANDLE_RESULT(ParseOperandIter(context));
			}
			else if (current.isDelimiterOf(expressionDelimiters))
			{
				break;
			}
			else
			{
				if (context.IsEndOfTokens())
				{
					context.Log(UNEXPECTED_EOS, current);
				}
				else
				{
					context.Log(UNEXPECTED_TOKEN, current);
				}
				context.PushOperand(std::make_unique<EmptyExpression>(current.Span));
				context.Advance();
				break;
			}
		}

		context.ReduceOperatorsFinal();
		return true;
	}

	static bool ParseExpressionInner(Parser& parser, TokenStream& stream, std::unique_ptr<Expression>& expressionOut, ParseContext& context)
	{
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
				if (!stream.ExpectOperator(Operator_TernaryElse, EXPECTED_COLON_TERNARY))
				{
					expr->SetSpan(expr->GetSpan().merge(expr->GetTrueBranch()->GetSpan()));
					expr->SetFalseBranch(std::make_unique<EmptyExpression>(TextSpan()));
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

		return context.GetResult(expressionOut);
	}

	bool HybridExpressionParser::ParseExpression(Parser& parser, TokenStream& stream, std::unique_ptr<Expression>& expression, ExpressionParserFlags flags)
	{
		auto start = stream.Current();
		stream.PushState();

		ParseContext context = ParseContext(parser, stream);
		context.flags = flags;
		context.PushParseExpressionTask();

		if (!ParseExpressionInner(parser, stream, expression, context))
		{
			stream.PopState();
			return false;
		}

		stream.PopState(false);

		return true;
	}

	bool HybridExpressionParser::ParseExpression(Parser& parser, TokenStream& stream, std::unique_ptr<Expression>& expression, ParseContext& context)
	{
		auto start = stream.Current();
		stream.PushState();
		if (!ParseExpressionInner(parser, stream, expression, context))
		{
			stream.PopState();
			return false;
		}
		stream.PopState(false);

		return true;
	}
}