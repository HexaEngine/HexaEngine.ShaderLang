#include "pointer_manager.hpp"
#include "ast_context.hpp"

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
		auto elementTypeN = context->GetIdentiferTable().Get(elementTypeName);
		auto pointerKey = context->GetIdentiferTable().Get(elementTypeName + "*");

		auto table = pointerAssembly->GetMutableSymbolTable();

		auto handle = table->FindNodeIndexFullPath(pointerKey->name);
		if (handle.valid())
		{
			handleOut = handle;
			pointerOut = handle.GetMetadata()->declaration;
			return true;
		}

		auto symbolRef = ast_ptr<SymbolRef>(SymbolRef::Create(context, TextSpan(), elementTypeN, SymbolRefType_Type, true));

		auto pointer = Pointer::Create(context, TextSpan(), pointerKey, std::move(symbolRef));

		auto meta = std::make_shared<SymbolMetadata>(SymbolType_Pointer, SymbolScopeType_Global, AccessModifier_Public, 0, pointer);
		handle = table->Insert(pointer->GetName(), meta);
		pointer->SetAssembly(pointerAssembly.get(), handle);

		handleOut = handle;
		pointerOut = pointer;

		return true;
	}
}