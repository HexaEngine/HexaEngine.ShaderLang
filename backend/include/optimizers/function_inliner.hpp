#ifndef FUNCTION_INLINER_HPP
#define FUNCTION_INLINER_HPP

#include "common.hpp"

namespace HXSL
{
	namespace Backend
	{
		class FunctionInliner
		{
			const OptionCollection& options;

			enum class ParamInfoType
			{
				Unknown,
				VarId,
				Imm,
			};

			struct ParamInfo
			{
				ParamInfoType type;
				union
				{
					ILVarId varId;
					Number imm;
				};

				ParamInfo() : type(ParamInfoType::Unknown), varId(ILVarId()) {}
			};

			struct InlineContext
			{
				FunctionLayout* caller;
				FunctionLayout* callee;
				CallInstr* callSite;

				std::vector<ParamInfo> params;
				dense_map<ILVarId, ILVarId> varIdMap;
				dense_map<ILVarId, ILVarId> baseVarMap;

				InlineContext(FunctionLayout* caller, FunctionLayout* callee, CallInstr* callSite) : caller(caller), callee(callee), callSite(callSite)
				{
				}

				ILVarId RemapVarId(const ILVarId& varId);

				void CloneInstruction(Instruction& instr, BasicBlock* block, BasicBlock::instr_iterator target);
			};

			float ComputeInlineCost(FunctionLayout* funcLayout);

		public:
			FunctionInliner(const OptionCollection& options) : options(options)
			{
			}

			void InlineAtSite(FunctionLayout* caller, FunctionLayout* callee, CallInstr* site);
			void InlineAtAllSites(FunctionLayout* caller, FunctionLayout* callee, const Span<CallInstr*>& sites)
			{
				for (auto& site : sites)
				{
					InlineAtSite(caller, callee, site);
				}
			}

			dense_set<FunctionLayout*> Inline(const Span<FunctionLayout*> functions);
		};
	}
}

#endif