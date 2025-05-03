#ifndef HXSL_COMPILER_HPP
#define HXSL_COMPILER_HPP

#include "parsers/parser.h"
#include "analyzers/analyzer.hpp"
#include "generated/localization.hpp"

namespace HXSL
{
	class HXSLCompiler
	{
	public:
		void Compile(const std::vector<std::string>& files, const std::string& output, const AssemblyCollection& references);
	};
}

#endif