#ifndef COMPILATION_UNIT_HPP
#define COMPILATION_UNIT_HPP

#include "namespace.hpp"

#include <mutex>
#include <shared_mutex>

namespace HXSL
{
	class CompilationUnit : public ASTNode
	{
	private:
		std::vector<ast_ptr<Namespace>> namespaces;
		std::vector<UsingDeclaration> usings;

		std::vector<ast_ptr<Primitive>> primitives;
		std::vector<ast_ptr<Class>> primitiveClasses;

		std::vector<ast_ptr<Array>> arrays;

		std::shared_mutex _mutex;

		friend class PrimitiveManager;
		friend class PrimitiveBuilder;
		friend class SymbolResolver;

		void AddArray(ast_ptr<Array> array)
		{
			arrays.push_back(std::move(array));
		}

	public:
		static constexpr NodeType ID = NodeType_CompilationUnit;
		CompilationUnit(bool isExtern = false) : ASTNode({ }, ID, isExtern)
		{
		}

		Namespace* AddNamespace(const NamespaceDeclaration& declaration);

		void AddNamespace(ast_ptr<Namespace> ns)
		{
			HXSL_ASSERT(isExtern, "Cannot add namespace HXSL manually on non extern compilations.");
			ns->SetParent(this);
			namespaces.push_back(std::move(ns));
		}

		const std::vector<ast_ptr<Namespace>>& GetNamespaces() const noexcept
		{
			return namespaces;
		}

		void AddUsing(UsingDeclaration _using)
		{
			usings.push_back(_using);
		}

		void Clear()
		{
			usings.clear();
			namespaces.clear();
		}

		std::vector<UsingDeclaration>& GetUsings() noexcept { return usings; }
	};
}

#endif