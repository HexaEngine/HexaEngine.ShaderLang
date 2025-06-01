#ifndef SEMANTIC_ANALYZER_HPP
#define SEMANTIC_ANALYZER_HPP

#include "pch/ast_analyzers.hpp"
#include "logging/logger_adapter.hpp"

namespace HXSL
{
#define IF_ERR_RET_BREAK(expr) \
if (!expr) { \
	return TraversalBehavior_Break; \
}

	struct SemanticAnalyzer : public LoggerAdapter
	{
	private:
		ASTContext& context;
		CompilationUnit* compilation;

		const AssemblyCollection& references;
		std::unique_ptr<Assembly> outputAssembly;
		std::unique_ptr<PrimitiveManager> primitiveManager;
		std::unique_ptr<PointerManager> pointerManager;
		std::unique_ptr<ArrayManager> arrayManager;
		std::unique_ptr<SwizzleManager> swizzleManager;

		class AnalyzerVisitor : public ASTVisitor<EmptyDeferralContext>
		{
		private:
			SemanticAnalyzer& analyzer;

		public:
			AnalyzerVisitor(SemanticAnalyzer& analyzer) : analyzer(analyzer)
			{
			}

			TraversalBehavior Visit(ASTNode*& node, size_t depth, bool deferred, EmptyDeferralContext& context) override;
		};

		friend class SymbolResolver;

	public:
		SemanticAnalyzer(ILogger* logger, ASTContext& context, CompilationUnit* compilation, const AssemblyCollection& references) :
			LoggerAdapter(logger),
			context(context),
			compilation(compilation),
			references(references),
			outputAssembly(Assembly::Create("")),
			primitiveManager(std::make_unique<PrimitiveManager>(context)),
			pointerManager(std::make_unique<PointerManager>(context)),
			arrayManager(std::make_unique<ArrayManager>(context)),
			swizzleManager(std::make_unique<SwizzleManager>(context, *primitiveManager.get()))
		{
		}

		static void InitializeSubSystems();

		DEFINE_GET_SET_MOVE(std::unique_ptr<ArrayManager>, ArrayManager, arrayManager)

			DEFINE_GET_SET_MOVE(std::unique_ptr<PointerManager>, PointerManager, pointerManager)

			DEFINE_GET_SET_MOVE(std::unique_ptr<SwizzleManager>, SwizzleManager, swizzleManager)

			std::unique_ptr<Assembly>& GetOutputAssembly() noexcept { return outputAssembly; }

		CompilationUnit* Compilation() const noexcept { return compilation; }

		void WarmupCache();

		bool Analyze();
	};

	inline static std::string MakeScopeId(long num)
	{
		std::ostringstream oss;
		oss << num;
		return oss.str();
	}
}

#endif