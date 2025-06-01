#include "array_manager.hpp"
#include "ast_context.hpp"

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
			handleOut = handle;
			arrayOut = handle.GetMetadata()->declaration;
			ref->SetTable(handle);
			return true;
		}

		auto& dims = ref->GetArrayDims();
		SymbolDef* currentType = elementType;
		for (size_t i = 0; i < dimsCount; i++)
		{
			auto arrayKey = context->GetIdentiferTable().Get(ref->MakeArrayTypeName(i, currentType));

			auto symbolRef = ref->Clone(context);
			symbolRef->OverwriteType(SymbolRefType_Type);
			symbolRef->SetDeclaration(currentType);
			symbolRef->GetArrayDims().pop_back();

			auto array = Array::Create(context, TextSpan(), arrayKey, std::move(symbolRef), dims[i]);

			auto meta = std::make_shared<SymbolMetadata>(SymbolType_Array, SymbolScopeType_Global, AccessModifier_Public, 0, array);
			handle = arrayTable->Insert(array->GetName(), meta);
			array->SetAssembly(arrayAssembly.get(), handle);

			currentType = array;
		}

		handleOut = handle;
		arrayOut = currentType;

		return true;
	}
}