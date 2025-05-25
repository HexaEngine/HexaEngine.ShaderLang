#ifndef IL_FUNCTION_HPP
#define IL_FUNCTION_HPP

#include "il_metadata.hpp"
#include "control_flow_graph.hpp"

namespace HXSL
{
	class ILFunction
	{
	public:
		LowerCompilationUnit* compilation;
		FunctionOverload* overload;
		ILMetadata metadata;
		ControlFlowGraph cfg;

		ILFunction(LowerCompilationUnit* compilation, FunctionOverload* overload) : compilation(compilation), overload(overload), cfg(metadata)
		{
		}

		ILMetadata& GetMetadata() { return metadata; }

		void Build();
	};
}

#endif