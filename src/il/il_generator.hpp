#ifndef IL_GENERATOR_HPP
#define IL_GENERATOR_HPP

#include "pch/ast_ilgen.hpp"
#include "pch/il.hpp"

namespace HXSL
{
	class ILGenerator
	{
		ILogger* logger;
		LowerCompilationUnit* compilation;

	public:
		ILGenerator(ILogger* logger, LowerCompilationUnit* compilation) : logger(logger), compilation(compilation)
		{
		}

		bool Emit();
	};
}

#endif