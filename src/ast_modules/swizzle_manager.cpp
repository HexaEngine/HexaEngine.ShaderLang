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

		auto handle = swizzleTable->FindNodeIndexPart(pattern);

		if (handle.valid())
		{
			ref->SetTable(handle);
			return true;
		}

		auto componentCount = (int)prim->GetRows();

		for (auto& c : pattern)
		{
			char n = NormalizeSwizzleChar(c);
			int i = SwizzleCharToIndex(n);
			if (i < 0 || i >= componentCount)
			{
				return false;
			}
		}

		std::string typeName = ToString(prim->GetKind());
		if (pattern.size() > 1)
		{
			typeName += std::to_string(pattern.size());
		}

		auto primitivesTable = primitives.GetSymbolTable();
		auto primitiveHandle = primitivesTable->FindNodeIndexPart(typeName);
		auto& resultingType = primitiveHandle.GetMetadata()->declaration;

		auto symbolRef = std::make_unique<SymbolRef>(TextSpan(), SymbolRefType_Member, false);
		symbolRef->SetTable(primitiveHandle);
		auto swizzleDef = std::make_unique<SwizzleDefinition>(resultingType->GetSpan(), std::move(symbolRef));
		auto metaField = std::make_shared<SymbolMetadata>(SymbolType_Field, SymbolScopeType_Struct, AccessModifier_Public, 0, swizzleDef.get());

		handle = swizzleTable->Insert(pattern, metaField, 0);
		ref->SetTable(handle);

		definitions.push_back(std::move(swizzleDef));

		return true;
	}
}