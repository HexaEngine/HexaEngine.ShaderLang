#ifndef ANALYZER_HPP
#define ANALYZER_HPP

#include "compilation.hpp"
#include "node_visitor.hpp"
#include "symbol_table.hpp"
#include "primitives.hpp"
#include "assembly_collection.hpp"
#include <string>

namespace HXSL
{
#define IF_ERR_RET_BREAK(expr) \
if (!expr) { \
	return HXSLTraversalBehavior_Break; \
}

	struct HXSLAnalyzer
	{
	private:
		Compilation* compilation;
		HXSLPrimitiveManager& primitives;

		const AssemblyCollection& references;
		std::unique_ptr<Assembly> outputAssembly;
		std::unique_ptr<HXSLSwizzleManager> swizzleManager;

	public:
		HXSLAnalyzer(Compilation* compilation, const AssemblyCollection& references) : compilation(compilation), references(references), outputAssembly(Assembly::Create("")), swizzleManager(std::make_unique<HXSLSwizzleManager>()), primitives(HXSLPrimitiveManager::GetInstance())
		{
		}

		std::unique_ptr<Assembly>& GetOutputAssembly() noexcept { return outputAssembly; }

		Compilation* Compilation() const noexcept { return compilation; }

		bool Analyze();

		template <typename... Args>
		void LogError(const std::string& message, const TextSpan& span, Args&&... args) const
		{
			std::string format = message + " (Line: %i, Column: %i)";
			compilation->LogFormatted(HXSLLogLevel_Error, format, std::forward<Args>(args)..., span.Line, span.Column);
		}
	};

	inline static std::string MakeScopeId(long num)
	{
		std::ostringstream oss;
		oss << num;
		return oss.str();
	}
}

#endif