#include "common.hpp"

TEST_P(PrattParserTest, TestWithParameter)
{
	Act();
}

INSTANTIATE_TEST_SUITE_P(
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

int main(int argc, char** argv)
{
	EnableErrorOutput = false;
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}