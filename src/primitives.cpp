#include "primitives.hpp"
#include "ast.hpp"
namespace HXSL
{
	std::once_flag PrimitiveManager::initFlag;

	Primitive* PrimitiveManager::GetPrimitiveType(const TextSpan& name) const
	{
		auto it = PrimitiveTypesCache.find(name);
		if (it != PrimitiveTypesCache.end())
		{
			return it->second.get();
		}
		return nullptr;
	}

	void PrimitiveManager::Populate()
	{
		AddPrim(PrimitiveKind_Void, PrimitiveClass_Scalar, 1, 1);

		for (int i = PrimitiveKind_Bool; i <= PrimitiveKind_Min16Uint; i++)
		{
			auto kind = static_cast<PrimitiveKind>(i);

			AddPrim(kind, PrimitiveClass_Scalar, 1, 1);

			for (uint n = 2; n <= 4; ++n)
			{
				AddPrim(kind, PrimitiveClass_Vector, n, 1);
			}

			for (uint r = 1; r <= 4; ++r)
			{
				for (uint c = 1; c <= 4; ++c)
				{
					AddPrim(kind, PrimitiveClass_Matrix, r, c);
				}
			}
		}

		Class* _class;
		size_t index;
		AddPrimClass(TextSpan("Texture2D"), &_class, &index);

		auto func = std::make_unique<Function>();
		func->SetName(TextSpan(PrimitiveSymbolTable->GetStringPool().add("Sample")));

		auto ret = std::make_unique<SymbolRef>(TextSpan(PrimitiveSymbolTable->GetStringPool().add("float4")), SymbolRefType_AnyType);
		ResolveInternal(ret.get());
		func->SetReturnSymbol(std::move(ret));

		auto meta = std::make_shared<SymbolMetadata>(SymbolType_Function, SymbolScopeType_Class, AccessModifier_Public, 0, func.get());
		PrimitiveSymbolTable->Insert(TextSpan(func->GetName()), meta, index);
		_class->AddFunction(std::move(func));

		AddPrimClass(TextSpan("SamplerState"));
	}

	void PrimitiveManager::AddPrimClass(TextSpan name, Class** outClass, size_t* indexOut)
	{
		auto _class = std::make_unique<Class>();
		_class->SetAccessModifiers(AccessModifier_Public);
		_class->SetName(PrimitiveSymbolTable->GetStringPool().add(name.toString()));

		auto meta = std::make_shared<SymbolMetadata>(SymbolType_Class, SymbolScopeType_Global, AccessModifier_Public, 0, _class.get());
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

	void PrimitiveManager::AddPrim(PrimitiveKind kind, PrimitiveClass primitiveClass, uint rows, uint columns)
	{
		std::ostringstream name;

		switch (kind)
		{
		case PrimitiveKind_Void:
			name << "void";
			break;
		case PrimitiveKind_Bool:
			name << "bool";
			break;
		case PrimitiveKind_Int:
			name << "int";
			break;
		case PrimitiveKind_Float:
			name << "float";
			break;
		case PrimitiveKind_Uint:
			name << "uint";
			break;
		case PrimitiveKind_Double:
			name << "double";
			break;
		case PrimitiveKind_Min8Float:
			name << "min8float";
			break;
		case PrimitiveKind_Min10Float:
			name << "min10float";
			break;
		case PrimitiveKind_Min16Float:
			name << "min16float";
			break;
		case PrimitiveKind_Min12Int:
			name << "min12int";
			break;
		case PrimitiveKind_Min16Int:
			name << "min16int";
			break;
		case PrimitiveKind_Min16Uint:
			name << "min16uint";
			break;
		}

		switch (primitiveClass)
		{
		case PrimitiveClass_Scalar:
			break;

		case PrimitiveClass_Vector:
			name << rows;
			break;

		case PrimitiveClass_Matrix:
			name << rows << "x" << columns;
			break;
		}

		std::string nameStr = name.str();

		auto type = std::make_unique<Primitive>(kind, primitiveClass, nameStr, rows, columns);
		auto meta = std::make_shared<SymbolMetadata>(SymbolType_Primitive, SymbolScopeType_Global, AccessModifier_Public, 0, type.get());
		auto index = PrimitiveSymbolTable->Insert(TextSpan(type->GetName()), meta);

		PrimitiveTypesCache[type->GetName()] = std::move(type);
	}

	void PrimitiveManager::ResolveInternal(SymbolRef* ref)
	{
		auto index = PrimitiveSymbolTable->FindNodeIndexPart(ref->GetSpan());
		if (index == -1)
		{
			HXSL_ASSERT(false, "Couldn't find primitive.");
		}
		ref->SetTable(PrimitiveSymbolTable.get(), index);
	}
}