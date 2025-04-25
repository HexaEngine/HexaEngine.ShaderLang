#include "primitive_manager.hpp"
#include "node_builder.hpp"

namespace HXSL
{
	std::once_flag PrimitiveManager::initFlag;

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

		auto table = GetMutableSymbolTable();

		Class* _class;

		AddPrimClass(TextSpan("SamplerState"));

		AddPrimClass(TextSpan("Texture2D"), &_class);

		FunctionBuilder(assembly.get())
			.WithName("Sample")
			.WithParam("state", "SamplerState")
			.WithParam("uv", "float2")
			.Returns("float4")
			.AttachToClass(_class);
	}

	void PrimitiveManager::AddPrimClass(TextSpan name, Class** outClass, SymbolHandle* handleOut)
	{
		auto table = GetMutableSymbolTable();
		auto compilation = table->GetCompilation();
		auto _class = std::make_unique<Class>();

		auto classPtr = _class.get();
		compilation->primitiveClasses.push_back(std::move(_class));

		classPtr->SetAccessModifiers(AccessModifier_Public);
		classPtr->SetName(table->GetStringPool().add(name.toString()));

		auto meta = std::make_shared<SymbolMetadata>(SymbolType_Class, SymbolScopeType_Global, AccessModifier_Public, 0, classPtr);
		auto handle = table->Insert(TextSpan(classPtr->GetName()), meta);
		if (outClass)
		{
			*outClass = classPtr;
		}
		if (handleOut)
		{
			*handleOut = handle;
		}

		classPtr->SetAssembly(assembly.get(), handle);
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

		PrimitiveBuilder builder(assembly.get());
		builder.WithName(nameStr);
		builder.WithKind(kind, primitiveClass);
		builder.WithRowsAndColumns(rows, columns);

		if (kind != PrimitiveKind_Void)
		{
			builder.WithBinaryOperators({ Operator_Add, Operator_Subtract, Operator_Multiply, Operator_Divide }, nameStr);
			builder.WithUnaryOperators({ Operator_LogicalNot, Operator_Increment, Operator_Decrement }, nameStr);
		}

		builder.Finish();
	}

	void PrimitiveManager::ResolveInternal(SymbolRef* ref)
	{
		auto table = GetMutableSymbolTable();
		auto handle = table->FindNodeIndexPart(ref->GetName());
		if (handle.invalid())
		{
			HXSL_ASSERT(false, "Couldn't find primitive.");
		}
		ref->SetTable(handle);
	}
}