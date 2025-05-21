#include "primitive_manager.hpp"
#include "node_builder.hpp"

namespace HXSL
{
	std::once_flag PrimitiveManager::initFlag;

	static void AddPrim(std::vector<std::unique_ptr<PrimitiveBuilder>>& primBuilders, Assembly* assembly, PrimitiveKind kind, PrimitiveClass primitiveClass, uint32_t rows, uint32_t columns)
	{
		std::string scalarName = ToString(kind);

		std::ostringstream name;
		name << scalarName;

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

		auto builder = std::make_unique<PrimitiveBuilder>(assembly);
		builder->WithName(nameStr);
		builder->WithKind(kind, primitiveClass);
		builder->WithRowsAndColumns(rows, columns);

		if (kind != PrimitiveKind_Void)
		{
			builder->WithBinaryOperators({ Operator_Add, Operator_Subtract, Operator_Multiply, Operator_Divide, Operator_Modulus }, nameStr, OperatorFlags_Intrinsic);
			builder->WithBinaryOperators({ Operator_Equal, Operator_NotEqual, Operator_GreaterThan, Operator_LessThan, Operator_GreaterThanOrEqual, Operator_LessThanOrEqual }, nameStr, nameStr, "bool", OperatorFlags_Intrinsic);
			builder->WithUnaryOperators({ Operator_LogicalNot, Operator_Increment, Operator_Decrement, Operator_Subtract }, nameStr, OperatorFlags_Intrinsic);
			if (kind == PrimitiveKind_Int && primitiveClass == PrimitiveClass_Scalar)
			{
				builder->WithImplicitCast(nameStr, "half", OperatorFlags_Intrinsic);
				builder->WithImplicitCast(nameStr, "float", OperatorFlags_Intrinsic);
				builder->WithImplicitCast(nameStr, "double", OperatorFlags_Intrinsic);
				builder->WithExplicitCast(nameStr, "uint", OperatorFlags_Explicit | OperatorFlags_Intrinsic);
				builder->WithBinaryOperators({ Operator_BitwiseShiftLeft, Operator_BitwiseShiftRight, Operator_BitwiseAnd, Operator_BitwiseOr, Operator_BitwiseXor,  Operator_BitwiseNot }, nameStr, OperatorFlags_Intrinsic);
			}
			if (kind == PrimitiveKind_UInt && primitiveClass == PrimitiveClass_Scalar)
			{
				builder->WithImplicitCast(nameStr, "half", OperatorFlags_Intrinsic);
				builder->WithImplicitCast(nameStr, "float", OperatorFlags_Intrinsic);
				builder->WithImplicitCast(nameStr, "double", OperatorFlags_Intrinsic);
				builder->WithExplicitCast(nameStr, "int", OperatorFlags_Explicit | OperatorFlags_Intrinsic);
				builder->WithBinaryOperators({ Operator_BitwiseShiftLeft, Operator_BitwiseShiftRight, Operator_BitwiseAnd, Operator_BitwiseOr, Operator_BitwiseXor,  Operator_BitwiseNot }, nameStr, OperatorFlags_Intrinsic);
			}
			if (kind == PrimitiveKind_Bool && primitiveClass == PrimitiveClass_Scalar)
			{
				builder->WithBinaryOperators({ Operator_AndAnd, Operator_OrOr, Operator_LogicalNot, Operator_BitwiseAnd, Operator_BitwiseOr, Operator_BitwiseXor }, nameStr, OperatorFlags_Intrinsic);
			}
			if (primitiveClass == PrimitiveClass_Vector)
			{
				builder->WithBinaryOperators({ Operator_Add, Operator_Subtract, Operator_Multiply, Operator_Divide, Operator_Modulus }, nameStr, scalarName, nameStr, OperatorFlags_Intrinsic);
				if (kind != PrimitiveKind_Int)
				{
					builder->WithBinaryOperators({ Operator_Add, Operator_Subtract, Operator_Multiply, Operator_Divide, Operator_Modulus }, nameStr, "int", nameStr, OperatorFlags_Intrinsic);
				}
			}
		}

		primBuilders.push_back(std::move(builder));
	}

	void PrimitiveManager::Populate()
	{
		std::vector<std::unique_ptr<PrimitiveBuilder>> primBuilders;

		AddPrim(primBuilders, assembly.get(), PrimitiveKind_Void, PrimitiveClass_Scalar, 1, 1);

		for (int i = PrimitiveKind_Bool; i <= PrimitiveKind_Min16UInt; i++)
		{
			auto kind = static_cast<PrimitiveKind>(i);

			AddPrim(primBuilders, assembly.get(), kind, PrimitiveClass_Scalar, 1, 1);

			for (uint32_t n = 2; n <= 4; ++n)
			{
				AddPrim(primBuilders, assembly.get(), kind, PrimitiveClass_Vector, n, 1);
			}

			for (uint32_t r = 1; r <= 4; ++r)
			{
				for (uint32_t c = 1; c <= 4; ++c)
				{
					AddPrim(primBuilders, assembly.get(), kind, PrimitiveClass_Matrix, r, c);
				}
			}
		}

		for (size_t phase = 0; phase < 2; phase++)
		{
			for (auto& builder : primBuilders)
			{
				builder->FinishPhased();
			}
		}
		primBuilders.clear();

		auto table = GetMutableSymbolTable();

		Class* _class;

		AddPrimClass("string");

		AddPrimClass("SamplerState");

		AddPrimClass("Texture2D", &_class);

		FunctionBuilder(assembly.get())
			.WithName("Sample")
			.WithParam("state", "SamplerState")
			.WithParam("uv", "float2")
			.Returns("float4")
			.AttachToClass(_class);
	}

	void PrimitiveManager::AddPrimClass(const std::string& name, Class** outClass, SymbolHandle* handleOut)
	{
		auto table = GetMutableSymbolTable();
		auto compilation = table->GetCompilation();
		auto _class = std::make_unique<Class>();

		auto classPtr = _class.get();
		compilation->primitiveClasses.push_back(std::move(_class));

		classPtr->SetAccessModifiers(AccessModifier_Public);
		classPtr->SetName(name);

		auto meta = std::make_shared<SymbolMetadata>(SymbolType_Class, SymbolScopeType_Global, AccessModifier_Public, 0, classPtr);
		auto handle = table->Insert(classPtr->GetName(), meta);
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