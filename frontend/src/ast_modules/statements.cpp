#include "statements.hpp"
#include "declarations.hpp"

namespace HXSL
{
	void DeclarationStatement::Write(Stream& stream) const
	{
		//HXSL_ASSERT(false, "Cannot write declaration statements.");
	}

	void DeclarationStatement::Read(Stream& stream, StringPool& container)
	{
		//HXSL_ASSERT(false, "Cannot read declaration statements.");
	}

	void DeclarationStatement::Build(SymbolTable& table, size_t index, CompilationUnit* compilation, std::vector<ast_ptr<SymbolDef>>& nodes)
	{
		//HXSL_ASSERT(false, "Cannot build declaration statements.");
	}
}