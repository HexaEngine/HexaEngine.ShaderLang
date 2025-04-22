#ifndef SYMBOL_COLLECTOR_HPP
#define SYMBOL_COLLECTOR_HPP

#include "analyzer.hpp"
#include "ast.hpp"
#include <memory>
#include <stack>

namespace HXSL
{
	class SymbolCollector : public Visitor<EmptyDeferralContext>
	{
	private:

		struct ScopeContext
		{
			const ASTNode* Parent;
			size_t NodeIndex;
			uint ScopeCounter;
			SymbolScopeType Type;

			ScopeContext() : Parent(nullptr), NodeIndex(0), ScopeCounter(0), Type(SymbolScopeType_Global)
			{
			}
		};

		Analyzer& analyzer;
		Assembly* targetAssembly;
		ScopeContext current;
		std::stack<ScopeContext> stack;

		bool Push(SymbolDef* def, std::shared_ptr<SymbolMetadata>& metadata, SymbolScopeType type);

		bool PushScope(const ASTNode* parent, TextSpan span, std::shared_ptr<SymbolMetadata>& metadata, SymbolScopeType type);

		bool PushLeaf(SymbolDef* def, std::shared_ptr<SymbolMetadata>& metadata);

	public:
		SymbolCollector(Analyzer& analyzer, Assembly* assembly) : analyzer(analyzer), targetAssembly(assembly), current({})
		{
		}

		void VisitClose(ASTNode* node, size_t depth) override;

		TraversalBehavior Visit(ASTNode*& node, size_t depth, bool deferred, EmptyDeferralContext& context) override;
	};
}
#endif