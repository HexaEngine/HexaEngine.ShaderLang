#ifndef IL_OPTIMIZER_HPP
#define IL_OPTIMIZER_HPP

#include "pch/il.hpp"
#include "logging/logger.hpp"

namespace HXSL
{
	namespace Backend
	{
		class ILOptimizer
		{
			Module* module;

		public:
			ILOptimizer(ILogger* logger, Module* compilation) : module(compilation)
			{
			}

			void Optimize();

			void Optimize(ILContext* function);

			void OptimizeFunctions(const Span<FunctionLayout*>& functions);
		};
	}
}

#endif