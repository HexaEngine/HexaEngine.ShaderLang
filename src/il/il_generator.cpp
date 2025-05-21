#include "il_generator.hpp"

namespace HXSL
{
	bool ILGenerator::Emit()
	{
		for (auto& func : compilation->GetFunctions())
		{
			auto ctx = std::make_unique<ILContext>(logger, func.get());
			ctx->Build();
			ctx->Print();

			callGraph.AddFunction(ctx.get());

			auto idx = contexts.size();
			contexts.push_back(std::move(ctx));
			defMap.insert({ func.get(), idx });
		}

		for (auto& ctx : contexts)
		{
			auto& funcRefs = ctx->builder.GetMetadata().functions;
			for (size_t i = 0; i < funcRefs.size(); ++i)
			{
				auto& funcRef = funcRefs[i];
				auto it = defMap.find(funcRef.func);
				if (it != defMap.end())
				{
					auto& other = contexts[it->second];
					callGraph.AddCall(ctx.get(), other.get());
				}
			}
			ctx->BuildCFG();
		}

		auto scc = callGraph.ComputeSCCs();

		return true;
	}
}