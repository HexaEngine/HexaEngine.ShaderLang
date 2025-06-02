#ifndef COMPILATION_UNIT_HPP
#define COMPILATION_UNIT_HPP

#include "namespace.hpp"

namespace HXSL
{
	template<typename T>
	class ASTBuilder;

	class CompilationUnit : public ASTNode, TrailingObjects<CompilationUnit, ast_ptr<Namespace>>
	{
		friend class ASTContext;
		friend class ASTBuilder<CompilationUnit>;
	private:
		uint32_t numNamespaces;

		CompilationUnit(bool isExtern = false) : ASTNode({ }, ID, isExtern)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_CompilationUnit;
		static CompilationUnit* Create(ASTContext* context, bool isExtern, ArrayRef<ast_ptr<Namespace>> namespaces);
		static CompilationUnit* Create(ASTContext* context, bool isExtern, uint32_t numNamespaces);

		ArrayRef<ast_ptr<Namespace>> GetNamespaces() noexcept
		{
			return { GetTrailingObjects<0>(numNamespaces), numNamespaces };
		}
	};
}

#endif