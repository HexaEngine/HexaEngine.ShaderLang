#ifndef HXSL_COMPILER_HPP
#define HXSL_COMPILER_HPP

#include "parsers/parser.h"
#include "analyzers/analyzer.hpp"

namespace HXSL
{
	class HXSLCompiler
	{
	public:
		static void Compile(const std::vector<std::string>& files, const std::string& output, const AssemblyCollection& references)
		{
			Parser::InitializeSubSystems();

			pool_ptr<Compilation> compilation = make_pool_ptr<NodeType_Compilation>();

			std::vector<std::unique_ptr<std::string>> sources;
			for (auto& file : files)
			{
				std::ifstream fs = std::ifstream(file);

				if (!fs) {
					std::cerr << "Error opening file." << std::endl;
					return;
				}

				std::stringstream buffer;
				buffer << fs.rdbuf();

				sources.push_back(std::make_unique<std::string>(std::move(buffer.str())));
				auto& c = sources.back();

				LexerState state = LexerState(compilation.get(), c->data(), c->length());

				TokenStream stream = TokenStream(state, HXSLLexerConfig::Instance());

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
	};
}

#endif