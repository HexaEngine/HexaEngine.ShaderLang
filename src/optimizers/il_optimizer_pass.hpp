#ifndef IL_OPTIMIZER_PASS_HPP
#define IL_OPTIMIZER_PASS_HPP

#include "pch/il.hpp"

namespace HXSL
{
	enum OptimizerPassResult
	{
		OptimizerPassResult_None,
		OptimizerPassResult_Changed,
		OptimizerPassResult_Rerun,
	};

	class ILOptimizerPass : protected ILMutatorBase
	{
	protected:
		bool changed = false;
		void DiscardInstr(size_t index) override
		{
			ILMutatorBase::DiscardInstr(index);
			changed = true;
		}

	public:
		ILOptimizerPass(ILMetadata& metadata) : ILMutatorBase(metadata) {}
		virtual OptimizerPassResult Run() = 0;
		virtual ~ILOptimizerPass() = default;
	};
}

#endif