#ifndef SYMBOL_RESOLVER_HPP
#define SYMBOL_RESOLVER_HPP

#include "analyzers/analyzer.hpp"
#include "utils/stack.hpp"
#include <memory>
#include <stack>
#include <optional>

namespace HXSL
{
	struct ResolverScopeContext
	{
		Namespace* CurrentNamespace;
		ASTNode* Parent;
		SymbolHandle SymbolHandle;
		uint ScopeCounter;

		ResolverScopeContext() : CurrentNamespace(nullptr), Parent(nullptr), SymbolHandle({}), ScopeCounter(0)
		{
		}
	};

	struct ResolverDeferralContext
	{
		ResolverScopeContext current;
		IterStack<ResolverScopeContext> stack;

		ResolverDeferralContext() = default;

		ResolverDeferralContext(ResolverScopeContext current, IterStack<ResolverScopeContext> stack) : current(current), stack(stack)
		{
		}
	};

	class SymbolResolver : public Visitor<ResolverDeferralContext>
	{
	private:

		Analyzer& analyzer;
		PrimitiveManager& primitives;
		const AssemblyCollection& references;
		const Assembly* targetAssembly;
		ArrayManager* arrayManager;
		SwizzleManager* swizzleManager;
		ResolverScopeContext current;
		IterStack<ResolverScopeContext> stack;
		Compilation* compilation;

	public:
		SymbolResolver(Analyzer& analyzer, const AssemblyCollection& references, const Assembly* targetAssembly, ArrayManager* arrayManager, SwizzleManager* swizzleManager)
			: analyzer(analyzer),
			primitives(PrimitiveManager::GetInstance()),
			references(references),
			targetAssembly(targetAssembly),
			arrayManager(arrayManager),
			swizzleManager(swizzleManager),
			compilation(analyzer.Compilation())
		{
		}

		template <typename... Args>
		void LogError(const std::string& message, const TextSpan& span, Args&&... args) const
		{
			std::string format = message + " (Line: %i, Column: %i)";
			compilation->LogFormatted(LogLevel_Error, format, std::forward<Args>(args)..., span.Line, span.Column);
		}

		bool SymbolTypeSanityCheck(const SymbolMetadata* metadata, SymbolRef* ref, bool silent = false) const;

		bool SymbolVisibilityChecks(const SymbolMetadata* metadata, SymbolRef* ref, ResolverScopeContext& context) const;

		bool TryResolve(const SymbolTable* table, const TextSpan& name, const SymbolHandle& lookup, SymbolHandle& outHandle, SymbolDef*& outDefinition) const;

		bool TryResolveFromRoot(const SymbolTable* table, const TextSpan& name, SymbolHandle& outHandle, SymbolDef*& outDefinition) const;

		bool TryResolveInAssemblies(const std::vector<AssemblySymbolRef>& references, const TextSpan& name, SymbolHandle& outHandle, SymbolDef*& outDefinition) const;

		SymbolDef* ResolveSymbol(const TextSpan& name, bool isFullyQualified, SymbolHandle& outHandle, bool silent = false) const;

		SymbolDef* ResolvePrimitiveSymbol(const std::string& str) const;

		bool ResolveSymbol(SymbolRef* ref, std::optional<TextSpan> name = std::nullopt, bool silent = false) const;

		bool ResolveSymbol(SymbolRef* ref, std::optional<TextSpan> name, const SymbolTable* table, const SymbolHandle& lookup, bool silent = false) const;

		/// <summary>
		///
		/// </summary>
		/// <param name="type"></param>
		/// <param name="getter"></param>
		/// <returns>-1 Failed, 0 Success, 1 Defer, 2 Skip/Terminal</returns>
		int ResolveMemberInner(ChainExpression* expr, SymbolRef* type) const;

		TraversalBehavior ResolveMember(ChainExpression* chainExprRoot, ASTNode*& next, bool skipInitialResolve = false) const;

		TraversalBehavior ResolveMember(ChainExpression* chainExprRoot) const
		{
			ASTNode* next = chainExprRoot;
			return ResolveMember(chainExprRoot, next, true);
		}

		void PushScope(ASTNode* parent, const TextSpan& span, bool external = false);

		void VisitClose(ASTNode* node, size_t depth) override;

		TraversalBehavior VisitExternal(ASTNode*& node, size_t depth, bool deferred, ResolverDeferralContext& context);

		bool UseBeforeDeclarationCheck(SymbolRef* ref, ASTNode* parent) const;

		TraversalBehavior Visit(ASTNode*& node, size_t depth, bool deferred, ResolverDeferralContext& context) override;

		void Traverse(ASTNode* node) override
		{
			auto assemblyBackup = targetAssembly;
			for (auto& reference : references.GetAssemblies())
			{
				targetAssembly = reference.get();
				auto table = reference->GetSymbolTable();
				Visitor::Traverse(table->GetCompilation(), std::bind(&SymbolResolver::VisitExternal, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), std::bind(&SymbolResolver::VisitClose, this, std::placeholders::_1, std::placeholders::_2));
			}

			targetAssembly = assemblyBackup;
			Visitor::Traverse(node);
		}
	};
}
#endif