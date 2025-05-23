#ifndef IL_CONTEXT_HPP
#define IL_CONTEXT_HPP

#include "il_metadata.hpp"
#include "control_flow_graph.hpp"
#include "io/logger_interface.hpp"

namespace HXSL
{
	struct ILContext : LoggerAdapter
	{
		FunctionOverload* overload;
		ILMetadata metadata;
		ControlFlowGraph cfg;
		bool canInline = false;

		ILContext(ILogger* logger, FunctionOverload* overload) : LoggerAdapter(logger), overload(overload), cfg(metadata)
		{
		}

		ILMetadata& GetMetadata() { return metadata; }

		void Build();

		void BuildCFG();

		void UpdateState();

		void TryInline(ILContext& ctx, uint64_t funcSlot);
	};
}

#endif