#ifndef COMPILATION_UNIT_HPP
#define COMPILATION_UNIT_HPP

#include "ast_base.hpp"
#include "symbol_base.hpp"
#include "container.hpp"

namespace HXSL
{
	class CompilationUnit : public Container
	{
		std::vector<std::unique_ptr<SymbolDef>> miscDefs;
	public:
		CompilationUnit() : Container({}, NodeType_CompilationUnit), ASTNode({}, NodeType_CompilationUnit) {}

		void AddMiscDef(std::unique_ptr<SymbolDef> def);
	};
}

#endif