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
				dense_map<ILLabel, ILLabel> blockMap;
				ILVarId returnVarId;
				std::vector<ILVarId> returnVars;
				BasicBlock* exitNode;
				bool multiBlockMode;

				InlineContext(FunctionLayout* caller, FunctionLayout* callee, CallInstr* callSite, BasicBlock* exitNode, bool multiBlockMode) 
					: caller(caller), callee(callee), callSite(callSite), returnVarId(callSite->GetResult()), exitNode(exitNode), multiBlockMode(multiBlockMode)
				{
				}

				void PrepareBlockMapping();

				ILVarId RemapVarId(const ILVarId& varId);

				void CloneInstruction(Instruction& instr, BasicBlock* block, BasicBlock::instr_iterator target);

				ILVarId AddReturnVar()
				{
					auto callSiteRetId = returnVarId.IncrementVersion();
					returnVars.push_back(callSiteRetId);
					return callSiteRetId;
				}
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