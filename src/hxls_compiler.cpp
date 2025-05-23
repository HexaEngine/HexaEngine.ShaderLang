#include "hxls_compiler.hpp"

#include "pch/localization.hpp"
#include "preprocessing/preprocessor.hpp"
#include "parsers/parser.hpp"
#include "semantics/semantic_analyzer.hpp"
#include "il/il_generator.hpp"
#include "utils/ast_flattener.hpp"
#include "utils/ast_allocator.hpp"

#include <chrono>

namespace HXSL
{
	void Compiler::Compile(const std::vector<std::string>& files, const std::string& output, const AssemblyCollection& references)
	{
		RadixTree<Operator> tree;
		tree.Insert("!=", Operator_NotEqual);
		tree.Insert("!", Operator_LogicalNot);

		Parser::InitializeSubSystems();

		GetThreadAllocator()->Reset();

		std::unique_ptr<ILogger> logger = std::make_unique<ILogger>();
		ast_ptr<CompilationUnit> compilation = make_ast_ptr<CompilationUnit>();

		std::vector<std::unique_ptr<SourceFile>> sources;
		for (auto& file : files)
		{
			auto fs = FileStream::OpenRead(file.c_str());

			if (!fs)
			{
				std::cerr << "Error opening file." << std::endl;
				continue;
			}

			sources.push_back(std::make_unique<SourceFile>(fs.release(), true));
			auto& source = sources.back();

			if (!source->PrepareInputStream())
			{
				std::cerr << "Error reading file." << std::endl;
				continue;
			}

			Preprocessor preprocessor = Preprocessor(logger.get());
			preprocessor.Process(source.get());

			LexerContext context = LexerContext(source.get(), source->GetInputStream().get(), logger.get(), HXSLLexerConfig::Instance());
			TokenStream tokenStream = TokenStream(&context);

			Parser parser = Parser(logger.get(), tokenStream, compilation.get());

			parser.Parse();
		}

		SemanticAnalyzer::InitializeSubSystems();
		SemanticAnalyzer analyzer = SemanticAnalyzer(logger.get(), compilation.get(), references);
		analyzer.Analyze();

		auto& stats = GetThreadAllocator()->GetStats();

		ASTFlattener flattener;
		auto lowerCompilation = flattener.Flatten(compilation.get());
		compilation.reset();

		ILGenerator ilGen = ILGenerator(logger.get(), lowerCompilation.get());
		ilGen.Emit();

		if (!logger->HasErrors())
		{
			auto assembly = analyzer.GetOutputAssembly().get();

			assembly->WriteToFile(output);
		}
	}

	void Compiler::SetIncludeHandler(IncludeOpen includeOpen, IncludeClose includeClose)
	{
		includeOpen_ = includeOpen;
		includeClose_ = includeClose;
	}
}