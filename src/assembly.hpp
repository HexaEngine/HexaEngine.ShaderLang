#ifndef ASSEMLBY_HPP
#define ASSEMLBY_HPP

#include "text_span.h"
#include "stream.h"
#include <string>
#include <memory>
#include <fstream>
#include <sstream>

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

		std::unique_ptr<std::string> Name;
		std::unique_ptr<SymbolTable> Table;
	public:
		const std::string& GetName() const noexcept { return *Name.get(); }

		const SymbolTable* GetSymbolTable() const noexcept { return Table.get(); }

		size_t AddSymbol(SymbolDef* def, std::shared_ptr<SymbolMetadata>& metadata, size_t lookupIndex = 0);

		size_t AddSymbolScope(TextSpan span, std::shared_ptr<SymbolMetadata>& metadata, size_t lookupIndex = 0);

		static std::unique_ptr<Assembly> Create(const std::string& path);

		static AssemblyLoadResult LoadFromFile(const std::string& path, std::unique_ptr<Assembly>& assemblyOut);

		static AssemblyLoadResult LoadFromStream(const std::string& path, Stream& stream, std::unique_ptr<Assembly>& assemblyOut);

		int WriteToFile(const std::string& path) const;

		int WriteToStream(Stream& stream) const;
	};
}
#endif