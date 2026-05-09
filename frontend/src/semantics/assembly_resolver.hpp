#ifndef ASSEMBLY_RESOLVER_HPP
#define ASSEMBLY_RESOLVER_HPP

#include "core/module_linker.hpp"

namespace HXSL
{
	class AssemblyResolver : public Backend::IModuleProvider
	{
		dense_map<StringSpan, uptr<Assembly>> assemblies;
		StringPool2 pool;
		std::vector<std::string> searchPaths;

		Assembly* ResolveInner(const AssemblyReference& name);

	public:
		AssemblyResolver();

		void AddSearchPath(const std::string& path);
		void AddAssembly(std::unique_ptr<Assembly>&& assembly);
		Assembly* Resolve(const AssemblyReference& name);
		AssemblyCollection BuildCollection();
		Backend::Module* LoadModule(const Backend::ModuleReference& ref) override;
	};
}

#endif