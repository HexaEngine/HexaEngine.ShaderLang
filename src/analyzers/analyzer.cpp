#include "analyzer.hpp"
#include "ast.hpp"
#include "sub_analyzer_registry.hpp"
#include "symbols/symbol_table.hpp"
#include "symbols/symbol_resolver.hpp"
#include "symbols/symbol_collector.hpp"
#include "symbols/type_checker.hpp"

namespace HXSL
{
	class DebugVisitor : public Visitor<EmptyDeferralContext>
	{
		TraversalBehavior Visit(ASTNode*& node, size_t depth, bool deferred, EmptyDeferralContext& context) override
		{
			std::string indentation(depth * 2, ' ');
			auto& span = node->GetSpan();
			std::cout << indentation << node->DebugName() << " (Line: " << span.line << " Column: " << span.column << ")" << std::endl;
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

	void Analyzer::InitializeSubSystems()
	{
		SubAnalyzerRegistry::EnsureCreated();
	}

	void Analyzer::WarmupCache()
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

	bool Analyzer::Analyze()
	{
		DebugVisitor debug = DebugVisitor();
		debug.Traverse(compilation);

		SymbolCollector collector(*this, outputAssembly.get());
		collector.Traverse(compilation);

		WarmupCache();

		SymbolResolver resolver(*this, references, outputAssembly.get(), arrayManager.get(), swizzleManager.get());
		resolver.Traverse(compilation);

		compilation->LogFormattedInternal(LogLevel_Verbose, "Symbol resolve initial phase done! %d errors.", compilation->GetErrorCount());

		collector.LateTraverse();

		TypeChecker checker(*this, resolver);
		checker.Traverse(compilation);

		compilation->LogFormattedInternal(LogLevel_Verbose, "Type checks done! %d errors.", compilation->GetErrorCount());

		AnalyzerVisitor visitor(*this);
		visitor.Traverse(compilation);

		compilation->LogFormattedInternal(LogLevel_Verbose, "Post Analysis AST");
		debug.Traverse(compilation);

		return true;
	}
}