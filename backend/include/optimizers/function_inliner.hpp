#ifndef FUNCTION_INLINER_HPP
#define FUNCTION_INLINER_HPP

#include "pch/il.hpp"

namespace HXSL
{
	namespace Backend
	{
		class FunctionInliner
		{
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

			FunctionLayout* callerLayout;
			FunctionLayout* calleeLayout;
			std::vector<ParamInfo> params;
			dense_map<ILVarId, ILVarId> varIdMap;
			dense_map<ILVarId, ILVarId> baseVarMap;

			ILVarId RemapVarId(const ILVarId& varId);

		public:
			FunctionInliner(FunctionLayout* caller, FunctionLayout* callee)
				: callerLayout(caller), calleeLayout(callee)
			{
			}
			void InlineAt(CallInstr* site);
			void InlineAll(const Span<CallInstr*>& sites)
			{
				for (auto& site : sites)
				{
					InlineAt(site);
				}
			}
		};
	}
}

#endif