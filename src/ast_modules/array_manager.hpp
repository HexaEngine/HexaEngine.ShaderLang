#ifndef ARRAY_MANAGER_HPP
#define ARRAY_MANAGER_HPP

#include "array.hpp"
#include "symbols/symbol_table.hpp"

namespace HXSL
{
	class ArrayManager
	{
	private:
		std::unique_ptr<SymbolTable> arrayTable;
		std::vector<std::unique_ptr<Array>> definitions;
	public:
		ArrayManager() : arrayTable(std::make_unique<SymbolTable>())
		{
		}

		bool TryGetOrCreateArrayType(SymbolRef* ref, SymbolDef* elementType, SymbolHandle& handleOut, SymbolDef*& arrayOut);
	};
}

#endif