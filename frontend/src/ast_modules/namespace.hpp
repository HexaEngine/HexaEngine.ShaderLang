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

	class Namespace : public SymbolDef
	{
	private:
		std::vector<ast_ptr<Struct>> structs;
		std::vector<ast_ptr<Class>> classes;
		std::vector<ast_ptr<FunctionOverload>> functions;
		std::vector<ast_ptr<Field>> fields;
		std::vector<ast_ptr<Namespace>> nestedNamespaces;
		std::vector<UsingDeclaration> usings;
		std::vector<AssemblySymbolRef> references;
	public:
		static constexpr NodeType ID = NodeType_Namespace;
		Namespace(const NamespaceDeclaration& declaration)
			: SymbolDef(declaration.Span, ID, declaration.Name)
		{
		}

		DEFINE_GET_SET_MOVE(std::vector<ast_ptr<FunctionOverload>>, Functions, functions);

		DEFINE_GET_SET_MOVE(std::vector<ast_ptr<Struct>>, Structs, structs);

		DEFINE_GET_SET_MOVE(std::vector<ast_ptr<Class>>, Classes, classes);

		DEFINE_GET_SET_MOVE(std::vector<ast_ptr<Field>>, Fields, fields);

		DEFINE_GET_SET_MOVE(std::vector<ast_ptr<Namespace>>, NestedNamespaces, nestedNamespaces);

		void AddNestedNamespace(ast_ptr<Namespace>& nestedNs)
		{
			RegisterChild(nestedNs);
			nestedNamespaces.push_back(std::move(nestedNs));
		}

		void AddUsing(UsingDeclaration _using)
		{
			usings.push_back(_using);
		}

		void AddFunction(ast_ptr<FunctionOverload> function)
		{
			RegisterChild(function);
			functions.push_back(std::move(function));
		}

		void AddStruct(ast_ptr<Struct> _struct)
		{
			RegisterChild(_struct);
			structs.push_back(std::move(_struct));
		}

		void AddClass(ast_ptr<Class> _class)
		{
			RegisterChild(_class);
			classes.push_back(std::move(_class));
		}

		void AddField(ast_ptr<Field> field)
		{
			RegisterChild(field);
			fields.push_back(std::move(field));
		}

		std::vector<UsingDeclaration>& GetUsings() { return usings; }

		const std::vector<AssemblySymbolRef>& GetAssemblyReferences() const { return references; }

		void Warmup(const AssemblyCollection& references);
	};
}

#endif