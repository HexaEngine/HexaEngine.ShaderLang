#ifndef INSTANTIATOR_HPP
#define INSTANTIATOR_HPP

#include "ast.hpp"

namespace HXSL
{
	static std::unique_ptr<SymbolDef> CreateInstance(NodeType type)
	{
		switch (type)
		{
		case NodeType_Namespace:
			return std::make_unique<Namespace>();
		case NodeType_Field:
			return std::make_unique<Field>();
		case NodeType_FunctionOverload:
			return std::make_unique<FunctionOverload>();
		case NodeType_OperatorOverload:
			return std::make_unique<OperatorOverload>();
		case NodeType_Struct:
			return std::make_unique<Struct>();
		case NodeType_Parameter:
			return std::make_unique<Parameter>();
		case NodeType_DeclarationStatement:
			return std::make_unique<DeclarationStatement>();
		default:
			return nullptr;
		}
	}
}

#endif