#ifndef ARRAY_MANAGER_HPP
#define ARRAY_MANAGER_HPP

#include "array.hpp"
#include "semantics/symbols/symbol_table.hpp"

namespace HXSL
{
	class ArrayManager
	{
	private:
		std::unique_ptr<Assembly> arrayAssembly = Assembly::Create("");
	public:
		ArrayManager()
		{
		}

		bool TryGetOrCreateArrayType(SymbolRef* ref, SymbolDef* elementType, SymbolHandle& handleOut, SymbolDef*& arrayOut);
	};
}

#endif