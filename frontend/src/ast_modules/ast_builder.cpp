#include "ast_builder.hpp"
#include "pch/ast.hpp"

namespace HXSL
{
	void DeclContainerBuilder::AddDeclaration(ASTNode* decl)
	{
		switch (decl->GetType())
		{
		case NodeType_Field:
			Append<Field, DeclContainerFlags::AllowFields>(decl, fields, FIELD_DECL_OUT_OF_SCOPE);
			break;
		case NodeType_Struct:
			Append<Struct, DeclContainerFlags::AllowStructs>(decl, structs, STRUCT_DECL_OUT_OF_SCOPE);
			break;
		case NodeType_Class:
			Append<Class, DeclContainerFlags::AllowClasses>(decl, classes, CLASS_DECL_OUT_OF_SCOPE);
			break;
		case NodeType_ConstructorOverload:
			Append<ConstructorOverload, DeclContainerFlags::AllowConstructors>(decl, constructors, CTOR_DECL_OUT_OF_SCOPE);
			break;
		case NodeType_FunctionOverload:
			Append<FunctionOverload, DeclContainerFlags::AllowFunctions>(decl, functions, FUNC_DECL_OUT_OF_SCOPE);
			break;
		case NodeType_OperatorOverload:
			Append<OperatorOverload, DeclContainerFlags::AllowOperators>(decl, operators, OP_DECL_OUT_OF_SCOPE);
			break;
		case NodeType_Namespace:
			Append<Namespace, DeclContainerFlags::AllowNamespaces>(decl, namespaces, NAMESPACE_DECL_OUT_OF_SCOPE);
			break;
		case NodeType_UsingDecl:
			Append<UsingDecl, DeclContainerFlags::AllowUsings>(decl, usings, USING_DECL_OUT_OF_SCOPE);
			break;
		default:
			HXSL_ASSERT(false, "Unhandled declaration type in DeclContainerBuilder.");
			break;
		}
	}
}