#ifndef IL_OPTIMIZER_HPP
#define IL_OPTIMIZER_HPP

#include "ast_ilgen.hpp"

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

		void Optimize(ILContext* function);
	};
}

#endif