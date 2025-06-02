#ifndef ASSEMLBY_HPP
#define ASSEMLBY_HPP

#include "semantics/symbols/symbol_handle.hpp"
#include "lexical/text_span.hpp"
#include "io/stream.hpp"

#include "pch/std.hpp"

namespace HXSL
{
	class SymbolTable;
	class SymbolDef;
	class SymbolMetadata;

	enum AssemblyLoadResult
	{
		AssemblyLoadResult_Success = 0,
		AssemblyLoadResult_FileNotFound = -1,
		AssemblyLoadResult_ParseError = -2
	};

	class Assembly
	{
	private:
		Assembly(const std::string& name);

		std::unique_ptr<std::string> name;
		std::unique_ptr<SymbolTable> table;
		bool sealed;
	public:
		const std::string& GetName() const noexcept { return *name.get(); }

		const SymbolTable* GetSymbolTable() const noexcept { return table.get(); }

		SymbolTable* GetMutableSymbolTable() const { if (sealed) { throw std::logic_error("Cannot modify symbol table: Assembly is sealed."); } return table.get(); }

		void Seal() noexcept { sealed = true; };

		bool IsSealed() const noexcept { return sealed; }

		SymbolHandle AddSymbol(ASTContext* context, const StringSpan& name, SymbolDef* def, std::shared_ptr<SymbolMetadata>& metadata, const size_t& lookupIndex = 0);

		SymbolHandle AddSymbolScope(const StringSpan& name, std::shared_ptr<SymbolMetadata>& metadata, const size_t& lookupIndex = 0);

		static std::unique_ptr<Assembly> Create(const std::string& path);

		static AssemblyLoadResult LoadFromFile(const std::string& path, std::unique_ptr<Assembly>& assemblyOut);

		static AssemblyLoadResult LoadFromStream(const std::string& path, Stream& stream, std::unique_ptr<Assembly>& assemblyOut);

		int WriteToFile(const std::string& path) const;

		int WriteToStream(Stream& stream) const;
	};
}
#endif