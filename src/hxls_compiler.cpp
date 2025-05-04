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
			std::ifstream fs = std::ifstream(file);

			if (!fs) {
				std::cerr << "Error opening file." << std::endl;
				return;
			}

			std::stringstream buffer;
			buffer << fs.rdbuf();

			sources.push_back(std::make_unique<SourceFile>(std::move(buffer.str())));
			auto& c = sources.back();

			LexerState state = LexerState(compilation.get(), c.get(), c->GetContent().data(), c->GetContent().length());

			Preprocessor preprocessor = Preprocessor();

			TokenStream stream = TokenStream(preprocessor, state, HXSLLexerConfig::Instance());

			Parser parser = Parser(stream, compilation.get());

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