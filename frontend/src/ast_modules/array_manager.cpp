#include "array_manager.hpp"
#include "ast_context.hpp"

namespace HXSL
{
	bool ArrayManager::TryGetOrCreateArrayType(SymbolRef* ref, SymbolDef* elementType, SymbolHandle& handleOut, SymbolDef*& arrayOut)
	{
		auto arrayRef = dyn_cast<SymbolRefArray>(ref);
		if (!arrayRef)return false;

		auto symType = elementType->GetSymbolType();
		if (symType != SymbolType_Array && symType != SymbolType_Struct && symType != SymbolType_Class && symType != SymbolType_Primitive && symType != SymbolType_Enum)
		{
			return false;
		}

		auto dimsCount = arrayRef->GetArrayDims().size();
		std::string arrayHeadKey = arrayRef->MakeArrayTypeName(dimsCount - 1, elementType); // full name

		auto arrayTable = arrayAssembly->GetMutableSymbolTable();

		auto handle = arrayTable->FindNodeIndexFullPath(arrayHeadKey);
		if (handle.valid())
		{
			handleOut = handle;
			arrayOut = handle.GetMetadata()->declaration;
			arrayRef->SetTable(handle);
			return true;
		}

		auto context = ASTContext::GetCurrentContext();
		auto dims = arrayRef->GetArrayDims();
		SymbolDef* currentType = elementType;
		for (size_t i = 0; i < dimsCount; i++)
		{
			auto arrayKey = context->GetIdentifierTable().Get(arrayRef->MakeArrayTypeName(i, currentType));

			auto symbolRef = ref->Clone();
			symbolRef->OverwriteType(SymbolRefType_Type);
			symbolRef->SetDeclaration(currentType);

			auto array = ArrayDecl::Create(TextSpan(), arrayKey, symbolRef, dims[i]);

			auto meta = SharedPtr<SymbolMetadata>::Create(array);
			handle = arrayTable->Insert(array->GetName(), meta);
			array->SetAssembly(arrayAssembly.get(), handle);

			currentType = array;
		}

		handleOut = handle;
		arrayOut = currentType;

		return true;
	}
}