#ifndef ASSEMBLY_BUILDER_HPP
#define ASSEMBLY_BUILDER_HPP

#include "assembly.hpp"

namespace HXSL
{
	class AssemblyBuilder
	{
		using RecordId = Backend::RecordId;
		using Module = Backend::Module;

		uptr<Assembly> assembly;
		Architecture architecture = Architecture::Any;
		LanguageIdentifier languageId = LanguageIdentifier::Unknown;
		RecordId entryPoint;
		std::vector<AssemblyReference> referencedAssemblies;

	public:
		explicit AssemblyBuilder(const StringSpan& name) : assembly(Assembly::Create(name)) {}

		Assembly* Peek() { return assembly.get(); }

		AssemblyBuilder& SetArchitecture(Architecture arch)
		{
			assembly->architecture = architecture;
			return *this;
		}

		AssemblyBuilder& SetLanguageId(LanguageIdentifier id)
		{
			assembly->languageId = languageId;
			return *this;
		}

		AssemblyBuilder& SetModule(uptr<Module>&& module)
		{
			assembly->module = std::move(module);
			return *this;
		}

		AssemblyBuilder& SetEntryPoint(RecordId id)
		{
			assembly->entryPoint = entryPoint;
			return *this;
		}

		AssemblyBuilder& AddReference(const AssemblyReference& ref)
		{
			assembly->referencedAssemblies.push_back(ref);
			return *this;
		}

		uptr<Assembly> Build()
		{
			return std::move(assembly);
		}
	};
}
#endif