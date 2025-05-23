#ifndef CONSTANT_FOLDER_HPP
#define CONSTANT_FOLDER_HPP

#include "pch/il.hpp"

namespace HXSL
{
	class ConstantFolder : public CFGVisitor<EmptyCFGContext>, ILMutatorBase
	{
		std::unordered_map<ILRegister, Number> constants;
		std::unordered_map<uint64_t, Number> varConstants;
		std::unordered_map<ILRegister, ILRegister> registerToRegister;

		void TryFoldOperand(ILOperand& op);

	public:
		ConstantFolder(ControlFlowGraph& cfg, ILMetadata& metadata) : ILMutatorBase(metadata), CFGVisitor(cfg)
		{
		}

		void Visit(size_t index, CFGNode& node, EmptyCFGContext& context);
	};
}

#endif