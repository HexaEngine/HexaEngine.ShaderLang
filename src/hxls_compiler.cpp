#include "hxls_compiler.hpp"

#include "pch/localization.hpp"
#include "semantics/semantic_analyzer.hpp"
#include "parsers/parser.hpp"
#include <preprocessing/preprocessor.hpp>

namespace HXSL
{
	void Compiler::Compile(const std::vector<std::string>& files, const std::string& output, const AssemblyCollection& references)
	{
		Parser::InitializeSubSystems();

		pool_ptr<Compilation> compilation = make_pool_ptr<NodeType_Compilation>();

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

			Preprocessor preprocessor = Preprocessor(compilation.get());
			preprocessor.Process(source.get());

			LexerContext context = LexerContext(source.get(), source->GetInputStream().get(), compilation.get(), HXSLLexerConfig::Instance());
			TokenStream tokenStream = TokenStream(&context);

			Parser parser = Parser(tokenStream, compilation.get());

			parser.Parse();
		}

		SemanticAnalyzer::InitializeSubSystems();
		SemanticAnalyzer analyzer = SemanticAnalyzer(compilation.get(), references);

		analyzer.Analyze();
		if (!compilation->HasErrors())
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