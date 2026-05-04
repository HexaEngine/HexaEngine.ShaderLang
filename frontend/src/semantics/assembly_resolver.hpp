#ifndef ASSEMBLY_RESOLVER_HPP
#define ASSEMBLY_RESOLVER_HPP

namespace HXSL
{
	class AssemblyResolver
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
	};
}

#endif