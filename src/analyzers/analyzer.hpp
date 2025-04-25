#ifndef ANALYZER_HPP
#define ANALYZER_HPP

#include "ast_analyzers.hpp"
#include <string>

namespace HXSL
{
#define IF_ERR_RET_BREAK(expr) \
if (!expr) { \
	return TraversalBehavior_Break; \
}

	struct Analyzer
	{
	private:
		Compilation* compilation;
		PrimitiveManager& primitives;

		const AssemblyCollection& references;
		std::unique_ptr<Assembly> outputAssembly;
		std::unique_ptr<SwizzleManager> swizzleManager;

	public:
		Analyzer(Compilation* compilation, const AssemblyCollection& references) : compilation(compilation), references(references), outputAssembly(Assembly::Create("")), swizzleManager(std::make_unique<SwizzleManager>()), primitives(PrimitiveManager::GetInstance())
		{
		}

		static void InitializeSubSystems();

		std::unique_ptr<Assembly>& GetOutputAssembly() noexcept { return outputAssembly; }

		Compilation* Compilation() const noexcept { return compilation; }

		void WarmupCache();

		bool Analyze();

		template <typename... Args>
		void LogError(const std::string& message, const TextSpan& span, Args&&... args) const
		{
			std::string format = message + " (Line: %i, Column: %i)";
			compilation->LogFormatted(LogLevel_Error, format, std::forward<Args>(args)..., span.Line, span.Column);
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