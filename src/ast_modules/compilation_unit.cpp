#include "compilation_unit.hpp"

namespace HXSL
{
	void CompilationUnit::AddMiscDef(std::unique_ptr<SymbolDef> def)
	{
		def->SetParent(this);
		miscDefs.push_back(std::move(def));
	}
}