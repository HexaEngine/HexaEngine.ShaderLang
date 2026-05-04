#ifndef MODULE_STUB_HPP
#define MODULE_STUB_HPP

#include "pch/ast_analyzers.hpp"
#include "core/module.hpp"

namespace HXSL
{
	struct StubModule
	{
		const Backend::Module* module;
		CompilationUnit* unit;
		dense_map<ASTNode*, Backend::Layout*> reverseMap;
	};
}

#endif