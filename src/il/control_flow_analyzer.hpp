#ifndef CONTROL_FLOW_ANALYZER_HPP
#define CONTROL_FLOW_ANALYZER_HPP

#include "io/logger_interface.hpp"
#include "pch/ast_ilgen.hpp"
#include "pch/il.hpp"

namespace HXSL
{
	class ControlFlowAnalyzer : public LoggerAdapter
	{
		LowerCompilationUnit* compilation;

		void DetectUnreachableCode(ILFunction* function);

	public:
		ControlFlowAnalyzer(ILogger* logger, LowerCompilationUnit* compilation) : LoggerAdapter(logger), compilation(compilation)
		{
		}

		void Analyze();
	};
}

#endif