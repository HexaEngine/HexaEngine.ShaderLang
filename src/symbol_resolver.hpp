#ifndef SYMBOL_RESOLVER_HPP
#define SYMBOL_RESOLVER_HPP

#include "analyzer.hpp"
#include "nodes.hpp"
#include <memory>
#include <stack>

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
		HXSLNamespace* CurrentNamespace;
		HXSLNode* Parent;
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

	class HXSLSymbolResolver : public HXSLVisitor<ResolverDeferralContext>
	{
	private:

		HXSLAnalyzer& analyzer;
		HXSLPrimitiveManager& primitives;
		const AssemblyCollection& references;
		const Assembly* targetAssembly;
		HXSLSwizzleManager* swizzleManager;
		ResolverScopeContext current;
		IterStack<ResolverScopeContext> stack;
		HXSLCompilation* compilation;

	public:
		HXSLSymbolResolver(HXSLAnalyzer& analyzer, const AssemblyCollection& references, const Assembly* targetAssembly, HXSLSwizzleManager* swizzleManager)
			: analyzer(analyzer),
			primitives(HXSLPrimitiveManager::GetInstance()),
			references(references),
			targetAssembly(targetAssembly),
			swizzleManager(swizzleManager),
			compilation(analyzer.Compilation())
		{
		}

		bool SymbolTypeSanityCheck(SymbolMetadata* metadata, HXSLSymbolRef* ref) const;

		bool SymbolVisibilityChecks(SymbolMetadata* metadata, HXSLSymbolRef* ref, ResolverScopeContext& context) const;

		bool TryResolve(const SymbolTable* table, const TextSpan& name, size_t nodeIndex, const SymbolTable*& outTable, size_t& outNodeIndex, HXSLSymbolDef*& outDefinition) const;

		bool TryResolveInAssemblies(const std::vector<AssemblySymbolRef>& references, const TextSpan& name, const SymbolTable*& outTable, size_t& nodeIndexOut, HXSLSymbolDef*& outDefinition) const;

		HXSLSymbolDef* ResolveSymbol(const TextSpan& name, const SymbolTable*& outTable, size_t& nodeIndexOut) const;

		HXSLSymbolDef* ResolveSymbol(const std::string& str) const
		{
			const SymbolTable* t;
			size_t i;
			return ResolveSymbol(TextSpan(str), t, i);
		}

		bool ResolveSymbol(HXSLSymbolRef* ref) const;

		/// <summary>
		///
		/// </summary>
		/// <param name="type"></param>
		/// <param name="getter"></param>
		/// <returns>-1 Failed, 0 Success, 1 Defer</returns>
		int ResolveMemberInner(HXSLSymbolRef* type, IHXSLHasSymbolRef* getter) const;

		HXSLTraversalBehavior ResolveMember(HXSLMemberAccessExpression* memberAccessExpr, HXSLNode*& next) const;

		void PushScope(HXSLNode* parent, const TextSpan& span, bool external = false);

		void VisitClose(HXSLNode* node, size_t depth) override;

		HXSLTraversalBehavior VisitExternal(HXSLNode*& node, size_t depth, bool deferred, ResolverDeferralContext& context);

		bool UseBeforeDeclarationCheck(HXSLSymbolRef* ref, HXSLNode* parent) const;

		HXSLTraversalBehavior Visit(HXSLNode*& node, size_t depth, bool deferred, ResolverDeferralContext& context) override;

		HXSLSymbolDef* GetNumericType(const HXSLNumberType& type) const;

		void TypeCheckExpression(HXSLExpression* node);

		void TypeCheckStatement(HXSLNode*& node);

		HXSLTraversalBehavior TypeChecksExpression(HXSLNode*& node, size_t depth, bool deferred, ResolverDeferralContext& context);

		void Traverse(HXSLNode* node) override
		{
			for (auto& reference : references.GetAssemblies())
			{
				auto table = reference->GetSymbolTable();
				HXSLVisitor::Traverse(table->GetCompilation(), std::bind(&HXSLSymbolResolver::VisitExternal, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), std::bind(&HXSLSymbolResolver::VisitClose, this, std::placeholders::_1, std::placeholders::_2));
			}

			HXSLVisitor::Traverse(node);

			HXSLVisitor::Traverse(node, std::bind(&HXSLSymbolResolver::TypeChecksExpression, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), nullptr);
		}
	};
}
#endif