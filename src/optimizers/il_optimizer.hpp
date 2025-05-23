#ifndef IL_OPTIMIZER_HPP
#define IL_OPTIMIZER_HPP

#include "pch/ast_ilgen.hpp"

namespace HXSL
{
	class ILOptimizer
	{
		LowerCompilationUnit* compilation;

	public:
		ILOptimizer(ILogger* logger, LowerCompilationUnit* compilation) : compilation(compilation)
		{
		}

		void Optimize();

		void Optimize(ILFunction* function);
	};
}

#endif