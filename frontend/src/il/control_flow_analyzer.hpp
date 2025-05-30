#ifndef CONTROL_FLOW_ANALYZER_HPP
#define CONTROL_FLOW_ANALYZER_HPP

#include "logging/logger_adapter.hpp"
#include "pch/il.hpp"

namespace HXSL
{
	namespace Backend
	{
		class ControlFlowAnalyzer : public LoggerAdapter
		{
			Module* module;

			void DetectUnreachableCode(ILContext* function);

		public:
			ControlFlowAnalyzer(ILogger* logger, Module* compilation) : LoggerAdapter(logger), module(compilation)
			{
			}

			void Analyze();
		};
	}
}

#endif