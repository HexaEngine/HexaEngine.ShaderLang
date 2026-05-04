#include "module_decompiler.hpp"

namespace HXSL
{
	using namespace Backend;

	SymbolRef* ModuleDecompiler::MakeTypeRef(TypeLayout* type)
	{
		auto* ident = ASTContext::GetCurrentContext()->GetIdentifier(type->GetName());
		return SymbolRef::Create({}, ident, SymbolRefType_Type, false);
	}

	SymbolDef* ModuleDecompiler::DeconvertType(TypeLayout* type)
	{
		auto it = map.find(type);
		if (it != map.end())
		{
			return it->second;
		}

		if (auto* prim = dyn_cast<PrimitiveLayout>(type))
		{
			return DeconvertPrimitive(prim);
		}

		if (auto* strct = dyn_cast<StructLayout>(type))
		{
			if (strct->GetStructFlags() == StructLayoutFlags_Class)
			{
				return DeconvertClass(strct);
			}
			return DeconvertStruct(strct);
		}

		if (auto* enm = dyn_cast<EnumLayout>(type))
		{
			return DeconvertEnum(enm);
		}

		return nullptr;
	}

	Primitive* ModuleDecompiler::DeconvertPrimitive(PrimitiveLayout* prim)
	{
		auto it = map.find(prim);
		if (it != map.end())
		{
			return static_cast<Primitive*>(it->second);
		}

		auto* name = ASTContext::GetCurrentContext()->GetIdentifier(prim->GetName());
		auto* node = Primitive::Create({}, name, prim->GetKind(), prim->GetClass(),
			static_cast<uint16_t>(prim->GetRows()), static_cast<uint16_t>(prim->GetColumns()), 0u);
		Transform(prim, node);
		map.insert({ prim, node });
		return node;
	}

	Field* ModuleDecompiler::DeconvertField(FieldLayout* field)
	{
		auto it = map.find(field);
		if (it != map.end())
		{
			return static_cast<Field*>(it->second);
		}

		auto* ctx = ASTContext::GetCurrentContext();
		auto* name = ctx->GetIdentifier(field->GetName());
		IdentifierInfo* semantic = field->GetSemantic().empty() ? nullptr : ctx->GetIdentifier(field->GetSemantic());
		auto* typeRef = MakeTypeRef(field->GetType());
		auto* node = Field::Create({}, name, field->GetAccess(), field->GetStorageClass(),
			field->GetInterpolationModifier(), typeRef, semantic);
		Transform(field, node);
		map.insert({ field, node });
		return node;
	}

	Parameter* ModuleDecompiler::DeconvertParameter(ParameterLayout* param)
	{
		auto it = map.find(param);
		if (it != map.end())
		{
			return static_cast<Parameter*>(it->second);
		}

		auto* ctx = ASTContext::GetCurrentContext();
		auto* name = ctx->GetIdentifier(param->GetName());
		IdentifierInfo* semantic = param->GetSemantic().empty() ? nullptr : ctx->GetIdentifier(param->GetSemantic());
		auto* typeRef = MakeTypeRef(param->GetType());
		auto* node = Parameter::Create({}, name, param->GetParameterFlags(),
			param->GetInterpolationModifier(), typeRef, semantic);
		Transform(param, node);
		map.insert({ param, node });
		return node;
	}

	FunctionOverload* ModuleDecompiler::DeconvertFunction(FunctionLayout* func)
	{
		auto it = map.find(func);
		if (it != map.end())
		{
			return static_cast<FunctionOverload*>(it->second);
		}

		auto* name = ASTContext::GetCurrentContext()->GetIdentifier(func->GetName());
		auto* returnRef = MakeTypeRef(func->GetReturnType());

		std::vector<Parameter*> params;
		for (auto* p : func->GetParameters())
		{
			params.push_back(DeconvertParameter(p));
		}

		auto* node = FunctionOverload::Create({}, name, func->GetAccess(), func->GetFunctionFlags(),
			returnRef, nullptr, nullptr, params, {});
		Transform(func, node);
		map.insert({ func, node });
		return node;
	}

