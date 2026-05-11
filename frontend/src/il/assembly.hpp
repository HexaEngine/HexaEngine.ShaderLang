#ifndef ASSEMLBY_HPP
#define ASSEMLBY_HPP

#include "semantics/symbols/symbol_handle.hpp"
#include "lexical/text_span.hpp"
#include "io/stream.hpp"

#include "pch/std.hpp"
#include "pch/il.hpp"
#include "utils/memory.hpp"
#include "assembly_format.hpp"

namespace HXSL
{
	class SymbolTable;
	class SymbolDef;
	class SymbolMetadata;

	enum class AssemblyLoadResult
	{
		Success = 0,
		FileNotFound = -1,
		ParseError = -2
	};

	struct AssemblyReference
	{
		std::string name;

		constexpr AssemblyReference() = default;
		constexpr AssemblyReference(const std::string& name) : name(name) {}
		constexpr AssemblyReference(std::string&& name) : name(std::move(name)) {}

		void Write(Stream* stream) const
		{
			stream->WriteLittleEndian<uint32_t>(name.size());
			stream->Write(name.c_str(), name.size());
		}

		void Read(Stream* stream)
		{
			auto len = stream->ReadLittleEndian<uint32_t>();
			name.resize(len);
			stream->Read(name.data(), len);
		}

		constexpr bool operator==(const AssemblyReference& other) const { return name == other.name; }
		constexpr bool operator!=(const AssemblyReference& other) const { return !(*this == other); }

		size_t hash() const
		{
			XXHash3_64 hash;
			hash.Combine(name.data(), name.size());
			return static_cast<size_t>(hash.Finalize());
		}
	};

	class AssemblyBuilder;

	class Assembly
	{
		using RecordId = Backend::RecordId;
		using Module = Backend::Module;

		friend class AssemblyBuilder;
	private:
		Assembly(const StringSpan& pathOrName);

		uptr<std::string> name;
		uptr<SymbolTable> table;
		uptr<Module> module;
		RecordId entryPoint = {};
		Architecture architecture = Architecture::Unknown;
		LanguageIdentifier languageId = LanguageIdentifier::Unknown;

		std::vector<AssemblyReference> referencedAssemblies;
		bool sealed;
	public:
		const std::string& GetName() const noexcept { return *name.get(); }

		AssemblyReference AsReference() const { return AssemblyReference(*name.get()); }

		Module* GetModule() { return module.get(); }

		const Module* GetModule() const { return module.get(); }

		RecordId GetEntryPointId() const { return entryPoint; }

		Backend::FunctionLayout* GetEntryPoint()
		{
			if (entryPoint.IsNull()) return nullptr;
			auto mainModule = GetModule();
			auto& entryPointSymbol = mainModule->GetExportTable()[entryPoint];
			return Backend::cast<Backend::FunctionLayout>(entryPointSymbol.layout);
		}

		Architecture GetArchitecture() const { return architecture; }

		LanguageIdentifier GetLanguageIdentifier() const { return languageId; }

		ConstSpan<AssemblyReference> GetReferencedAssemblies() const noexcept { return referencedAssemblies; }

		const SymbolTable* GetSymbolTable() const noexcept { return table.get(); }

		SymbolTable* GetMutableSymbolTable() const { if (sealed) { throw std::logic_error("Cannot modify symbol table: Assembly is sealed."); } return table.get(); }

		void Seal() noexcept { sealed = true; };

		void UnsealUnsafe() noexcept { sealed = false; }

		bool IsSealed() const noexcept { return sealed; }

		SymbolHandle AddSymbol(const StringSpan& name, SymbolDef* def, const ObjPtr<SymbolMetadata>& metadata, SymbolTableNode* lookupIndex = nullptr);

		SymbolHandle AddSymbolScope(const StringSpan& name, const ObjPtr<SymbolMetadata>& metadata, SymbolTableNode* lookupIndex = nullptr);

		static std::unique_ptr<Assembly> Create(const StringSpan& path);

		static AssemblyLoadResult LoadFromFile(const std::string& path, std::unique_ptr<Assembly>& assemblyOut);

		static AssemblyLoadResult LoadFromStream(const std::string& path, const ObjPtr<Stream>& stream, std::unique_ptr<Assembly>& assemblyOut);
	};
}

namespace std
{
	template<>
	struct hash<HXSL::AssemblyReference>
	{
		size_t operator()(const HXSL::AssemblyReference& ref) const noexcept
		{
			return ref.hash();
		}
	};
}

#endif