#include "analyzer.hpp"
#include "primitives.hpp"
#include "ast.hpp"
#include "sub_analyzer_registry.hpp"
#include "symbols/symbol_table.hpp"
#include "symbols/symbol_resolver.hpp"
#include "symbols/symbol_collector.hpp"

namespace HXSL
{
	class DebugVisitor : public Visitor<EmptyDeferralContext>
	{
		TraversalBehavior Visit(ASTNode*& node, size_t depth, bool deferred, EmptyDeferralContext& context) override
		{
			std::string indentation(depth * 2, ' ');
			auto& span = node->GetSpan();
			std::cout << indentation << node->DebugName() << " (Line: " << span.Line << " Column: " << span.Column << ")" << std::endl;
			return TraversalBehavior_Keep;
		}
	};

	class AnalyzerVisitor : public Visitor<EmptyDeferralContext>
	{
	private:
		Analyzer& analyzer;

	public:
		AnalyzerVisitor(Analyzer& analyzer) : analyzer(analyzer)
		{
		}

		TraversalBehavior Visit(ASTNode*& node, size_t depth, bool deferred, EmptyDeferralContext& context) override
		{
			return SubAnalyzerRegistry::TryAnalyze(analyzer, node, analyzer.Compilation());
		}
	};

	bool Analyzer::Analyze()
	{
		DebugVisitor debug = DebugVisitor();
		debug.Traverse(compilation);

		SymbolCollector collector(*this, outputAssembly.get());
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

		SymbolResolver resolver(*this, references, outputAssembly.get(), swizzleManager.get());
		resolver.Traverse(compilation);

		AnalyzerVisitor visitor(*this);
		visitor.Traverse(compilation);

		return true;
	}
}