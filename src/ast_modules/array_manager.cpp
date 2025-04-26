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

		auto dimsCount = ref->GetArrayDimCount();
		std::string arrayHeadKey = ref->MakeArrayTypeName(dimsCount - 1, elementType); // full name

		auto arrayTable = arrayAssembly->GetMutableSymbolTable();

		auto handle = arrayTable->FindNodeIndexFullPath(arrayHeadKey);
		if (handle.valid())
		{
			ref->SetTable(handle);
			return true;
		}

		auto& dims = ref->GetArrayDims();
		SymbolDef* currentType = elementType;
		for (size_t i = 0; i < dimsCount; i++)
		{
			std::string arrayKey = ref->MakeArrayTypeName(i, currentType);

			auto symbolRef = ref->Clone();
			symbolRef->OverwriteType(SymbolRefType_Type);
			symbolRef->SetDeclaration(currentType);
			symbolRef->GetArrayDims().pop_back();

			auto array = std::make_unique<Array>(arrayKey, symbolRef, dims[i]);

			auto meta = std::make_shared<SymbolMetadata>(SymbolType_Array, SymbolScopeType_Global, AccessModifier_Public, 0, array.get());
			handle = arrayTable->Insert(array->GetName(), meta);
			array->SetAssembly(arrayAssembly.get(), handle);

			currentType = array.get();

			definitions.push_back(std::move(array));
		}

		handleOut = handle;
		arrayOut = currentType;

		return true;
	}
}