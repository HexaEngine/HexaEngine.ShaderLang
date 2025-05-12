#ifndef HXSL_COMPILER_HPP
#define HXSL_COMPILER_HPP

#include "c/hxsl_compiler.h"
#include "parsers/parser.hpp"
#include "analyzers/analyzer.hpp"
#include "generated/localization.hpp"

namespace HXSL
{
	class Compiler
	{
	private:
		IncludeOpen includeOpen_;
		IncludeClose includeClose_;
	public:
		void Compile(const std::vector<std::string>& files, const std::string& output, const AssemblyCollection& references);
		void SetIncludeHandler(IncludeOpen includeOpen, IncludeClose includeClose);
	};
}

#endif