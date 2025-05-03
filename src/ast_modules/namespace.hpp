#ifndef NAMESPACE_HPP
#define NAMESPACE_HPP

#include "ast_base.hpp"
#include "symbol_base.hpp"
#include "container.hpp"

namespace HXSL
{
	struct NamespaceDeclaration
	{
		TextSpan Span;
		TextSpan Name;

		NamespaceDeclaration() = default;
		NamespaceDeclaration(TextSpan span, TextSpan name) : Span(span), Name(name) {}
	};

	struct AssemblySymbolRef
	{
		Assembly* TargetAssembly;
		SymbolHandle LookupHandle;

		AssemblySymbolRef() = default;

		AssemblySymbolRef(Assembly* TargetAssembly, const SymbolHandle& lookupHandle)
			: TargetAssembly(TargetAssembly), LookupHandle(lookupHandle)
		{
		}
	};

	struct UsingDeclaration
	{
		TextSpan Span;
		TextSpan Target;
		TextSpan Alias;
		bool IsAlias;
		std::vector<AssemblySymbolRef> AssemblyReferences;

		UsingDeclaration() : IsAlias(false) {}
		UsingDeclaration(TextSpan span, TextSpan name) : Span(span), Target(name), Alias({}), IsAlias(false) {}
		UsingDeclaration(TextSpan span, TextSpan name, TextSpan alias) : Span(span), Target(name), Alias(alias), IsAlias(true) {}

		bool Warmup(const AssemblyCollection& references);
	};

	class Namespace : virtual public ASTNode, public Container, public SymbolDef
	{
	private:
		std::vector<UsingDeclaration> usings;
		std::vector<AssemblySymbolRef> references;
	public:
		Namespace()
			: ASTNode(TextSpan(), NodeType_Namespace, true),
			SymbolDef(TextSpan(), NodeType_Namespace, TextSpan(), true),
			Container(TextSpan(), NodeType_Namespace, true)
		{
		}
		Namespace(const NamespaceDeclaration& declaration)
			: SymbolDef(declaration.Span, NodeType_Namespace, declaration.Name),
			ASTNode(declaration.Span, NodeType_Namespace),
			Container(declaration.Span, NodeType_Namespace)
		{
		}

		void AddUsing(UsingDeclaration _using)
		{
			usings.push_back(_using);
		}

		std::vector<UsingDeclaration>& GetUsings() { return usings; }

		const std::vector<AssemblySymbolRef>& GetAssemblyReferences() { return references; }

		virtual SymbolType GetSymbolType() const { return SymbolType_Namespace; }

		void Write(Stream& stream) const override;

		void Read(Stream& stream, StringPool& container) override;

		void Build(SymbolTable& table, size_t index, Compilation* compilation, std::vector<std::unique_ptr<SymbolDef>>& nodes) override;

		void Warmup(const AssemblyCollection& references);
	};
}

#endif