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
		IdentifierInfo* Name;

		NamespaceDeclaration() = default;
		NamespaceDeclaration(TextSpan span, IdentifierInfo* name) : Span(span), Name(name) {}
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

	class UsingDecl : public ASTNode
	{
		friend class ASTContext;
	private:
		IdentifierInfo* target;
		IdentifierInfo* alias;
		Span<AssemblySymbolRef> assemblyReferences;

		UsingDecl(const TextSpan& span, IdentifierInfo* name, IdentifierInfo* alias) 
			: ASTNode(span, ID), 
			target(name), 
			alias(alias) 
		{
		}
	
	public:
		static constexpr NodeType ID = NodeType_UsingDecl;

		static UsingDecl* Create(const TextSpan& span, IdentifierInfo* name, IdentifierInfo* alias = nullptr);

		DEFINE_GETTER_SETTER_PTR(IdentifierInfo*, Target, target);
		DEFINE_GETTER_SETTER_PTR(IdentifierInfo*, Alias, alias);
		
		bool IsAlias() const noexcept { return alias != nullptr; }

		bool Warmup(const AssemblyCollection& references);

		Span<AssemblySymbolRef> GetAssemblyReferences() const noexcept
		{
			return assemblyReferences;
		}
	};

	class Namespace : public SymbolDef, public TrailingObjects<Namespace, Struct*, Class*, FunctionOverload*, Field*, Namespace*, UsingDecl*>
	{
		friend class ASTContext;
	private:
		TrailingObjStorage<Namespace, uint32_t> storage;
		Span<AssemblySymbolRef> assemblyReferences;
	
		Namespace(const TextSpan& span, IdentifierInfo* name)
			: SymbolDef(span, ID, name)
		{
		}
	public:
		static constexpr NodeType ID = NodeType_Namespace;

		static Namespace* Create(const TextSpan& span, IdentifierInfo* name,
			const ArrayRef<Struct*>& structs,
			const ArrayRef<Class*>& classes,
			const ArrayRef<FunctionOverload*>& functions,
			const ArrayRef<Field*>& fields,
			const ArrayRef<Namespace*>& nestedNamespaces,
			const ArrayRef<UsingDecl*>& usings);

		DEFINE_TRAILING_OBJ_SPAN_GETTER(GetStructs, 0, storage);
		DEFINE_TRAILING_OBJ_SPAN_GETTER(GetClasses, 1, storage);
		DEFINE_TRAILING_OBJ_SPAN_GETTER(GetFunctions, 2, storage);
		DEFINE_TRAILING_OBJ_SPAN_GETTER(GetFields, 3, storage);
		DEFINE_TRAILING_OBJ_SPAN_GETTER(GetNestedNamespaces, 4, storage);
		DEFINE_TRAILING_OBJ_SPAN_GETTER(GetUsings, 5, storage);

		void Warmup(const AssemblyCollection& references);

		Span<AssemblySymbolRef> GetAssemblyReferences() const noexcept
		{
			return assemblyReferences;
		}

		void ForEachChild(ASTChildCallback cb, void* userdata);
		void ForEachChild(ASTConstChildCallback cb, void* userdata) const;
	};
}

#endif