#ifndef ARRAY_MANAGER_HPP
#define ARRAY_MANAGER_HPP

#include "array.hpp"
#include "semantics/symbols/symbol_table.hpp"

namespace HXSL
{
	class ArrayManager
	{
	private:
		ASTContext* context;
		std::unique_ptr<Assembly> arrayAssembly = Assembly::Create("");
	public:
		ArrayManager(ASTContext& context) : context(&context)
		{
		}

		bool TryGetOrCreateArrayType(SymbolRef* ref, SymbolDef* elementType, SymbolHandle& handleOut, SymbolDef*& arrayOut);
	};
}

#endif