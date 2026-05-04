#include "assembly.hpp"
#include "semantics/symbols/symbol_table.hpp"
#include "semantics/semantic_analyzer.hpp"

namespace HXSL
{
	static constexpr size_t cstrlen(const char* str)
	{
		size_t len = 0;
		while (str[len] != '\0')
		{
			++len;
		}
		return len;
	}

	static constexpr char const* magic = "HXSL";
	static constexpr size_t magicSize = cstrlen(magic);

	inline Assembly::Assembly(const std::string& name) : name(std::make_unique<std::string>(name)), table(std::make_unique<SymbolTable>()), module(make_uptr<Backend::Module>()), sealed(false)
	{
	}

	SymbolHandle Assembly::AddSymbol(const StringSpan& name, SymbolDef* def, const ObjPtr<SymbolMetadata>& metadata, SymbolTableNode* lookupIndex)
	{
		if (sealed)
		{
			throw std::logic_error("Cannot modify symbol table: Assembly is sealed.");
		}
		auto handle = table->Insert(name, metadata, lookupIndex);
		if (handle.valid())
		{
			def->SetAssembly(this, handle);
		}
		return handle;
	}

	SymbolHandle Assembly::AddSymbolScope(const StringSpan& span, const ObjPtr<SymbolMetadata>& metadata, SymbolTableNode* lookupIndex)
	{
		if (sealed)
		{
			throw std::logic_error("Cannot modify symbol table: Assembly is sealed.");
		}
		return table->Insert(span, metadata, lookupIndex);
	}

	std::unique_ptr<Assembly> Assembly::Create(const std::string& path)
	{
		return std::unique_ptr<Assembly>(new Assembly(path));
	}

	AssemblyLoadResult Assembly::LoadFromFile(const std::string& path, std::unique_ptr<Assembly>& assemblyOut)
	{
		FILE* file;
		auto error = fopen_s(&file, path.c_str(), "r");

		if (error != 0 || file == nullptr)
		{
			return AssemblyLoadResult_FileNotFound;
		}

		std::unique_ptr<FILE, decltype(&fclose)> filePtr(file, &fclose);

		FileStream fs(file);

		auto result = LoadFromStream(path, fs, assemblyOut);
		return result;
	}

	AssemblyLoadResult Assembly::LoadFromStream(const std::string& path, Stream& stream, std::unique_ptr<Assembly>& assemblyOut)
	{
		char buffer[magicSize];
		stream.Read(buffer, magicSize);

		if (memcmp(buffer, magic, magicSize) != 0)
		{
			return AssemblyLoadResult_ParseError;
		}

		auto assembly = Create(path);

		auto referenceCount = stream.ReadUInt();
		for (uint32_t i = 0; i < referenceCount; ++i)
		{
			AssemblyReference reference;
			reference.name = stream.ReadString();
			assembly->referencedAssemblies.push_back(std::move(reference));
		}

		Backend::ModuleReader reader(&stream);
		assembly->module = reader.Read();

		assembly->Seal();
		assemblyOut = std::move(assembly);
		return AssemblyLoadResult_Success;
	}

	int Assembly::WriteToFile(const std::string& path) const
	{
		FILE* file;
		auto error = fopen_s(&file, path.c_str(), "wb+");

		if (error != 0 || file == nullptr)
		{
			return -1;
		}

		std::unique_ptr<FILE, decltype(&fclose)> filePtr(file, &fclose);

		FileStream fs(file);
		return WriteToStream(fs);
	}

	int Assembly::WriteToStream(Stream& stream) const
	{
		stream.Write(magic, magicSize);

		stream.WriteUInt(static_cast<uint32_t>(referencedAssemblies.size()));
		for (const auto& reference : referencedAssemblies)
		{
			stream.WriteString(reference.name);
		}

		Backend::ModuleWriter writer(&stream);
		writer.Write(module.get());
		return 0;
	}
}