	OperatorOverload* ModuleDecompiler::DeconvertOperator(OperatorLayout* op)
	{
		auto it = map.find(op);
		if (it != map.end())
		{
			return static_cast<OperatorOverload*>(it->second);
		}

		auto* name = ASTContext::GetCurrentContext()->GetIdentifier(op->GetName());
		auto* returnRef = MakeTypeRef(op->GetReturnType());

		std::vector<Parameter*> params;
		for (auto* p : op->GetParameters())
		{
			params.push_back(DeconvertParameter(p));
		}

		auto* node = OperatorOverload::Create({}, name, op->GetAccess(), op->GetFunctionFlags(),
			op->GetOperatorFlags(), op->GetOperator(), returnRef, nullptr, params, {});
		Transform(op, node);
		map.insert({ op, node });
		return node;
	}

	ConstructorOverload* ModuleDecompiler::DeconvertConstructor(ConstructorLayout* ctor, StructLayout* parent)
	{
		auto it = map.find(ctor);
		if (it != map.end())
		{
			return static_cast<ConstructorOverload*>(it->second);
		}

		auto* name = ASTContext::GetCurrentContext()->GetIdentifier(ctor->GetName());
		auto* targetRef = MakeTypeRef(parent);

		std::vector<Parameter*> params;
		for (auto* p : ctor->GetParameters())
		{
			params.push_back(DeconvertParameter(p));
		}

		auto* node = ConstructorOverload::Create({}, name, ctor->GetAccess(), ctor->GetFunctionFlags(),
			targetRef, nullptr, params, {});
		Transform(ctor, node);
		map.insert({ ctor, node });
		return node;
	}

	void ModuleDecompiler::FillDataType(StructLayout* strct,
		std::vector<Field*>& fields,
		std::vector<Struct*>& structs,
		std::vector<Class*>& classes,
		std::vector<ConstructorOverload*>& ctors,
		std::vector<FunctionOverload*>& funcs,
		std::vector<OperatorOverload*>& ops)
	{
		for (auto* f : strct->GetFields())
		{
			fields.push_back(DeconvertField(f));
		}

		for (auto* nested : strct->GetStructs())
		{
			if (nested->GetStructFlags() == StructLayoutFlags_Class)
			{
				classes.push_back(DeconvertClass(nested));
			}
			else
			{
				structs.push_back(DeconvertStruct(nested));
			}
		}

		for (auto* ctor : strct->GetConstructors())
		{
			ctors.push_back(DeconvertConstructor(ctor, strct));
		}

		for (auto* func : strct->GetFunctions())
		{
			funcs.push_back(DeconvertFunction(func));
		}

		for (auto* op : strct->GetOperators())
		{
			ops.push_back(DeconvertOperator(op));
		}
	}

	void ModuleDecompiler::Transform(Layout* layout, ASTNode* node)
	{
		if (options.markAsExtern)
		{
			node->SetExtern(true);
		}

		reverseMap.insert({ node, layout });
	}

	Struct* ModuleDecompiler::DeconvertStruct(StructLayout* strct)
	{
		auto it = map.find(strct);
		if (it != map.end())
		{
			return static_cast<Struct*>(it->second);
		}

		auto* name = ASTContext::GetCurrentContext()->GetIdentifier(strct->GetName());

		std::vector<Field*> fields;
		std::vector<Struct*> structs;
		std::vector<Class*> classes;
		std::vector<ConstructorOverload*> ctors;
		std::vector<FunctionOverload*> funcs;
		std::vector<OperatorOverload*> ops;

		FillDataType(strct, fields, structs, classes, ctors, funcs, ops);

		auto* node = Struct::Create({}, name, strct->GetAccess(), fields, structs, classes, ctors, funcs, ops);
		Transform(strct, node);
		map.insert({ strct, node });
		return node;
	}

