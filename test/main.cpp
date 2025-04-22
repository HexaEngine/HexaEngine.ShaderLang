#include <iostream>
#include "lexical/token.h"
#include "lexical/lexer.h"
#include "lexical/token_stream.h"
#include "parsers/parser.h"
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

		std::unique_ptr<std::string> content = std::make_unique<std::string>(std::move(buffer.str()));
		sources.push_back(std::move(content));
		auto& c = sources.back();

		LexerState state = LexerState(compilation.get(), c->data(), c->length());

		TokenStream stream = TokenStream(state, HXSLLexerConfig::Instance());

		Parser parser = Parser(stream, compilation.get());

		parser.Parse();
	}

	Analyzer analyzer = Analyzer(compilation.get(), references);

	analyzer.Analyze();

	auto assembly = analyzer.GetOutputAssembly().get();

	assembly->WriteToFile(output);
}

int main()
{
	SubParserRegistry::Register<StructParser>();
	SubParserRegistry::Register<DeclarationParser>();
	StatementParserRegistry::Register<MiscKeywordStatementParser>();
	StatementParserRegistry::Register<SwitchStatementParser>();
	StatementParserRegistry::Register<ForStatementParser>();
	StatementParserRegistry::Register<WhileStatementParser>();
	StatementParserRegistry::Register<IfStatementParser>();
	StatementParserRegistry::Register<ElseStatementParser>();
	StatementParserRegistry::Register<ReturnStatementParser>();
	StatementParserRegistry::Register<DeclarationStatementParser>();
	StatementParserRegistry::Register<AssignmentStatementParser>();
	StatementParserRegistry::Register<FunctionCallStatementParser>();
	ExpressionParserRegistry::Register<LiteralExpressionParser>();
	ExpressionParserRegistry::Register<MemberAccessExpressionParser>();
	ExpressionParserRegistry::Register<SymbolExpressionParser>();
	ExpressionParserRegistry::Register<AssignmentExpressionParser>();

	SubAnalyzerRegistry::Register<DeclarationAnalyzer>();

	AssemblyCollection collection;
	//Compile({ "library.txt" }, "library.module", collection);

	collection.LoadAssemblyFromFile("library.module");

	Compile({ "shader.txt" , "shader2.txt" }, "shader.module", collection);

	return 0;
}