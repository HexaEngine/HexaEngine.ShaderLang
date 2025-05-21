#include "semantic_analyzer.hpp"
#include "sub_analyzer_registry.hpp"
#include "symbols/symbol_table.hpp"
#include "symbols/symbol_resolver.hpp"
#include "symbols/symbol_collector.hpp"
#include "type_checker.hpp"

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

	TraversalBehavior SemanticAnalyzer::AnalyzerVisitor::Visit(ASTNode*& node, size_t depth, bool deferred, EmptyDeferralContext& context)
	{
		switch (node->GetType())
		{
		case NodeType_FunctionOverload:
			analyzer.functions.push_back(node->As<FunctionOverload>());
			break;
		case NodeType_OperatorOverload:
			analyzer.operators.push_back(node->As<OperatorOverload>());
			break;
		case NodeType_Struct:
			analyzer.structs.push_back(node->As<Struct>());
			break;
		case NodeType_Class:
			analyzer.classes.push_back(node->As<Class>());
			break;
		default:
			break;
		}
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

		return true;
	}
}