#include "assembly_loader.hpp"
#include "assembly_builder.hpp"
#include <core/module_reader.hpp>

namespace HXSL
{
	using AssemblyHeader = AssemblyFormat::AssemblyHeader;
	using RecordId = Backend::RecordId;
	using ModuleReader = Backend::ModuleReader;
	using ModuleLinker = Backend::ModuleLinker;

	Assembly* AssemblyLoadContext::LoadFromStream(uptr<Stream>&& stream)
	{
		return nullptr;
	}

	Assembly* AssemblyLoadContext::LoadFromFile(const StringSpan& path)
	{
		{
			std::shared_lock shared(mutex);
			auto it = nameToAssembly.find(path);
			if (it != nameToAssembly.end())
			{
				return it->second;
			}
		}

		std::unique_lock lock(mutex);
		auto it = nameToAssembly.find(path);
		if (it != nameToAssembly.end())
		{
			return it->second;
		}

		auto fs = FileStream::OpenRead(path.data());
		if (!fs)
		{
			throw std::runtime_error("");
		}
		AssemblyReader reader = { *this, fs };
		uptr<Assembly> ass;
		reader.Read(path, ass);
		assemblies.push_back(std::move(ass));
		auto pAss = assemblies.back().get();
		nameToAssembly.insert({ pAss->GetName(), pAss });
		return pAss;
	}

	AssemblyCollection AssemblyLoadContext::BuildCollection()
	{
		AssemblyCollection collection;
		for (auto& ass : assemblies)
		{
			collection.AddAssembly(ass.get());
		}
		return collection;
	}

	AssemblyLoadResult AssemblyReader::Read(const StringSpan& pathOrName, uptr<Assembly>& assembly)
	{
		assembly = {};
		AssemblyHeader header;
		if (!header.Read(stream.Get())) return AssemblyLoadResult::ParseError;

		AssemblyBuilder builder = AssemblyBuilder(pathOrName);
		builder.SetArchitecture(header.architecture);
		builder.SetLanguageId(header.languageIdentifier);
		builder.SetEntryPoint(header.entryPoint);

		auto& resolver = ctx.GetResolver();
		auto referenceCount = stream->ReadLEB128<uint32_t>();
		for (uint32_t i = 0; i < referenceCount; ++i)
		{
			AssemblyReference reference;
			reference.name = stream->ReadString();
			builder.AddReference(reference);
			resolver.Resolve(reference);
		}

		ModuleLinker& linker = ctx.GetLinker();
		builder.SetModule(Backend::Module::OpenRead(stream, linker));

		assembly = builder.Build();

		return AssemblyLoadResult::Success;
	}
}