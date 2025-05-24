#ifndef LOWER_COMPILATION_UNIT_HPP
#define LOWER_COMPILATION_UNIT_HPP

#include "pch/ast.hpp"
#include "pch/il.hpp"

namespace HXSL
{
	class LowerCompilationUnit : public Container
	{
		FuncCallGraph<ILFunction*> callGraph;
		std::vector<ast_ptr<SymbolDef>> miscDefs;
		std::vector<std::unique_ptr<ILFunction>> ilFunctions;
	public:
		LowerCompilationUnit() : Container({}, NodeType_LowerCompilationUnit), ASTNode({}, NodeType_LowerCompilationUnit) {}

		FuncCallGraph<ILFunction*>& GetCallGraph()
		{
			return callGraph;
		}

		void AddMiscDef(ast_ptr<SymbolDef> def);

		const std::vector<ast_ptr<SymbolDef>>& GetMiscDefs() const noexcept
		{
			return miscDefs;
		}

		std::vector<ast_ptr<SymbolDef>>& GetMiscDefsMut() noexcept
		{
			return miscDefs;
		}

		void AddILFunction(std::unique_ptr<ILFunction>&& context)
		{
			ilFunctions.push_back(std::move(context));
		}

		const std::vector<std::unique_ptr<ILFunction>>& GetILFunctions() const noexcept
		{
			return ilFunctions;
		}

		std::vector<std::unique_ptr<ILFunction>>& GetILFunctionsMut() noexcept
		{
			return ilFunctions;
		}
	};
}

#endif