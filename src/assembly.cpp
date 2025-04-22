#include "assembly.hpp"
#include "symbols/symbol_table.hpp"
namespace HXSL
{
	inline Assembly::Assembly(const std::string& name) : Name(std::make_unique<std::string>(name)), Table(std::make_unique<SymbolTable>())
	{
	}

	size_t Assembly::AddSymbol(SymbolDef* def, std::shared_ptr<SymbolMetadata>& metadata, size_t lookupIndex)
	{
		auto& name = def->GetName();
		auto index = Table->Insert(name, metadata, lookupIndex);
		if (index != 0)
		{
			def->SetAssembly(this, index);
		}
		return index;
	}

	size_t Assembly::AddSymbolScope(TextSpan span, std::shared_ptr<SymbolMetadata>& metadata, size_t lookupIndex)
	{
		return Table->Insert(span, metadata, lookupIndex);
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

		assembly->Table->Read(stream, assembly.get());

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
		Table->Write(stream);
		return 0;
	}
}