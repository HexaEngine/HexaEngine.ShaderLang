#ifndef LOOP_UNROLLER_HPP
#define LOOP_UNROLLER_HPP

#include "il_optimizer_pass.hpp"

namespace HXSL
{
	namespace Backend
	{
		class LoopUnroller : public ILOptimizerPass
		{
			dense_map<ILVarId, ILVarId> varMap;

			struct LoopAnalysis
			{
				Instruction* inductionVarPhi = nullptr;
				ILVarId inductionVar;
				Number startValue;
				Number endValue;
				Number stepValue;
				ILOpCode compareOp = OpCode_Noop;
				bool isValidForUnroll = false;
				uint64_t tripCount;
			};

			void MapVariable(Operand*& operand);
			void MapVariable(LoopAnalysis& analysis, Operand*& operand, size_t iteration);
			ILVarId VersionVariable(ILVarId varId);
			void MapInstruction(LoopAnalysis& analysis, Instruction*& instr, size_t iteration);
			LoopAnalysis AnalyzeLoop(LoopNode* loop, ControlFlowGraph& cfg);
			bool UnrollLoop(LoopNode* loop, LoopAnalysis& analysis, ILContext* context);
		public:
			LoopUnroller(ILContext* context) : ILOptimizerPass(context)
			{
			}

			std::string GetName() override { return "LoopUnroller"; }

			OptimizerPassResult Run() override;
		};
	}
}

#endif