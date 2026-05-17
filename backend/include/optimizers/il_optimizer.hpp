#ifndef IL_OPTIMIZER_HPP
#define IL_OPTIMIZER_HPP

#include "common.hpp"

namespace HXSL
{
	namespace Backend
	{
		class ILOptimizer
		{
			Module* module;
			const OptionCollection& options;

		public:
			ILOptimizer(ILogger* logger, Module* compilation, const OptionCollection& options) : module(compilation), options(options)
			{
			}

			void Optimize();

			void Optimize(ILContext* function);

			void OptimizeFunctions(const Span<FunctionLayout*>& functions);
		};
	}
}

#endif