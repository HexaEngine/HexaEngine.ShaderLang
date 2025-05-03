#include "common.hpp"

class PrattParserTest : public ASTTestBase
{
public:
	PrattParserTest() : ASTTestBase()
	{
	}
protected:
	std::tuple<CompilationPtr, ASTNodePtr> ActCore(const std::string& input) override
	{
		TokenStream stream;
		Parser parser;
		CompilationPtr compilation;
		PrepareTestData(input, parser, stream, compilation);
		stream.TryAdvance();

		ExpressionPtr expr;
		PrattParser::ParseExpression(parser, stream, parser.Compilation(), expr);
		return std::make_tuple(std::move(compilation), std::move(expr));
	}
};

TEST_P(PrattParserTest, TestWithParameter)
{
	Act();
}

INSTANTIATE_TEST_SUITE_P
(
	PrattParserTests,
	PrattParserTest,
	::testing::Values(
		std::make_tuple("pratt_parser_tests/test_empty.txt", "EmptyExpressionTest"),
		std::make_tuple("pratt_parser_tests/test_simple.txt", "SimpleExpressionTest"),
		std::make_tuple("pratt_parser_tests/test_complex.txt", "ComplexExpressionTest"),
		std::make_tuple("pratt_parser_tests/test_ternary.txt", "TernaryExpressionTest"),
		std::make_tuple("pratt_parser_tests/test_simple_no_semicolon.txt", "SimpleExpressionNoSemicolonTest"),
		std::make_tuple("pratt_parser_tests/test_operator_precedence.txt", "OperatorPrecedenceTest"),
		std::make_tuple("pratt_parser_tests/test_operator_associativity.txt", "OperatorAssociativityTest"),
		std::make_tuple("pratt_parser_tests/test_unary_operator.txt", "UnaryOperatorParsingTest"),
		std::make_tuple("pratt_parser_tests/test_parentheses_grouping.txt", "ParenthesesGroupingTest"),
		std::make_tuple("pratt_parser_tests/test_nested_parentheses.txt", "NestedParenthesesTest"),
		std::make_tuple("pratt_parser_tests/test_multiple_same_precedence.txt", "MultipleSamePrecedenceOperatorsTest"),
		std::make_tuple("pratt_parser_tests/test_division_multiplication.txt", "DivisionAndMultiplicationTest"),
		std::make_tuple("pratt_parser_tests/test_function_call.txt", "FunctionCallTest"),
		std::make_tuple("pratt_parser_tests/test_invalid_syntax.txt", "InvalidSyntaxTest"),
		std::make_tuple("pratt_parser_tests/test_empty_parentheses.txt", "EmptyParenthesesTest")
	),
	[](const testing::TestParamInfo<PrattParserTest::ParamType>& info)
	{
		return std::get<1>(info.param);
	}
);