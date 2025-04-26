#include "array_manager.hpp"

namespace HXSL
{
	bool ArrayManager::TryGetOrCreateArrayType(SymbolRef* ref, SymbolDef* elementType, SymbolHandle& handleOut, SymbolDef*& arrayOut)
	{
		auto symType = elementType->GetSymbolType();
		if (symType != SymbolType_Array && symType != SymbolType_Struct && symType != SymbolType_Class && symType != SymbolType_Primitive && symType != SymbolType_Enum)
		{
			return false;
		}

		std::string arrayKey = ref->MakeArrayTypeName(0, elementType);

		auto handle = arrayTable->FindNodeIndexFullPath(arrayKey);
		if (handle.valid())
		{
			ref->SetTable(handle);
			return true;
		}

		auto symbolRef = ref->Clone();
		symbolRef->OverwriteType(SymbolRefType_Type);
		symbolRef->SetDeclaration(elementType);

		auto& dims = ref->GetArrayDims();
		auto array = std::make_unique<Array>(arrayKey, symbolRef, dims);

		auto meta = std::make_shared<SymbolMetadata>(SymbolType_Array, SymbolScopeType_Global, AccessModifier_Public, 0, array.get());
		handle = arrayTable->Insert(array->GetName(), meta);

		handleOut = handle;
		arrayOut = array.get();
		definitions.push_back(std::move(array));
		return true;
	}
}