#include "module_builder.hpp"
#include "il/il_context.hpp"
#include "il_builder.hpp"

namespace HXSL
{
	using namespace Backend;

	std::unique_ptr<Module> ModuleBuilder::Convert(CompilationUnit* compilation)
	{
		module = std::make_unique<Module>();
		for (auto& ns : compilation->GetNamespaces())
		{
			module->AddNamespace(ConvertNamespace(ns.get()));
		}

		module->SetAllFunctions(module->GetAllocator().CopySpan(functions));

		return std::move(module);
	}

	Backend::PointerLayout* ModuleBuilder::MakePointerType(Backend::TypeLayout* elementType)
	{
		auto it = pointerMap.find(elementType);
		if (it != pointerMap.end())
		{
			return it->second;
		}

		std::string name = elementType->GetName().str() + "*";
		PointerLayoutBuilder builder = PointerLayoutBuilder(*module.get());
		builder
			.Name(name)
			.Access(AccessModifier_Public)
			.ElementType(elementType);

		auto res = builder.Build();
		pointerMap.insert({ elementType, res });
		return res;
	}

	Backend::PointerLayout* ModuleBuilder::MakePointerType(SymbolDef* elementType)
	{
		return MakePointerType(ConvertType(elementType));
	}

	TypeLayout* ModuleBuilder::ConvertType(SymbolDef* type)
	{
		auto it = map.find(type);
		if (it != map.end())
		{
			return cast<TypeLayout>(it->second);
		}

		if (auto prim = dyn_cast<Primitive>(type))
		{
			return ConvertPrimitive(prim);
		}

		if (auto strct = dyn_cast<Struct>(type))
		{
			return ConvertStruct(strct);
		}

		if (auto clss = dyn_cast<Class>(type))
		{
			return ConvertClass(clss);
		}

		HXSL_ASSERT(false, "Unhandled SymbolDef type, this should never happen.");
		return nullptr;
	}

	PrimitiveLayout* ModuleBuilder::ConvertPrimitive(Primitive* prim)
	{
		auto it = map.find(prim);
		if (it != map.end())
		{
			return cast<PrimitiveLayout>(it->second);
		}

		PrimitiveLayoutBuilder builder = PrimitiveLayoutBuilder(*module.get());
		builder
			.Name(prim->GetName())
			.Access(AccessModifier_Public)
			.Kind(prim->GetKind())
			.Class(prim->GetClass())
			.Rows(prim->GetRows())
			.Columns(prim->GetColumns());
		auto res = builder.Build();
		map.insert({ prim, res });
		return res;
	}

	FieldLayout* ModuleBuilder::ConvertField(Field* field)
	{
		auto it = map.find(field);
		if (it != map.end())
		{
			return cast<FieldLayout>(it->second);
		}

		FieldLayoutBuilder builder = FieldLayoutBuilder(*module.get());
		builder
			.Name(field->GetName())
			.Semantic(field->GetSemantic()->name)
			.Access(field->GetAccessModifiers())
			.InterpolationModifier(field->GetInterpolationModifiers())
			.StorageClass(field->GetStorageClass())
			.Type(ConvertType(field->GetSymbolRef()->GetDeclaration()));

		auto res = builder.Build();
		map.insert({ field, res });
		return res;
	}

	ParameterLayout* ModuleBuilder::ConvertParameter(Parameter* param)
	{
		auto it = map.find(param);
		if (it != map.end())
		{
			return cast<ParameterLayout>(it->second);
		}

		ParameterLayoutBuilder builder = ParameterLayoutBuilder(*module.get());
		builder
			.Name(param->GetName())
			.Semantic(param->GetSemantic()->name)
			.InterpolationModifier(param->GetInterpolationModifiers())
			.StorageClass(StorageClass_None)
			.ParameterFlags(param->GetParameterFlags())
			.Type(ConvertType(param->GetDeclaredType()));

		auto res = builder.Build();
		map.insert({ param, res });
		return res;
	}