	Class* ModuleDecompiler::DeconvertClass(StructLayout* clss)
	{
		auto it = map.find(clss);
		if (it != map.end())
		{
			return static_cast<Class*>(it->second);
		}

		auto* name = ASTContext::GetCurrentContext()->GetIdentifier(clss->GetName());

		std::vector<Field*> fields;
		std::vector<Struct*> structs;
		std::vector<Class*> classes;
		std::vector<ConstructorOverload*> ctors;
		std::vector<FunctionOverload*> funcs;
		std::vector<OperatorOverload*> ops;

		FillDataType(clss, fields, structs, classes, ctors, funcs, ops);

		auto* node = Class::Create({}, name, clss->GetAccess(), fields, structs, classes, ctors, funcs, ops);
		Transform(clss, node);
		map.insert({ clss, node });
		return node;
	}

	Enum* ModuleDecompiler::DeconvertEnum(EnumLayout* enm)
	{
		auto it = map.find(enm);
		if (it != map.end())
		{
			return static_cast<Enum*>(it->second);
		}

		auto* ctx = ASTContext::GetCurrentContext();
		auto* name = ctx->GetIdentifier(enm->GetName());
		auto* baseRef = MakeTypeRef(enm->GetBaseType());

		std::vector<EnumItem*> items;
		for (auto* item : enm->GetItems())
		{
			auto* itemName = ctx->GetIdentifier(item->GetName());
			auto* itemNode = EnumItem::Create({}, itemName, nullptr);
			itemNode->SetComputedValue(item->GetValue());
			Transform(item, itemNode);
			items.push_back(itemNode);
		}

		auto* node = Enum::Create({}, name, enm->GetAccess(), baseRef, items);
		Transform(enm, node);
		map.insert({ enm, node });
		return node;
	}

	Namespace* ModuleDecompiler::DeconvertNamespace(NamespaceLayout* ns)
	{
		auto it = map.find(ns);
		if (it != map.end())
		{
			return static_cast<Namespace*>(it->second);
		}

		auto* name = ASTContext::GetCurrentContext()->GetIdentifier(ns->GetName());

		std::vector<Enum*> enums;
		std::vector<Struct*> structs;
		std::vector<Class*> classes;
		std::vector<FunctionOverload*> funcs;
		std::vector<Field*> fields;
		std::vector<Namespace*> nestedNs;

		for (auto* enm : ns->GetEnums())
		{
			enums.push_back(DeconvertEnum(enm));
		}

		for (auto* strct : ns->GetStructs())
		{
			if (strct->GetStructFlags() == StructLayoutFlags_Class)
			{
				classes.push_back(DeconvertClass(strct));
			}
			else
			{
				structs.push_back(DeconvertStruct(strct));
			}
		}

		for (auto* func : ns->GetFunctions())
		{
			funcs.push_back(DeconvertFunction(func));
		}

		for (auto* field : ns->GetGlobalFields())
		{
			fields.push_back(DeconvertField(field));
		}

		for (auto* nested : ns->GetNestedNamespaces())
		{
			nestedNs.push_back(DeconvertNamespace(nested));
		}

		auto* node = Namespace::Create({}, name, structs, classes, funcs, fields, enums, nestedNs, {});
		Transform(ns, node);
		map.insert({ ns, node });
		return node;
	}

	uptr<StubModule> ModuleDecompiler::Deconvert(Module* module)
	{
		std::vector<Namespace*> namespaces;
		for (auto* ns : module->GetNamespaces())
		{
			namespaces.push_back(DeconvertNamespace(ns));
		}

		auto* unit = CompilationUnit::Create(true, namespaces, {});
		Transform(module, unit);

		auto stub = make_uptr<StubModule>();
		stub->module = module;
		stub->unit = unit;
		stub->reverseMap = std::move(reverseMap);
		return stub;
	}
}
