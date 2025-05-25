#include "pointer_manager.hpp"

namespace HXSL
{
	bool PointerManager::TryGetOrCreatePointerType(SymbolRef* ref, SymbolDef* elementType, SymbolHandle& handleOut, SymbolDef*& pointerOut)
	{
		return false;
	}

	bool PointerManager::TryGetOrCreatePointerType(SymbolDef* elementType, SymbolHandle& handleOut, SymbolDef*& pointerOut)
	{
		auto symType = elementType->GetSymbolType();
		if (symType != SymbolType_Pointer && symType != SymbolType_Struct && symType != SymbolType_Class && symType != SymbolType_Primitive && symType != SymbolType_Enum)
		{
			return false;
		}

		std::string elementTypeName = elementType->GetFullyQualifiedName();
		std::string pointerKey = elementTypeName + "*";

		auto table = pointerAssembly->GetMutableSymbolTable();

		auto handle = table->FindNodeIndexFullPath(pointerKey);
		if (handle.valid())
		{
			handleOut = handle;
			pointerOut = handle.GetMetadata()->declaration;
			return true;
		}

		auto symbolRef = make_ast_ptr<SymbolRef>(elementTypeName, SymbolRefType_Type, true);

		auto pointer = std::make_unique<Pointer>(pointerKey, symbolRef);

		auto meta = std::make_shared<SymbolMetadata>(SymbolType_Pointer, SymbolScopeType_Global, AccessModifier_Public, 0, pointer.get());
		handle = table->Insert(pointer->GetName(), meta);
		pointer->SetAssembly(pointerAssembly.get(), handle);

		handleOut = handle;
		pointerOut = pointer.get();

		definitions.push_back(std::move(pointer));

		return true;
	}
}