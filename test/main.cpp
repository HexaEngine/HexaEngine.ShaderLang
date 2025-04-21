#include <iostream>
#include "lexical/token.h"
#include "lexical/lexer.h"
#include "token_stream.h"
#include "parser.h"
#include "parsers/expression_parser.hpp"
#include "parsers/declaration_parser.hpp"
#include "parsers/statement_parser.hpp"
#include <sub_parser_registry.hpp>
#include "analyzers/declaration_analyzer.hpp"
#include "sub_analyzer_registry.hpp"

#include <fstream>
#include <sstream>
#include <string>
#include <analyzer.hpp>

using namespace HXSL;

void Compile(const std::vector<std::string>& files, const std::string& output, const AssemblyCollection& references)
{
	std::unique_ptr<Compilation> compilation = std::make_unique<Compilation>();

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

		std::unique_ptr<std::string> content = std::make_unique<std::string>(move(buffer.str()));
		sources.push_back(move(content));
		auto& c = sources.back();

		LexerState state = LexerState(compilation.get(), c->data(), c->length());

		TokenStream stream = TokenStream(state, LexerConfigHXSL::Instance());

		HXSLParser parser = HXSLParser(stream, compilation.get());

		parser.Parse();
	}

	HXSLAnalyzer analyzer = HXSLAnalyzer(compilation.get(), references);

	analyzer.Analyze();

	auto assembly = analyzer.GetOutputAssembly().get();

	assembly->WriteToFile(output);
}

int main()
{
	HXSLSubParserRegistry::Register<HXSLStructParser>();
	HXSLSubParserRegistry::Register<HXSLDeclarationParser>();
	HXSLStatementParserRegistry::Register<HXSLMiscKeywordStatementParser>();
	HXSLStatementParserRegistry::Register<HXSLSwitchStatementParser>();
	HXSLStatementParserRegistry::Register<HXSLForStatementParser>();
	HXSLStatementParserRegistry::Register<HXSLWhileStatementParser>();
	HXSLStatementParserRegistry::Register<HXSLIfStatementParser>();
	HXSLStatementParserRegistry::Register<HXSLElseStatementParser>();
	HXSLStatementParserRegistry::Register<HXSLReturnStatementParser>();
	HXSLStatementParserRegistry::Register<HXSLDeclarationStatementParser>();
	HXSLStatementParserRegistry::Register<HXSLAssignmentStatementParser>();
	HXSLStatementParserRegistry::Register<HXSLFunctionCallStatementParser>();
	HXSLExpressionParserRegistry::Register<HXSLLiteralExpressionParser>();
	HXSLExpressionParserRegistry::Register<HXSLMemberAccessExpressionParser>();
	HXSLExpressionParserRegistry::Register<HXSLSymbolExpressionParser>();
	HXSLExpressionParserRegistry::Register<HXSLAssignmentExpressionParser>();

	HXSLSubAnalyzerRegistry::Register<HXSLDeclarationAnalyzer>();

	AssemblyCollection collection;
	//Compile({ "library.txt" }, "library.module", collection);

	collection.LoadAssemblyFromFile("library.module");

	Compile({ "shader.txt" , "shader2.txt" }, "shader.module", collection);

	return 0;
}