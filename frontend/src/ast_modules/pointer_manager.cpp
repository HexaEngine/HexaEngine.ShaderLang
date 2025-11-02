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

		auto context = ASTContext::GetCurrentContext();
		auto elementTypeName = elementType->GetFullyQualifiedName().str();
		auto elementTypeN = context->GetIdentifierTable().Get(elementTypeName);
		auto pointerKey = context->GetIdentifierTable().Get(elementTypeName + "*");

		auto table = pointerAssembly->GetMutableSymbolTable();

		auto handle = table->FindNodeIndexFullPath(pointerKey->name);
		if (handle.valid())
		{
			handleOut = handle;
			pointerOut = handle.GetMetadata()->declaration;
			return true;
		}

		auto symbolRef = SymbolRef::Create(TextSpan(), elementTypeN, SymbolRefType_Type, true);

		auto pointer = Pointer::Create(TextSpan(), pointerKey, symbolRef);

		auto meta = std::make_shared<SymbolMetadata>(SymbolType_Pointer, SymbolScopeType_Global, AccessModifier_Public, 0, pointer);
		handle = table->Insert(pointer->GetName(), meta);
		pointer->SetAssembly(pointerAssembly.get(), handle);

		handleOut = handle;
		pointerOut = pointer;

		return true;
	}
}