#ifndef INSTANTIATOR_HPP
#define INSTANTIATOR_HPP

#include "pch/ast.hpp"

namespace HXSL
{
	static ast_ptr<SymbolDef> CreateInstance(NodeType type)
	{
		switch (type)
		{
		case NodeType_Namespace:
			return make_ast_ptr<Namespace>();
		case NodeType_Field:
			return make_ast_ptr<Field>();
		case NodeType_FunctionOverload:
			return make_ast_ptr<FunctionOverload>();
		case NodeType_OperatorOverload:
			return make_ast_ptr<OperatorOverload>();
		case NodeType_Struct:
			return make_ast_ptr<Struct>();
		case NodeType_Parameter:
			return make_ast_ptr<Parameter>();
		case NodeType_DeclarationStatement:
			return make_ast_ptr<DeclarationStatement>();
		default:
			return nullptr;
		}
	}
}

#endif