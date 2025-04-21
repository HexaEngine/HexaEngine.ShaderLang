#include "analyzer.hpp"
#include "symbol_table.hpp"
#include "primitives.hpp"
#include "nodes.hpp"
#include "sub_analyzer_registry.hpp"
#include "symbol_resolver.hpp"
#include "symbol_collector.hpp"

namespace HXSL
{
	class HXSLDebugVisitor : public HXSLVisitor<EmptyDeferralContext>
	{
		HXSLTraversalBehavior Visit(ASTNode*& node, size_t depth, bool deferred, EmptyDeferralContext& context) override
		{
			std::string indentation(depth * 2, ' ');
			auto& span = node->GetSpan();
			std::cout << indentation << node->DebugName() << " (Line: " << span.Line << " Column: " << span.Column << ")" << std::endl;
			return HXSLTraversalBehavior_Keep;
		}
	};

	class HXSLAnalyzerVisitor : public HXSLVisitor<EmptyDeferralContext>
	{
	private:
		HXSLAnalyzer& analyzer;

	public:
		HXSLAnalyzerVisitor(HXSLAnalyzer& analyzer) : analyzer(analyzer)
		{
		}

		HXSLTraversalBehavior Visit(ASTNode*& node, size_t depth, bool deferred, EmptyDeferralContext& context) override
		{
			return HXSLSubAnalyzerRegistry::TryAnalyze(analyzer, node, analyzer.Compilation());
		}
	};

	bool HXSLAnalyzer::Analyze()
	{
		HXSLDebugVisitor debug = HXSLDebugVisitor();
		debug.Traverse(compilation);

		HXSLSymbolCollector collector(*this, outputAssembly.get());
		collector.Traverse(compilation);

		for (auto& us : compilation->GetUsings())
		{
			HXSL_ASSERT(us.Warmup(references), "Needs better error handling lol");
		}

		for (auto& ns : compilation->GetNamespaces())
		{
			for (auto& us : ns->GetUsings())
			{
				HXSL_ASSERT(us.Warmup(references), "Needs better error handling lol");
			}
			ns->Warmup(references);
		}

		HXSLSymbolResolver resolver(*this, references, outputAssembly.get(), swizzleManager.get());
		resolver.Traverse(compilation);

		HXSLAnalyzerVisitor visitor(*this);
		visitor.Traverse(compilation);

		return true;
	}
}