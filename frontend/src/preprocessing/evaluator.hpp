#ifndef EVALUATOR_HPP
#define EVALUATOR_HPP

#include "lexical/lexer.hpp"

namespace HXSL
{
	struct EvalItem
	{
		Token token;
		bool isUnary = false;

		explicit EvalItem(const Token& token) : token(token), isUnary(false)
		{
		}

		EvalItem(const Token& token, bool isUnary) : token(token), isUnary(isUnary)
		{
		}
	};

	class Evaluator
	{
		std::vector<EvalItem> toPostfix(const std::vector<Token>& infixTokens);

		Number evaluatePostfix(const std::vector<EvalItem>& postfixTokens);

	public:
		Number evaluate(const std::vector<Token>& infixTokens);
	};
}

#endif