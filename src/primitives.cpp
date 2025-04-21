#include "primitives.hpp"
#include "nodes.hpp"
namespace HXSL
{
	std::once_flag HXSLPrimitiveManager::initFlag;

	HXSLPrimitive* HXSLPrimitiveManager::GetPrimitiveType(const TextSpan& name) const
	{
		auto it = PrimitiveTypesCache.find(name);
		if (it != PrimitiveTypesCache.end())
		{
			return it->second.get();
		}
		return nullptr;
	}

	void HXSLPrimitiveManager::Populate()
	{
		AddPrim(HXSLPrimitiveKind_Void, HXSLPrimitiveClass_Scalar, 1, 1);

		for (int i = HXSLPrimitiveKind_Bool; i <= HXSLPrimitiveKind_Min16Uint; i++)
		{
			auto kind = static_cast<HXSLPrimitiveKind>(i);

			AddPrim(kind, HXSLPrimitiveClass_Scalar, 1, 1);

			for (uint n = 2; n <= 4; ++n)
			{
				AddPrim(kind, HXSLPrimitiveClass_Vector, n, 1);
			}

			for (uint r = 1; r <= 4; ++r)
			{
				for (uint c = 1; c <= 4; ++c)
				{
					AddPrim(kind, HXSLPrimitiveClass_Matrix, r, c);
				}
			}
		}

		HXSLClass* _class;
		size_t index;
		AddPrimClass(TextSpan("Texture2D"), &_class, &index);

		auto func = std::make_unique<HXSLFunction>();
		func->SetName(TextSpan(PrimitiveSymbolTable->GetStringPool().add("Sample")));

		auto ret = std::make_unique<HXSLSymbolRef>(TextSpan(PrimitiveSymbolTable->GetStringPool().add("float4")), HXSLSymbolRefType_AnyType);
		ResolveInternal(ret.get());
		func->SetReturnSymbol(std::move(ret));

		auto meta = std::make_shared<SymbolMetadata>(HXSLSymbolType_Function, HXSLSymbolScopeType_Class, HXSLAccessModifier_Public, 0, func.get());
		PrimitiveSymbolTable->Insert(TextSpan(func->GetName()), meta, index);
		_class->AddFunction(std::move(func));

		AddPrimClass(TextSpan("SamplerState"));
	}

	void HXSLPrimitiveManager::AddPrimClass(TextSpan name, HXSLClass** outClass, size_t* indexOut)
	{
		auto _class = std::make_unique<HXSLClass>();
		_class->SetAccessModifiers(HXSLAccessModifier_Public);
		_class->SetName(PrimitiveSymbolTable->GetStringPool().add(name.toString()));

		auto meta = std::make_shared<SymbolMetadata>(HXSLSymbolType_Class, HXSLSymbolScopeType_Global, HXSLAccessModifier_Public, 0, _class.get());
		auto index = PrimitiveSymbolTable->Insert(TextSpan(_class->GetName()), meta);
		if (outClass)
		{
			*outClass = _class.get();
		}
		if (indexOut)
		{
			*indexOut = index;
		}
		auto compilation = PrimitiveSymbolTable->GetCompilation();
		compilation->AddClass(std::move(_class));
	}

	void HXSLPrimitiveManager::AddPrim(HXSLPrimitiveKind kind, HXSLPrimitiveClass primitiveClass, uint rows, uint columns)
	{
		std::ostringstream name;

		switch (kind)
		{
		case HXSLPrimitiveKind_Void:
			name << "void";
			break;
		case HXSLPrimitiveKind_Bool:
			name << "bool";
			break;
		case HXSLPrimitiveKind_Int:
			name << "int";
			break;
		case HXSLPrimitiveKind_Float:
			name << "float";
			break;
		case HXSLPrimitiveKind_Uint:
			name << "uint";
			break;
		case HXSLPrimitiveKind_Double:
			name << "double";
			break;
		case HXSLPrimitiveKind_Min8Float:
			name << "min8float";
			break;
		case HXSLPrimitiveKind_Min10Float:
			name << "min10float";
			break;
		case HXSLPrimitiveKind_Min16Float:
			name << "min16float";
			break;
		case HXSLPrimitiveKind_Min12Int:
			name << "min12int";
			break;
		case HXSLPrimitiveKind_Min16Int:
			name << "min16int";
			break;
		case HXSLPrimitiveKind_Min16Uint:
			name << "min16uint";
			break;
		}

		switch (primitiveClass)
		{
		case HXSLPrimitiveClass_Scalar:
			break;

		case HXSLPrimitiveClass_Vector:
			name << rows;
			break;

		case HXSLPrimitiveClass_Matrix:
			name << rows << "x" << columns;
			break;
		}

		std::string nameStr = name.str();

		auto type = std::make_unique<HXSLPrimitive>(kind, primitiveClass, nameStr, rows, columns);
		auto meta = std::make_shared<SymbolMetadata>(HXSLSymbolType_Primitive, HXSLSymbolScopeType_Global, HXSLAccessModifier_Public, 0, type.get());
		auto index = PrimitiveSymbolTable->Insert(TextSpan(type->GetName()), meta);

		PrimitiveTypesCache[type->GetName()] = std::move(type);
	}

	void HXSLPrimitiveManager::ResolveInternal(HXSLSymbolRef* ref)
	{
		auto index = PrimitiveSymbolTable->FindNodeIndexPart(ref->GetSpan());
		if (index == -1)
		{
			HXSL_ASSERT(false, "Couldn't find primitive.");
		}
		ref->SetTable(PrimitiveSymbolTable.get(), index);
	}
}