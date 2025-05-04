#include "assembly.hpp"
#include "symbols/symbol_table.hpp"

namespace HXSL
{
	inline Assembly::Assembly(const std::string& name) : name(std::make_unique<std::string>(name)), table(std::make_unique<SymbolTable>()), sealed(false)
	{
	}

	SymbolHandle Assembly::AddSymbol(const StringSpan& name, SymbolDef* def, std::shared_ptr<SymbolMetadata>& metadata, const size_t& lookupIndex)
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

	SymbolHandle Assembly::AddSymbolScope(const StringSpan& span, std::shared_ptr<SymbolMetadata>& metadata, const size_t& lookupIndex)
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
		auto assembly = Create(path);

		assembly->table->Read(stream, assembly.get());

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
		table->Write(stream);
		return 0;
	}
}