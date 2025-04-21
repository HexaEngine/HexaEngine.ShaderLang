#ifndef SYMBOL_COLLECTOR_HPP
#define SYMBOL_COLLECTOR_HPP

#include "analyzer.hpp"
#include "nodes.hpp"
#include <memory>
#include <stack>

namespace HXSL
{
	class HXSLSymbolCollector : public HXSLVisitor<EmptyDeferralContext>
	{
	private:

		struct ScopeContext
		{
			const ASTNode* Parent;
			size_t NodeIndex;
			uint ScopeCounter;
			HXSLSymbolScopeType Type;

			ScopeContext() : Parent(nullptr), NodeIndex(0), ScopeCounter(0), Type(HXSLSymbolScopeType_Global)
			{
			}
		};

		HXSLAnalyzer& analyzer;
		Assembly* targetAssembly;
		ScopeContext current;
		std::stack<ScopeContext> stack;

		bool Push(HXSLSymbolDef* def, std::shared_ptr<SymbolMetadata>& metadata, HXSLSymbolScopeType type);

		bool PushScope(const ASTNode* parent, TextSpan span, std::shared_ptr<SymbolMetadata>& metadata, HXSLSymbolScopeType type);

		bool PushLeaf(HXSLSymbolDef* def, std::shared_ptr<SymbolMetadata>& metadata);

	public:
		HXSLSymbolCollector(HXSLAnalyzer& analyzer, Assembly* assembly) : analyzer(analyzer), targetAssembly(assembly), current({})
		{
		}

		void VisitClose(ASTNode* node, size_t depth) override;

		HXSLTraversalBehavior Visit(ASTNode*& node, size_t depth, bool deferred, EmptyDeferralContext& context) override;
	};
}
#endif