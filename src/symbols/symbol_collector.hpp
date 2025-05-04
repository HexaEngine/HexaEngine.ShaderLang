#ifndef SYMBOL_COLLECTOR_HPP
#define SYMBOL_COLLECTOR_HPP

#include "analyzers/analyzer.hpp"
#include "ast.hpp"
#include <memory>
#include <stack>

namespace HXSL
{
	struct CollectorScopeContext
	{
		const ASTNode* Parent;
		size_t NodeIndex;
		uint32_t ScopeCounter;
		SymbolScopeType Type;

		CollectorScopeContext() : Parent(nullptr), NodeIndex(0), ScopeCounter(0), Type(SymbolScopeType_Global)
		{
		}
	};

	struct CollectorDeferralContext
	{
		CollectorScopeContext current;
		std::stack<CollectorScopeContext> stack;
	};

	class SymbolCollector : public Visitor<EmptyDeferralContext>
	{
	private:

		Analyzer& analyzer;
		Assembly* targetAssembly;
		CollectorScopeContext current;
		std::stack<CollectorScopeContext> stack;
		std::vector<ASTNode*> lateNodes;

		bool Push(const StringSpan& span, SymbolDef* def, std::shared_ptr<SymbolMetadata>& metadata, SymbolScopeType type);

		bool PushScope(const ASTNode* parent, const StringSpan& span, std::shared_ptr<SymbolMetadata>& metadata, SymbolScopeType type);

		bool PushLeaf(SymbolDef* def, std::shared_ptr<SymbolMetadata>& metadata);

		void RegisterForLatePass(ASTNode* node)
		{
			lateNodes.push_back(node);
		}

		void VisitClose(ASTNode* node, size_t depth) override;

		TraversalBehavior Visit(ASTNode*& node, size_t depth, bool deferred, EmptyDeferralContext& context) override;

	public:
		SymbolCollector(Analyzer& analyzer, Assembly* assembly) : analyzer(analyzer), targetAssembly(assembly), current({})
		{
		}

		void LateTraverse();
	};
}
#endif