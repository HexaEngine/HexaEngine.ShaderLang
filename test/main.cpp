#include <iostream>
#include "lexical/token.h"
#include "lexical/lexer.h"
#include "lexical/token_stream.h"
#include "parsers/parser.h"
#include "parsers/expression_parser.hpp"
#include "parsers/declaration_parser.hpp"
#include "parsers/statement_parser.hpp"
#include "parsers/sub_parser_registry.hpp"

#include "analyzers/analyzer.hpp"
#include "analyzers/declaration_analyzer.hpp"
#include "analyzers/sub_analyzer_registry.hpp"

#include <fstream>
#include <sstream>
#include <string>

using namespace HXSL;

void Compile(const std::vector<std::string>& files, const std::string& output, const AssemblyCollection& references)
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

int main()
{
	AssemblyCollection collection;
	Compile({ "library.txt" }, "library.module", collection);

	collection.LoadAssemblyFromFile("library.module");

	Compile({ "shader.txt" , "shader2.txt" }, "shader.module", collection);

	return 0;
}