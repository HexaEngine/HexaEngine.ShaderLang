#ifndef ASSEMBLY_RESOLVER_HPP
#define ASSEMBLY_RESOLVER_HPP

#include "core/module_linker.hpp"

namespace HXSL
{
	class AssemblyLoadContext;
	class AssemblyResolver : public Backend::IModuleProvider
	{
		std::vector<std::string> searchPaths;
		AssemblyLoadContext& context;

		Assembly* ResolveInner(const AssemblyReference& name);

	public:
		AssemblyResolver(AssemblyLoadContext& context);

		void AddSearchPath(const std::string& path);
		Assembly* Resolve(const AssemblyReference& name);
		Backend::Module* LoadModule(const Backend::ModuleReference& ref) override;
	};
}

#endif