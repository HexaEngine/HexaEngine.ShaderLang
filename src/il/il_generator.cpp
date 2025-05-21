#include "il_generator.hpp"

#include "il/il_context.hpp"
#include "il/func_call_graph.hpp"
#include "utils/ast_pruner.hpp"

bool HXSL::ILGenerator::Emit()
{
	/*
	AstPruner pruner;
	pruner.Flatten(compilation);

	std::vector<std::unique_ptr<ILContext>> contexts;
	std::unordered_map<SymbolDef*, size_t> defMap;
	FuncCallGraph<ILContext*> callGraph;

	for (auto& func : functions)
	{
		auto ctx = std::make_unique<ILContext>(compilation, func);
		ctx->Build();
		ctx->Print();

		callGraph.AddFunction(ctx.get());

		auto idx = contexts.size();
		contexts.push_back(std::move(ctx));
		defMap.insert({ func, idx });
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

	for (auto& ctx : contexts)
	{
		std::cout << "Post first pass fold: " << ctx->overload->GetName() << std::endl;
		ctx->Print();

		auto& funcRefs = ctx->builder.GetMetadata().functions;
		for (size_t i = 0; i < funcRefs.size(); ++i)
		{
			auto& funcRef = funcRefs[i];
			auto it = defMap.find(funcRef.func);
			if (it != defMap.end())
			{
				auto other = contexts[it->second].get();
				ctx->TryInline(*other, i);
			}
		}

		std::cout << "Post inline: " << ctx->overload->GetName() << std::endl;
		ctx->Print();

		ctx->Fold();

		std::cout << "Post second-pass fold: " << ctx->overload->GetName() << std::endl;
		ctx->Print();
	}

	return false;
	*/
	return true;
}