#ifndef AST_STUB_MANAGER
#define AST_STUB_MANAGER

#include "pch/ast_analyzers.hpp"
#include "middleware/module_stub.hpp"
namespace HXSL
{
	class ASTStubManager
	{
		dense_map<Assembly*, StubModule*> stubMap;
		std::vector<uptr<StubModule>> stubs;
	public:
		ASTStubManager() = default;
		StubModule* GetStub(Assembly* assembly);
		StubModule* AddStub(Assembly* assembly);
		Span<uptr<StubModule>> GetAllStubs() { return stubs; }

	};
}

#endif