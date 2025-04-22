#include "sub_parser_registry.hpp"

namespace HXSL 
{
	std::vector<std::unique_ptr<SubParser>> SubParserRegistry::parsers;
	std::vector<std::unique_ptr<StatementParser>> StatementParserRegistry::parsers;
	std::vector<std::unique_ptr<ExpressionParser>> ExpressionParserRegistry::parsers;
}