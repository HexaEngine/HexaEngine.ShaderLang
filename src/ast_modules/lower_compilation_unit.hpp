#ifndef LOWER_COMPILATION_UNIT_HPP
#define LOWER_COMPILATION_UNIT_HPP

#include "pch/ast.hpp"
#include "pch/ast_misc.hpp"
#include "il/func_call_graph.hpp"

namespace HXSL
{
	class ILContext;

	class LowerCompilationUnit : public Container
	{
		std::unique_ptr<ArrayManager> arrayManager;
		std::unique_ptr<PointerManager> pointerManager;
		std::unique_ptr<SwizzleManager> swizzleManager;
		FuncCallGraph<ILContext*> callGraph;
		std::vector<ast_ptr<SymbolDef>> miscDefs;
		std::vector<std::unique_ptr<ILContext>> ilFunctions;
	public:
		LowerCompilationUnit() : Container({}, NodeType_LowerCompilationUnit), ASTNode({}, NodeType_LowerCompilationUnit) {}

		FuncCallGraph<ILContext*>& GetCallGraph()
		{
			return callGraph;
		}

		DEFINE_GET_SET_MOVE(std::unique_ptr<ArrayManager>, ArrayManager, arrayManager)

			DEFINE_GET_SET_MOVE(std::unique_ptr<PointerManager>, PointerManager, pointerManager)

			DEFINE_GET_SET_MOVE(std::unique_ptr<SwizzleManager>, SwizzleManager, swizzleManager)

			void AddMiscDef(ast_ptr<SymbolDef> def);

		const std::vector<ast_ptr<SymbolDef>>& GetMiscDefs() const noexcept
		{
			return miscDefs;
		}

		std::vector<ast_ptr<SymbolDef>>& GetMiscDefsMut() noexcept
		{
			return miscDefs;
		}

		void AddILFunction(std::unique_ptr<ILContext>&& context)
		{
			ilFunctions.push_back(std::move(context));
		}

		const std::vector<std::unique_ptr<ILContext>>& GetILFunctions() const noexcept
		{
			return ilFunctions;
		}

		std::vector<std::unique_ptr<ILContext>>& GetILFunctionsMut() noexcept
		{
			return ilFunctions;
		}
	};
}

#endif