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
		ILContext* context;
		bool changed = false;
		void DiscardInstr(ILInstruction& instr) override
		{
			ILMutatorBase::DiscardInstr(instr);
			changed = true;
		}

	public:
		ILOptimizerPass(ILContext* context) : ILMutatorBase(context->GetMetadata()), context(context) {}
		virtual OptimizerPassResult Run() = 0;
		virtual ~ILOptimizerPass() = default;
	};
}

#endif