	void ModuleBuilder::ConvertFunction(FunctionOverload* func, FunctionLayoutBuilder& builder)
	{
		builder
			.Name(func->GetName())
			.Access(func->GetAccessModifiers())
			.StorageClass(StorageClass_None)
			.FunctionFlags(func->GetFunctionFlags())
			.ReturnType(ConvertType(func->GetReturnType()));

		for (auto& param : func->GetParameters())
		{
			builder.AddParameter(ConvertParameter(param.get()));
		}

		functions.push_back(builder.Peek());

		ILContext* context = module->GetAllocator().Alloc<ILContext>(module.get(), builder.Peek());

		auto& allocator = context->GetAllocator();

		ILContainer container = { allocator };
		JumpTable jumpTable = {};

		ILMetadataBuilder metaBuilder = ILMetadataBuilder(*this, context->GetMetadata());
		ILBuilder ilBuilder = ILBuilder(*this, context, metaBuilder, container, jumpTable);
		ilBuilder.Build(func);
		jumpTable.Prepare();
		ilBuilder.Print();

		auto& cfg = context->GetCFG();
		cfg.Build(container, jumpTable);
		cfg.Print();

		builder.Context(context);
	}

	Backend::FunctionLayout* ModuleBuilder::ConvertFunction(FunctionOverload* func)
	{
		auto it = map.find(func);
		if (it != map.end())
		{
			return cast<FunctionLayout>(it->second);
		}

		FunctionLayoutBuilder builder = FunctionLayoutBuilder(*module.get());
		map.insert({ func, builder.Peek() });
		ConvertFunction(func, builder);
		return builder.Build();
	}

	Backend::OperatorLayout* ModuleBuilder::ConvertOperator(OperatorOverload* op)
	{
		auto it = map.find(op);
		if (it != map.end())
		{
			return cast<OperatorLayout>(it->second);
		}

		OperatorLayoutBuilder builder = OperatorLayoutBuilder(*module.get());
		map.insert({ op, builder.Peek() });
		ConvertFunction(op, builder);
		builder
			.Operator(op->GetOperator())
			.OperatorFlags(op->GetOperatorFlags());
		return builder.Build();
	}

	Backend::ConstructorLayout* ModuleBuilder::ConvertConstructor(ConstructorOverload* ctor)
	{
		auto it = map.find(ctor);
		if (it != map.end())
		{
			return cast<ConstructorLayout>(it->second);
		}

		ConstructorLayoutBuilder builder = ConstructorLayoutBuilder(*module.get());
		map.insert({ ctor, builder.Peek() });
		ConvertFunction(ctor, builder);
		return builder.Build();
	}

	StructLayout* ModuleBuilder::ConvertStruct(Struct* strct)
	{
		auto it = map.find(strct);
		if (it != map.end())
		{
			return cast<StructLayout>(it->second);
		}

		StructLayoutBuilder builder = StructLayoutBuilder(*module.get());
		ConvertType(strct, builder);
		builder
			.StructFlags(StructLayoutFlags_None);
		auto res = builder.Build();
		map.insert({ strct, res });
		return res;
	}

	Backend::StructLayout* ModuleBuilder::ConvertClass(Class* clss)
	{
		auto it = map.find(clss);
		if (it != map.end())
		{
			return cast<StructLayout>(it->second);
		}

		StructLayoutBuilder builder = StructLayoutBuilder(*module.get());
		ConvertType(clss, builder);
		builder
			.StructFlags(StructLayoutFlags_Class);
		auto res = builder.Build();
		map.insert({ clss, res });
		return res;
	}

	Backend::NamespaceLayout* ModuleBuilder::ConvertNamespace(Namespace* ns)
	{
		auto it = map.find(ns);
		if (it != map.end())
		{
			return cast<NamespaceLayout>(it->second);
		}

		NamespaceLayoutBuilder builder = NamespaceLayoutBuilder(*module.get());
		builder.Name(ns->GetName());

		for (auto& strct : ns->GetStructs())
		{
			builder.AddStruct(ConvertStruct(strct.get()));
		}

		for (auto& clss : ns->GetClasses())
		{
			builder.AddStruct(ConvertClass(clss.get()));
		}

		for (auto& func : ns->GetFunctions())
		{
			builder.AddFunction(ConvertFunction(func.get()));
		}

		for (auto& field : ns->GetFields())
		{
			builder.AddGlobalField(ConvertField(field.get()));
		}

		for (auto& nestedNs : ns->GetNestedNamespaces())
		{
			builder.AddNestedNamespace(ConvertNamespace(nestedNs.get()));
		}

		auto res = builder.Build();
		map.insert({ ns, res });
		return res;
	}
}