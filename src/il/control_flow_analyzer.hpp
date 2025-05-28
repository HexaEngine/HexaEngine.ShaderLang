#ifndef CONTROL_FLOW_ANALYZER_HPP
#define CONTROL_FLOW_ANALYZER_HPP

#include "io/logger_interface.hpp"
#include "ast_ilgen.hpp"
#include "pch/il.hpp"

namespace HXSL
{
	class ControlFlowAnalyzer : public LoggerAdapter
	{
		LowerCompilationUnit* compilation;

		void DetectUnreachableCode(ILContext* function);

	public:
		ControlFlowAnalyzer(ILogger* logger, LowerCompilationUnit* compilation) : LoggerAdapter(logger), compilation(compilation)
		{
		}

		void Analyze();
	};
}

#endif