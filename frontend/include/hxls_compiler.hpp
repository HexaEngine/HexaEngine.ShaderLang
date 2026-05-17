#ifndef HXSL_COMPILER_HPP
#define HXSL_COMPILER_HPP

#include "c/hxsl_compiler.h"
#include "parsers/parser.hpp"
#include "semantics/semantic_analyzer.hpp"
#include "pch/localization.hpp"
#include <core/options.hpp>

namespace HXSL
{
	using OptionCollection = Backend::OptionCollection;

	class Compiler
	{
	private:
		const OptionCollection& options;
		IncludeOpen includeOpen_;
		IncludeClose includeClose_;
		void CompileCore(const std::vector<std::string>& files, const std::string& output, const AssemblyCollection& references);
	public:
		Compiler(const OptionCollection& options) : options(options) {}

		void Compile(const std::vector<std::string>& files, const std::string& output, const ConstSpan<AssemblyReference>& references = {});
		void Compile(const std::vector<std::string>& files, const std::string& output, const AssemblyCollection& references);
		void SetIncludeHandler(IncludeOpen includeOpen, IncludeClose includeClose);
	};
}

#endif