#include "lower_compilation_unit.hpp"
#include "il/il_context.hpp"

namespace HXSL
{
	void LowerCompilationUnit::AddMiscDef(ast_ptr<SymbolDef> def)
	{
		def->SetParent(this);
		miscDefs.push_back(std::move(def));
	}
}