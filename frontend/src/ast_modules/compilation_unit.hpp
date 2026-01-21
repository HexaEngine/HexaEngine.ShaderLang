#ifndef COMPILATION_UNIT_HPP
#define COMPILATION_UNIT_HPP

#include "namespace.hpp"

namespace HXSL
{
	template<typename T>
	class ASTBuilder;

	class CompilationUnit : public ASTNode, public TrailingObjects<CompilationUnit, Namespace*, UsingDecl*>
	{
		friend class ASTContext;
		friend class ASTBuilder<CompilationUnit>;
	private:
		TrailingObjStorage<CompilationUnit, uint32_t> storage;

		CompilationUnit(bool isExtern = false) : ASTNode({ }, ID, isExtern)
		{
		}

	public:
		static constexpr NodeType ID = NodeType_CompilationUnit;
		static CompilationUnit* Create(bool isExtern, const Span<Namespace*>& namespaces, const Span<UsingDecl*>& usings);
		static CompilationUnit* Create(bool isExtern, uint32_t numNamespaces, uint32_t numUsings);

		DEFINE_TRAILING_OBJ_SPAN_GETTER(GetNamespaces, 0, storage);
		DEFINE_TRAILING_OBJ_SPAN_GETTER(GetUsings, 1, storage);

		void ForEachChild(ASTChildCallback cb, void* userdata);
		void ForEachChild(ASTConstChildCallback cb, void* userdata) const;

		std::string DebugName() const;
	};
}

#endif