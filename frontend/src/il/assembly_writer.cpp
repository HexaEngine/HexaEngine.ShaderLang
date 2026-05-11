#include "assembly_writer.hpp"
#include <core/module_writer.hpp>

namespace HXSL
{
	using AssemblyHeader = AssemblyFormat::AssemblyHeader;
	using RecordId = Backend::RecordId;
	using ModuleWriter = Backend::ModuleWriter;

	void AssemblyWriter::Write(Assembly* assembly)
	{
		auto langId = assembly->GetLanguageIdentifier();
		auto arch = assembly->GetArchitecture();
		auto entryPointId = assembly->GetEntryPointId();
		AssemblyHeader header = { langId, arch, entryPointId };
		header.Write(stream.Get());

		auto references = assembly->GetReferencedAssemblies();
		stream->WriteLittleEndian(static_cast<uint32_t>(references.size()));
		for (auto& ref : references)
		{
			stream->WriteString(ref.name);
		}

		auto module = assembly->GetModule();
		ModuleWriter writer = { stream };
		writer.Write(module);
	}
}