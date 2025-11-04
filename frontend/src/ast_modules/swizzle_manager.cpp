#include "swizzle_manager.hpp"
#include "node_builder.hpp"

namespace HXSL
{
	bool SwizzleManager::VerifySwizzle(Primitive* prim, SymbolRef* ref)
	{
		auto _class = prim->GetClass();
		if (_class == PrimitiveClass_Matrix) return false;
		auto& pattern = ref->GetName();
		if (pattern.size() < 1 || pattern.size() > 4)
			return false;

		auto primHandle = swizzleTable->FindNodeIndexPart(prim->GetName());

		auto handle = primHandle.FindPart(pattern);

		if (handle.valid())
		{
			ref->SetTable(handle);
			return true;
		}

		auto componentCount = (int)prim->GetRows();

		uint8_t mask = 0;
		size_t shift = 0;
		for (auto& c : pattern)
		{
			char n = NormalizeSwizzleChar(c);
			int i = SwizzleCharToIndex(n);
			if (i < 0 || i >= componentCount)
			{
				return false;
			}
			mask = (i & 0x3) | (mask << shift);
			shift += 2;
		}

		std::string typeName = ToString(prim->GetKind());
		if (pattern.size() > 1)
		{
			typeName += std::to_string(pattern.size());
		}

		auto context = ASTContext::GetCurrentContext();
		auto& idTable = context->GetIdentifierTable();

		auto typeN = primitives.GetSymbolTable();
		auto primitivesTable = primitives.GetSymbolTable();
		auto primitiveHandle = primitivesTable->FindNodeIndexPart(typeName);
		auto& resultingType = primitiveHandle.GetMetadata()->declaration;

		auto symbolRef = SymbolRef::Create(TextSpan(), idTable.Get(typeName), SymbolRefType_Member, false);
		symbolRef->SetTable(primitiveHandle);
		auto swizzleDef = SwizzleDefinition::Create(TextSpan(), idTable.Get(pattern), mask, prim, symbolRef);
		auto metaField = SharedPtr<SymbolMetadata>::Create(swizzleDef);

		if (primHandle.invalid())
		{
			auto meta = SharedPtr<SymbolMetadata>();
			primHandle = swizzleTable->Insert(prim->GetName(), meta, 0);
		}

		handle = swizzleTable->Insert(pattern, metaField, primHandle);
		ref->SetTable(handle);

		return true;
	}
}