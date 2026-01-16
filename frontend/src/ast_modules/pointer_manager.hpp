#ifndef POINTER_MANAGER_HPP
#define POINTER_MANAGER_HPP

#include "pointer.hpp"
#include "semantics/symbols/symbol_table.hpp"

namespace HXSL
{
	class PointerManager
	{
	private:
		std::unique_ptr<Assembly> pointerAssembly = Assembly::Create("");
	public:
		PointerManager() {}
		bool TryGetOrCreatePointerType(SymbolRef* ref, SymbolDef* elementType, SymbolHandle& handleOut, SymbolDef*& pointerOut);

		bool TryGetOrCreatePointerType(SymbolDef* elementType, SymbolHandle& handleOut, SymbolDef*& pointerOut);
	};
}

#endif