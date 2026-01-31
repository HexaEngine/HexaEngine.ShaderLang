#ifndef SYMBOL_RESOLVER_HPP
#define SYMBOL_RESOLVER_HPP

#include "semantics/semantic_analyzer.hpp"
#include "utils/stack.hpp"

namespace HXSL
{
	struct ResolverScopeContext
	{
		Namespace* CurrentNamespace;
		ASTNode* Parent;
		SymbolHandle SymbolHandle;
		uint32_t ScopeCounter;

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

	enum class ResolveMemberResult
	{
		Failure = -1, /// Resolution failed
		Success = 0,  /// Resolution succeeded
		Defer = 1,  /// Resolution should be deferred
		Skip = 2   /// Skip resolution (e.g., function call or indexer)
	};

	class SymbolResolver : public ASTVisitor<ResolverDeferralContext>
	{
	private:

		SemanticAnalyzer& analyzer;
		PrimitiveManager& primitives;
		const AssemblyCollection& references;
		const Assembly* targetAssembly;
		ArrayManager& arrayManager;
		SwizzleManager& swizzleManager;
		ResolverScopeContext current;
		IterStack<ResolverScopeContext> stack;
		CompilationUnit* compilation;

	public:
		SymbolResolver(SemanticAnalyzer& analyzer, const AssemblyCollection& references, const Assembly& targetAssembly, PrimitiveManager& primitiveManager, ArrayManager& arrayManager, PointerManager& pointerManager, SwizzleManager& swizzleManager)
			: analyzer(analyzer),
			primitives(primitiveManager),
			references(references),
			targetAssembly(&targetAssembly),
			arrayManager(arrayManager),
			swizzleManager(swizzleManager),
			compilation(analyzer.Compilation())
		{
		}

		bool SymbolTypeSanityCheck(const SymbolMetadata* metadata, SymbolRef* ref, bool silent = false) const;

		bool SymbolVisibilityChecks(const SymbolMetadata* metadata, SymbolRef* ref, ResolverScopeContext& context) const;

		bool TryResolveThis(SymbolHandle& outHandle, SymbolDef*& outDefinition) const;

		bool TryResolve(const SymbolTable* table, const StringSpan& name, const SymbolHandle& lookup, SymbolHandle& outHandle, SymbolDef*& outDefinition) const;

		bool TryResolveFromRoot(const SymbolTable* table, const StringSpan& name, SymbolHandle& outHandle, SymbolDef*& outDefinition) const;

		bool TryResolveInAssemblies(const Span<AssemblySymbolRef>& references, const StringSpan& name, SymbolHandle& outHandle, SymbolDef*& outDefinition) const;

		SymbolDef* ResolveSymbol(const TextSpan& span, const StringSpan& name, bool isFullyQualified, SymbolHandle& outHandle, bool silent = false) const;

		SymbolDef* ResolvePrimitiveSymbol(const std::string& str) const;

		bool ResolveSymbol(SymbolRef* ref, std::optional<StringSpan> name = std::nullopt, bool silent = false) const;

		bool ResolveSymbol(SymbolRef* ref, std::optional<StringSpan> name, const SymbolTable* table, const SymbolHandle& lookup, bool silent = false) const;

		/**
		 * @brief Attempts to resolve the constructor call represented by the given expression.
		 *
		 * This function checks whether the given expression corresponds to a constructor call.
		 *
		 * @param funcCallExpr Pointer to the function call expression to analyze.
		 * @param[out] outDefinition Pointer to the declaration. The pointer to the constructor overload if the ctor overload was successfully resolved; nullptr otherwise.
		 * @param[out] success Optional output parameter. If provided, set to true if the constructor overload
		 *                    was successfully resolved; false otherwise. Defaults to nullptr.
		 * @param silent If true, suppresses error logging for resolution failures. Defaults to false.
		 *
		 * @return true if the expression represents a constructor call (regardless of resolution success).
		 * @return false if the expression does not represent a constructor call.
		 */
		bool ResolveConstructor(FunctionCallExpression* funcCallExpr, SymbolDef*& outDefinition, bool silent = false) const;

		bool ResolveFunction(FunctionCallExpression* funcCallExpr, SymbolDef*& outDefinition, bool silent = false) const;

		/**
		 * @brief Attempts to resolve a member within a chain expression.
		 *
		 * @param expr The chain expression containing the member access.
		 * @param type The base symbol reference to resolve against.
		 * @return A ResolveMemberResult indicating the resolution outcome.
		 */
		ResolveMemberResult ResolveMemberInner(ChainExpression* expr, SymbolRef* type) const;

		TraversalBehavior ResolveMember(ChainExpression* chainExprRoot, ASTNode*& next, bool skipInitialResolve = false) const;

		TraversalBehavior ResolveMember(ChainExpression* chainExprRoot) const
		{
			ASTNode* next = chainExprRoot;
			return ResolveMember(chainExprRoot, next, true);
		}

		void PushScope(ASTNode* parent, const StringSpan& span, bool external = false, SymbolHandle* searchHandle = nullptr);

		void PopScope();

		ResolverScopeContext& CurrentScope() noexcept { return current; }

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
				ASTVisitor::Traverse(reference->GetCompilation(), std::bind(&SymbolResolver::VisitExternal, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), std::bind(&SymbolResolver::VisitClose, this, std::placeholders::_1, std::placeholders::_2));
			}

			targetAssembly = assemblyBackup;
			ASTVisitor::Traverse(node);
		}
	};
}
#endif