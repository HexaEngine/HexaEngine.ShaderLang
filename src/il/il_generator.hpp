#ifndef IL_GENERATOR_HPP
#define IL_GENERATOR_HPP

#include "pch/ast.hpp"
#include "pch/il.hpp"

namespace HXSL
{
	class ILGenerator
	{
		ILogger* logger;
		LowerCompilationUnit* compilation;
		std::vector<std::unique_ptr<ILContext>> contexts;
		std::unordered_map<SymbolDef*, size_t> defMap;
		FuncCallGraph<ILContext*> callGraph;

	public:
		ILGenerator(ILogger* logger, LowerCompilationUnit* compilation) : logger(logger), compilation(compilation)
		{
		}

		bool Emit();
	};
}

#endif