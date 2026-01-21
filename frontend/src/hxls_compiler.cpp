#include "hxls_compiler.hpp"

#include "ast_modules/ast_context.hpp"
#include "ast_modules/ast_validator.hpp"
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
		if (span.source == INVALID_SOURCE_ID) return {};
		auto& manger = ASTContext::GetCurrentContext()->GetSourceManager();
		auto source = manger.GetSource(span.source);	
		return source->GetSpan(span.start, span.length);
	}

	static std::string textSpanGetStr(const TextSpan& span)
	{
		if (span.source == INVALID_SOURCE_ID) return {};
		auto& manger = ASTContext::GetCurrentContext()->GetSourceManager();
		auto source = manger.GetSource(span.source);
		return source->GetString(span.start, span.length);
	}

	static std::unique_ptr<Backend::Module> CompileFrontend(ILogger* logger, const std::vector<std::string>& files, const AssemblyCollection& references)
	{
		Parser::InitializeSubSystems();

		uptr<ASTContext> context = make_uptr<ASTContext>();
		ASTContext::SetCurrentContext(context.get());
		CompilationUnitBuilder builder = CompilationUnitBuilder(logger);
		for (auto& file : files)
		{
			auto fs = FileStream::OpenRead(file.c_str());

			if (!fs)
			{
				std::cerr << "Error opening file." << std::endl;
				continue;
			}

			auto source = context->GetSourceManager().AddSource(fs.release(), true);

			if (!source->PrepareInputStream())
			{
				std::cerr << "Error reading file." << std::endl;
				continue;
			}

			Preprocessor preprocessor = Preprocessor(logger);
			preprocessor.Process(source);

			LexerContext lexerContext = LexerContext(context->GetIdentifierTable(), source, source->GetInputStream().get(), logger, HXSLLexerConfig::Instance());
			TokenStream tokenStream = TokenStream(&lexerContext);

			Parser parser = Parser(logger, tokenStream);

			parser.Parse(builder);
		}

		CompilationUnit* compilation = builder.Build();

		ASTValidator validator = ASTValidator(logger);
		validator.Validate(compilation);

		SemanticAnalyzer::InitializeSubSystems();
		SemanticAnalyzer analyzer = SemanticAnalyzer(logger, compilation, references);
		analyzer.Analyze();

		if (logger->HasErrors())
		{
			return nullptr;
		}

		ModuleBuilder conv;
		return conv.Convert(compilation);
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
		if (!module)
		{
			return;
		}

		Backend::ControlFlowAnalyzer cfAnalyzer = Backend::ControlFlowAnalyzer(logger.get(), module.get());
		cfAnalyzer.Analyze();

		Backend::ILOptimizer optimizer = Backend::ILOptimizer(logger.get(), module.get());
		optimizer.Optimize();

		if (!logger->HasErrors())
		{
			auto outputStream = FileStream::OpenCreate(output.c_str());
			Backend::ModuleWriter writer(outputStream.get());
			writer.Write(module.get());
		}
	}

	void Compiler::SetIncludeHandler(IncludeOpen includeOpen, IncludeClose includeClose)
	{
		includeOpen_ = includeOpen;
		includeClose_ = includeClose;
	}
}