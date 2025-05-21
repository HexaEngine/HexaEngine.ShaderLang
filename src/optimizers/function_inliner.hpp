#ifndef FUNCTION_INLINER_HPP
#define FUNCTION_INLINER_HPP

#include "il/il_context.hpp"

namespace HXSL
{
	class FunctionInliner
	{
		ILBuilder& builder;
		std::unordered_map<ILRegister, ILRegister> registerMap;
		std::unordered_map<uint64_t, uint64_t> variableMap;

		bool TryMapOperand(ILOperand& operand);
		ILOperand MapOperand(const ILOperand& operand);
		ILInstruction MapInstr(const ILInstruction& instr);
		ILOperand AddMapping(const ILOperand& op);
	public:
		FunctionInliner(ILBuilder& builder) : builder(builder) {}
		void Inline(ILContext& ctx, uint64_t funcSlot);
	};
}

#endif