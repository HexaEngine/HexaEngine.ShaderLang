#ifndef LOWER_COMPILATION_UNIT_HPP
#define LOWER_COMPILATION_UNIT_HPP

#include "ast_base.hpp"
#include "symbol_base.hpp"
#include "container.hpp"
#include "io/logger.hpp"

namespace HXSL
{
	class LowerCompilationUnit : public Container
	{
		std::vector<ast_ptr<SymbolDef>> miscDefs;
	public:
		LowerCompilationUnit() : Container({}, NodeType_LowerCompilationUnit), ASTNode({}, NodeType_LowerCompilationUnit) {}

		void AddMiscDef(ast_ptr<SymbolDef> def);

		const std::vector<ast_ptr<SymbolDef>>& GetMiscDefs() const noexcept
		{
			return miscDefs;
		}

		std::vector<ast_ptr<SymbolDef>>& GetMiscDefsMut() noexcept
		{
			return miscDefs;
		}
	};
}

#endif