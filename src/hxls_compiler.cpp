#include "hxls_compiler.hpp"

#include "generated/localization.hpp"
#include "analyzers/analyzer.hpp"
#include "parsers/parser.hpp"
#include <preprocessing/preprocessor.hpp>

namespace HXSL
{
	void HXSLCompiler::Compile(const std::vector<std::string>& files, const std::string& output, const AssemblyCollection& references)
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

			LexerContext context = LexerContext(source.get(), source->GetInputStream(), compilation.get(), HXSLLexerConfig::Instance());
			Preprocessor preprocessor = Preprocessor();
			TokenStream tokenStream = TokenStream(&context, preprocessor);

			Parser parser = Parser(tokenStream, compilation.get());

			parser.Parse();
		}

		Analyzer::InitializeSubSystems();
		Analyzer analyzer = Analyzer(compilation.get(), references);

		analyzer.Analyze();
		if (!compilation->HasErrors())
		{
			auto assembly = analyzer.GetOutputAssembly().get();

			assembly->WriteToFile(output);
		}
	}
}