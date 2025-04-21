#include "sub_parser_registry.hpp"

namespace HXSL
{
	std::vector<std::unique_ptr<HXSLSubParser>> HXSLSubParserRegistry::parsers;
	std::vector<std::unique_ptr<HXSLStatementParser>> HXSLStatementParserRegistry::parsers;
	std::vector<std::unique_ptr<HXSLExpressionParser>> HXSLExpressionParserRegistry::parsers;
}