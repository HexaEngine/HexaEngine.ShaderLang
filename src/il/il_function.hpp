#ifndef IL_FUNCTION_HPP
#define IL_FUNCTION_HPP

#include "il_metadata.hpp"
#include "control_flow_graph.hpp"

namespace HXSL
{
	struct ILFunction
	{
		FunctionOverload* overload;
		ILMetadata metadata;
		ControlFlowGraph cfg;

		ILFunction(FunctionOverload* overload) : overload(overload), cfg(metadata)
		{
		}

		ILMetadata& GetMetadata() { return metadata; }

		void Build();
	};
}

#endif