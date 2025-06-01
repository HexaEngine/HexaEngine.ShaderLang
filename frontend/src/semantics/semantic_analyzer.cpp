#include "semantic_analyzer.hpp"
#include "sub_analyzer_registry.hpp"
#include "symbols/symbol_table.hpp"
#include "symbols/symbol_resolver.hpp"
#include "symbols/symbol_collector.hpp"
#include "type_checker.hpp"
#include "config.h"

namespace HXSL
{
	class DebugVisitor : public ASTVisitor<EmptyDeferralContext>
	{
		size_t size = 0;
		TraversalBehavior Visit(ASTNode*& node, size_t depth, bool deferred, EmptyDeferralContext& context) override
		{
			size += sizeof(*node);
			std::string indentation(depth * 2, ' ');
			auto& span = node->GetSpan();
			std::cout << indentation << node->DebugName() << " (Line: " << span.line << " Column: " << span.column << ")" << std::endl;
			return TraversalBehavior_Keep;
		}
	};

	TraversalBehavior SemanticAnalyzer::AnalyzerVisitor::Visit(ASTNode*& node, size_t depth, bool deferred, EmptyDeferralContext& context)
	{
		return SubAnalyzerRegistry::TryAnalyze(analyzer, node, analyzer.Compilation());
	}

	void SemanticAnalyzer::InitializeSubSystems()
	{
		SubAnalyzerRegistry::EnsureCreated();
	}

	void SemanticAnalyzer::WarmupCache()
	{
		for (auto& us : compilation->GetUsings())
		{
			if (!us.Warmup(references))
			{
				Log(NAMESPACE_OR_TYPE_NOT_FOUND, us.Span, us.Target);
			}
		}

		for (auto& ns : compilation->GetNamespaces())
		{
			for (auto& us : ns->GetUsings())
			{
				if (!us.Warmup(references))
				{
					Log(NAMESPACE_OR_TYPE_NOT_FOUND, us.Span, us.Target);
				}
			}
			ns->Warmup(references);
		}
	}

	bool SemanticAnalyzer::Analyze()
	{
#if HXSL_DEBUG
		DebugVisitor debug = DebugVisitor();
		debug.Traverse(compilation);
#endif

		SymbolCollector collector(*this, outputAssembly.get());
		collector.Traverse(compilation);

		WarmupCache();

		SymbolResolver resolver(*this, references, *outputAssembly.get(), *primitiveManager.get(), *arrayManager.get(), *pointerManager.get(), *swizzleManager.get());
		resolver.Traverse(compilation);

#if HXSL_DEBUG
		logger->LogFormattedInternal(LogLevel_Verbose, "Symbol resolve initial phase done! {} errors.", logger->GetErrorCount());
#endif

		collector.LateTraverse();

		TypeChecker checker(*this, resolver);
		checker.Traverse(compilation);

#if HXSL_DEBUG
		logger->LogFormattedInternal(LogLevel_Verbose, "Type checks done! {} errors.", logger->GetErrorCount());
#endif

		AnalyzerVisitor visitor(*this);
		visitor.Traverse(compilation);

		return true;
	}
}