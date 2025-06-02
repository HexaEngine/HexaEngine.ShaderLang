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

	struct UsingDeclaration
	{
		TextSpan Span;
		IdentifierInfo* Target;
		IdentifierInfo* Alias;
		std::vector<AssemblySymbolRef> AssemblyReferences;

		UsingDeclaration() : Span({}), Target(nullptr), Alias(nullptr) {}
		UsingDeclaration(TextSpan span, IdentifierInfo* name) : Span(span), Target(name), Alias(nullptr) {}
		UsingDeclaration(TextSpan span, IdentifierInfo* name, IdentifierInfo* alias) : Span(span), Target(name), Alias(alias) {}

		bool IsAlias() const noexcept { return Alias != nullptr; }

		bool Warmup(const AssemblyCollection& references);
	};

	class Namespace : public SymbolDef, TrailingObjects<Namespace, ast_ptr<Struct>, ast_ptr<Class>, ast_ptr<FunctionOverload>, ast_ptr<Field>, ast_ptr<Namespace>, UsingDeclaration>
	{
	private:
		uint32_t numStructs;
		uint32_t numClasses;
		uint32_t numFunctions;
		uint32_t numFields;
		uint32_t numNestedNamespaces;
		uint32_t numUsings;
	public:
		static constexpr NodeType ID = NodeType_Namespace;
		Namespace(const TextSpan& span, IdentifierInfo* name)
			: SymbolDef(span, ID, name)
		{
		}

		static Namespace* Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name,
			ArrayRef<ast_ptr<Struct>> structs,
			ArrayRef<ast_ptr<Class>> classes,
			ArrayRef<ast_ptr<FunctionOverload>> functions,
			ArrayRef<ast_ptr<Field>> fields,
			ArrayRef<ast_ptr<Namespace>> nestedNamespaces,
			ArrayRef<UsingDeclaration> usings);

		ArrayRef<ast_ptr<Struct>> GetStructs()
		{
			return { GetTrailingObjects<0>(numStructs, numClasses, numFunctions, numFields, numNestedNamespaces, numUsings), numStructs };
		}

		ArrayRef<ast_ptr<Class>> GetClasses()
		{
			return { GetTrailingObjects<1>(numStructs, numClasses, numFunctions, numFields, numNestedNamespaces, numUsings), numClasses };
		}

		ArrayRef<ast_ptr<FunctionOverload>> GetFunctions()
		{
			return { GetTrailingObjects<2>(numStructs, numClasses, numFunctions, numFields, numNestedNamespaces, numUsings), numFunctions };
		}

		ArrayRef<ast_ptr<Field>> GetFields()
		{
			return { GetTrailingObjects<3>(numStructs, numClasses, numFunctions, numFields, numNestedNamespaces, numUsings), numFields };
		}

		ArrayRef<ast_ptr<Namespace>> GetNestedNamespacess()
		{
			return { GetTrailingObjects<4>(numStructs, numClasses, numFunctions, numFields, numNestedNamespaces, numUsings), numNestedNamespaces };
		}

		ArrayRef<UsingDeclaration> GetUsings()
		{
			return { GetTrailingObjects<5>(numStructs, numClasses, numFunctions, numFields, numNestedNamespaces, numUsings), numUsings };
		}

		void Warmup(const AssemblyCollection& references);
	};
}

#endif