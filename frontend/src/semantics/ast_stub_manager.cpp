#include "ast_stub_manager.hpp"
#include "middleware/module_decompiler.hpp"

namespace HXSL
{
	StubModule* ASTStubManager::GetStub(Assembly* assembly)
	{
		auto it = stubMap.find(assembly);
		if (it != stubMap.end())
		{
			return it->second;
		}
		return nullptr;
	}

	StubModule* ASTStubManager::AddStub(Assembly* assembly)
	{
		ModuleDecompilerOptions options;
		options.markAsExtern = true;
		ModuleDecompiler decompiler(options);
		auto stub = decompiler.Deconvert(assembly->GetModule());
		auto pStub = stub.get();
		stubs.push_back(std::move(stub));
		stubMap.insert({ assembly, pStub });
		return pStub;
	}
}
