#ifndef HYBRID_EXPRESSION_PARSER_HPP
#define HYBRID_EXPRESSION_PARSER_HPP

#include "parser.hpp"

namespace HXSL
{
	using ExpressionPtr = ast_ptr<Expression>;
	using BinaryExpressionPtr = ast_ptr<BinaryExpression>;
	using OperatorPtr = ast_ptr<OperatorExpression>;

	enum ExpressionParserFlags
	{
		ExpressionParserFlags_None = 0,
		ExpressionParserFlags_SwitchCase = 1,
	};

	DEFINE_FLAGS_OPERATORS(ExpressionParserFlags, int);

	struct ExpressionParserOptions
	{
		ExpressionParserFlags flags;
		std::vector<ast_ptr<Expression>> operands;
		std::vector<ast_ptr<OperatorExpression>> operators;
	};

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
		ExpressionPtr result;
		bool wasOperator;
		bool hadBrackets;
		TextSpan start;

		TaskFrame(const TaskType& type, const size_t& stackLimit, ExpressionPtr result)
			: type(type), stackBoundary(stackLimit), result(std::move(result)), wasOperator(true), hadBrackets(false)
		{
		}
		TaskFrame(const TaskType& type, const size_t& stackLimit, ExpressionPtr result, TextSpan start)
			: type(type), stackBoundary(stackLimit), result(std::move(result)), wasOperator(true), hadBrackets(false), start(start)
		{
		}

		TaskFrame() : type(TaskType_Unknown), stackBoundary(0), wasOperator(true), hadBrackets(false)
		{
		}
	};

	struct ParseContext : public TokenStreamAdapter, public ParserAdapter
	{
		std::stack<TaskFrame> tasks;
		std::stack<ExpressionPtr> operandStack;
		std::stack<OperatorPtr> operatorStack;
		TaskFrame currentTask;
		std::stack<size_t> ternaryStack;
		ExpressionParserFlags flags;
		bool canInject = false;

		ParseContext(Parser& parser, TokenStream& stream) : TokenStreamAdapter(stream), ParserAdapter(parser), flags(ExpressionParserFlags_None)
		{
		}

		bool NextTask();

		bool IsInTernary() const noexcept { return !ternaryStack.empty() && ternaryStack.top() == tasks.size() + 1; }

		void PushTask(TaskFrame&& frame)
		{
			tasks.push(std::move(frame));
			canInject = false;
		}

		void PushCurrentTask()
		{
			PushTask(std::move(currentTask));
		}

		void PushParseExpressionTask()
		{
			PushTask(TaskFrame(TaskType_ParseExpression, operatorStack.size(), nullptr));
		}

		void PushBracketCloseTask()
		{
			currentTask.hadBrackets = true;
			PushCurrentTask();
			PushTask(TaskFrame(TaskType_BracketClose, operatorStack.size(), nullptr, LastToken().Span));
			PushParseExpressionTask();
			canInject = true;
		}

		void PushSubExpressionTask(TaskType type, ExpressionPtr&& expr)
		{
			PushCurrentTask();
			PushTask(TaskFrame(type, operatorStack.size(), std::move(expr)));
			PushParseExpressionTask();
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
			PushTask(TaskFrame(type, operatorStack.size(), std::move(expr)));
			PushParseExpressionTask();
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
			PushSubExpressionTask(type, make_ast_ptr<ExpressionType>(std::forward<Args>(args)...));
		}

		void InjectTask(TaskType type, ExpressionPtr&& expr);

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
			PushOperator(make_ast_ptr<ExpressionType>(std::forward<Args>(args)...));
		}

		ExpressionPtr PopOperand();

		bool HasOperand() const
		{
			return !operandStack.empty();
		}

		const ExpressionPtr& TopOperand() const
		{
			return operandStack.top();
		}

		void TryReduceOperators(int precedence);

		void ReduceOperatorsFinal();

		bool GetResult(ExpressionPtr& exprOut)
		{
			if (operandStack.size() == 1)
			{
				exprOut = std::move(operandStack.top());
				return true;
			}
			return false;
		}
	};

	class HybridExpressionParser
	{
	public:
		static bool ParseExpression(Parser& parser, TokenStream& stream, ast_ptr<Expression>& expression, ExpressionParserFlags flags = ExpressionParserFlags_None);

		static bool ParseExpression(Parser& parser, TokenStream& stream, ast_ptr<Expression>& expression, ParseContext& context);
	};
}

#endif