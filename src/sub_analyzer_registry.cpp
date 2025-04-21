#include "sub_analyzer_registry.hpp"

namespace HXSL
{
	std::vector<std::unique_ptr<HXSLSubAnalyzer>> HXSLSubAnalyzerRegistry::analyzers;
}