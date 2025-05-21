#ifndef CONSTANT_FOLDER_HPP
#define CONSTANT_FOLDER_HPP

#include "il/il_instruction.hpp"
#include "il/il_mutator_base.hpp"
#include "il/control_flow_graph.hpp"

namespace HXSL
{
	class ConstantFolder : public ILMutatorBase, public CFGVisitor<EmptyCFGContext>
	{
		std::unordered_map<ILRegister, Number> constants;
		std::unordered_map<uint64_t, Number> varConstants;
		std::unordered_map<ILRegister, ILRegister> registerToRegister;

		void TryFoldOperand(ILOperand& op);

	public:
		ConstantFolder(ControlFlowGraph& cfg, ILBuilder& builder) : ILMutatorBase(builder), CFGVisitor(cfg)
		{
		}

		void Visit(size_t index, CFGNode& node, EmptyCFGContext& context);
	};
}

#endif