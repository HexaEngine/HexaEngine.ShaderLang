#include "hxls_compiler.hpp"

#include "pch/localization.hpp"
#include "preprocessing/preprocessor.hpp"
#include "parsers/parser.hpp"
#include "semantics/semantic_analyzer.hpp"
#include "middleware/module_builder.hpp"
#include "il/control_flow_analyzer.hpp"
#include "optimizers/il_optimizer.hpp"

namespace HXSL
{
	static StringSpan textSpanGetSpan(const TextSpan& span)
	{
		if (span.source == nullptr) return {};
		auto source = reinterpret_cast<SourceFile*>(span.source);
		return source->GetSpan(span.start, span.length);
	}

	static std::string textSpanGetStr(const TextSpan& span)
	{
		if (span.source == nullptr) return {};
		auto source = reinterpret_cast<SourceFile*>(span.source);
		return source->GetString(span.start, span.length);
	}

	static std::unique_ptr<Backend::Module> CompileFrontend(ILogger* logger, const std::vector<std::string>& files, const AssemblyCollection& references)
	{
		Parser::InitializeSubSystems();

		GetThreadAllocator()->Reset();

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

			Preprocessor preprocessor = Preprocessor(logger);
			preprocessor.Process(source.get());

			LexerContext context = LexerContext(source.get(), source->GetInputStream().get(), logger, HXSLLexerConfig::Instance());
			TokenStream tokenStream = TokenStream(&context);

			Parser parser = Parser(logger, tokenStream, compilation.get());

			parser.Parse();
		}

		SemanticAnalyzer::InitializeSubSystems();
		SemanticAnalyzer analyzer = SemanticAnalyzer(logger, compilation.get(), references);
		analyzer.Analyze();

		ModuleBuilder conv;
		return conv.Convert(compilation.get());
	}

	void Compiler::Compile(const std::vector<std::string>& files, const std::string& output, const AssemblyCollection& references)
	{
		TextSpan::textSpanGetSpan = textSpanGetSpan;
		TextSpan::textSpanGetStr = textSpanGetStr;
		DiagnosticCode::encodeDiagnosticCode = EncodeCodeId;
		DiagnosticCode::getMessageForCode = GetMessageForCode;
		DiagnosticCode::getStringForCode = GetStringForCode;

		std::unique_ptr<ILogger> logger = std::make_unique<ILogger>();

		auto module = CompileFrontend(logger.get(), files, references);
		GetThreadAllocator()->ReleaseAll();

		Backend::ControlFlowAnalyzer cfAnalyzer = Backend::ControlFlowAnalyzer(logger.get(), module.get());
		cfAnalyzer.Analyze();

		Backend::ILOptimizer optimizer = Backend::ILOptimizer(logger.get(), module.get());
		optimizer.Optimize();

		if (!logger->HasErrors())
		{
			//auto assembly = analyzer.GetOutputAssembly().get();

			//assembly->WriteToFile(output);
		}
	}

	void Compiler::SetIncludeHandler(IncludeOpen includeOpen, IncludeClose includeClose)
	{
		includeOpen_ = includeOpen;
		includeClose_ = includeClose;
	}
}