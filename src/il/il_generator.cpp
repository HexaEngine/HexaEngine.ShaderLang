#include "il_generator.hpp"
#include "utils/ast_pruner.hpp"

namespace HXSL
{
	bool ILGenerator::Emit()
	{
		std::unordered_map<SymbolDef*, size_t> defMap;
		auto& ilFunctions = compilation->GetILFunctionsMut();
		auto& callGraph = compilation->GetCallGraph();

		for (auto& func : compilation->GetFunctions())
		{
			auto ilFunc = std::make_unique<ILFunction>(compilation, func.get());
			ilFunc->Build();

			callGraph.AddFunction(ilFunc.get());

			auto idx = ilFunctions.size();
			ilFunctions.push_back(std::move(ilFunc));
			defMap.insert({ func.get(), idx });
		}

		if (logger->HasErrors())
		{
			return false;
		}

		ASTPruner pruner;
		pruner.Prune(compilation);

		for (auto& ilFunc : ilFunctions)
		{
			auto& funcRefs = ilFunc->GetMetadata().functions;
			for (size_t i = 0; i < funcRefs.size(); ++i)
			{
				auto& funcRef = funcRefs[i];
				auto it = defMap.find(funcRef.func);
				if (it != defMap.end())
				{
					auto& other = ilFunctions[it->second];
					callGraph.AddCall(ilFunc.get(), other.get());
				}
			}
		}

		return true;
	}
}