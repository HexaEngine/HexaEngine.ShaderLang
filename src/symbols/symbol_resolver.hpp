#ifndef SYMBOL_RESOLVER_HPP
#define SYMBOL_RESOLVER_HPP

#include "analyzers/analyzer.hpp"
#include "ast.hpp"
#include <memory>
#include <stack>
#include <optional>

namespace HXSL
{
	template <typename T>
	class IterStack {
	private:
		std::vector<T> data;

	public:
		IterStack() = default;

		IterStack(const IterStack& other)
		{
			data = other.data;
		}

		IterStack& operator=(const IterStack& other)
		{
			if (this != &other) {
				data = other.data;
			}
			return *this;
		}

		IterStack& operator=(IterStack&& other) noexcept
		{
			if (this != &other) {
				data = std::move(other.data);
			}
			return *this;
		}

		void push(const T& value)
		{
			data.push_back(value);
		}

		void pop()
		{
			if (!empty())
			{
				data.pop_back();
			}
		}

		T& top()
		{
			if (!empty())
			{
				return data.back();
			}
			throw std::out_of_range("Stack is empty");
		}

		bool empty() const
		{
			return data.empty();
		}

		size_t size() const
		{
			return data.size();
		}

		class ReverseIterator
		{
		private:
			typename std::vector<T>::reverse_iterator it;

		public:
			ReverseIterator(typename std::vector<T>::reverse_iterator start) : it(start) {}

			T& operator*() { return *it; }

			ReverseIterator& operator++() {
				++it;
				return *this;
			}

			bool operator!=(const ReverseIterator& other) const {
				return it != other.it;
			}
		};

		class ConstReverseIterator
		{
		private:
			typename std::vector<T>::const_reverse_iterator it;

		public:
			ConstReverseIterator(typename std::vector<T>::const_reverse_iterator start) : it(start) {}

			const T& operator*() { return *it; }

			ConstReverseIterator& operator++() {
				++it;
				return *this;
			}

			bool operator!=(const ConstReverseIterator& other) const {
				return it != other.it;
			}
		};

		ReverseIterator rbegin()
		{
			return ReverseIterator(data.rbegin());
		}

		ReverseIterator rend()
		{
			return ReverseIterator(data.rend());
		}

		ConstReverseIterator rbegin() const
		{
			return ConstReverseIterator(data.rbegin());
		}

		ConstReverseIterator rend() const
		{
			return ConstReverseIterator(data.rend());
		}
	};

	struct ResolverScopeContext
	{
		Namespace* CurrentNamespace;
		ASTNode* Parent;
		size_t SymbolTableIndex;
		uint ScopeCounter;

		ResolverScopeContext() : CurrentNamespace(nullptr), Parent(nullptr), SymbolTableIndex(0), ScopeCounter(0)
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
		SwizzleManager* swizzleManager;
		ResolverScopeContext current;
		IterStack<ResolverScopeContext> stack;
		Compilation* compilation;

	public:
		SymbolResolver(Analyzer& analyzer, const AssemblyCollection& references, const Assembly* targetAssembly, SwizzleManager* swizzleManager)
			: analyzer(analyzer),
			primitives(PrimitiveManager::GetInstance()),
			references(references),
			targetAssembly(targetAssembly),
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

		bool SymbolTypeSanityCheck(SymbolMetadata* metadata, SymbolRef* ref) const;

		bool SymbolVisibilityChecks(SymbolMetadata* metadata, SymbolRef* ref, ResolverScopeContext& context) const;

		bool TryResolve(const SymbolTable* table, const TextSpan& name, size_t nodeIndex, const SymbolTable*& outTable, size_t& outNodeIndex, SymbolDef*& outDefinition) const;

		bool TryResolveInAssemblies(const std::vector<AssemblySymbolRef>& references, const TextSpan& name, const SymbolTable*& outTable, size_t& nodeIndexOut, SymbolDef*& outDefinition) const;

		SymbolDef* ResolveSymbol(const TextSpan& name, const SymbolTable*& outTable, size_t& nodeIndexOut) const;

		SymbolDef* ResolveSymbol(const std::string& str) const
		{
			const SymbolTable* t;
			size_t i;
			return ResolveSymbol(TextSpan(str), t, i);
		}

		bool ResolveSymbol(SymbolRef* ref, std::optional<TextSpan> name = std::nullopt) const;

		bool ResolveSymbol(SymbolRef* ref, std::optional<TextSpan> name, const SymbolTable* table, size_t nodeIndex) const;

		/// <summary>
		///
		/// </summary>
		/// <param name="type"></param>
		/// <param name="getter"></param>
		/// <returns>-1 Failed, 0 Success, 1 Defer</returns>
		int ResolveMemberInner(SymbolRef* type, SymbolRef* refInner) const;

		TraversalBehavior ResolveMember(MemberAccessExpression* memberAccessExpr, ASTNode*& next) const;

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