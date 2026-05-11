#include "assembly.hpp"
#include "semantics/symbols/symbol_table.hpp"
#include "semantics/semantic_analyzer.hpp"
#include "core/module_reader.hpp"
#include "core/module_writer.hpp"
#include "core/module_linker.hpp"
#include "../compiler_context.hpp"

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

	inline Assembly::Assembly(const StringSpan& pathOrName) : table(std::make_unique<SymbolTable>()), sealed(false)
	{
		StringSpan span = pathOrName;
		auto idx = span.LastIndexOf([](const char c) { return c == '/' || c == '\\'; });
		if (idx.IsValid())
		{
			span = span[{idx + 1, 0_rr}];
		}

		name = make_uptr<std::string>(span.str());
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

	std::unique_ptr<Assembly> Assembly::Create(const StringSpan& path)
	{
		return std::unique_ptr<Assembly>(new Assembly(path));
	}

	AssemblyLoadResult Assembly::LoadFromFile(const std::string& path, std::unique_ptr<Assembly>& assemblyOut)
	{
		auto fs = FileStream::OpenRead(path.c_str());

		if (!fs)
		{
			return AssemblyLoadResult::FileNotFound;
		}

		auto result = LoadFromStream(path, fs, assemblyOut);
		return result;
	}

	AssemblyLoadResult Assembly::LoadFromStream(const std::string& path, const ObjPtr<Stream>& stream, std::unique_ptr<Assembly>& assemblyOut)
	{
		char buffer[magicSize];
		stream->Read(buffer, magicSize);

		if (memcmp(buffer, magic, magicSize) != 0)
		{
			return AssemblyLoadResult::ParseError;
		}

		auto assembly = Create(path);

		auto referenceCount = stream->ReadUInt();
		for (uint32_t i = 0; i < referenceCount; ++i)
		{
			AssemblyReference reference;
			reference.name = stream->ReadString();
			assembly->referencedAssemblies.push_back(std::move(reference));
		}

		auto ctx = CompilerContext::GetCurrent();
		Backend::ModuleLinker linker = { ctx->GetResolver() };
		Backend::ModuleReader reader(stream, linker);
		assembly->module = reader.Read();

		assembly->Seal();
		assemblyOut = std::move(assembly);
		return AssemblyLoadResult::Success;
	}
}