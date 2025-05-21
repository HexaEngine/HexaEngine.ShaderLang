#ifndef COMMON_HPP
#define COMMON_HPP

#include <gtest/gtest.h>
#include "parsers/parser.hpp"
#include "parsers/hybrid_expr_parser.hpp"
#include "pch/ast_analyzers.hpp"
#include "expect_file.hpp"

using namespace HXSL;

class ASTValidatorVisitor : Visitor<EmptyDeferralContext>
{
	std::ostringstream debugOutput;

	TraversalBehavior Visit(ASTNode*& node, size_t depth, bool deferred, EmptyDeferralContext& context) override
	{
		std::string indentation(depth * 2, ' ');
		auto& span = node->GetSpan();
		debugOutput << indentation << node->DebugName() << " (Line: " << span.line << " Column: " << span.column << ")" << std::endl;
		return TraversalBehavior_Keep;
	}

	static std::vector<std::string> GetLines(std::string input)
	{
		std::vector<std::string> lines;
		std::string line;
		std::istringstream stream(input);

		while (std::getline(stream, line))
		{
			lines.push_back(line);
		}

		return lines;
	}

public:
	ASTValidatorVisitor()
	{
	}

	void Validate(ILogger* log, ASTNode* actualAst, const ExpectFileSubContent& expected)
	{
		auto& expectedAst = expected.expected;
		debugOutput.clear();
		if (actualAst)
		{
			Traverse(actualAst);
		}
		auto actualAstStr = debugOutput.str();
		auto actualLines = GetLines(actualAstStr);
		auto expectedLines = GetLines(expectedAst);

		bool hadError = false;

		if (actualLines.size() != expectedLines.size())
		{
			EXPECT_EQ(actualLines.size(), expectedLines.size())
				<< "The number of nodes in the actual and expected ASTs don't match." << std::endl;
			hadError = true;
		}

		size_t size = std::min(actualLines.size(), expectedLines.size());

		for (size_t i = 0; i < size; ++i)
		{
			if (actualLines[i] != expectedLines[i])
			{
				hadError = true;
				EXPECT_EQ(actualLines[i], expectedLines[i])
					<< "Mismatch at node " << i + 1 << std::endl
					<< ": Expected Node: " << expectedLines[i] << std::endl
					<< "Actual Node: " << actualLines[i] << std::endl;
			}
		}

		auto& actualMessages = log->GetMessages();
		auto& expectedMessages = expected.expectedLogs;

		if (actualMessages.size() != expectedMessages.size())
		{
			EXPECT_EQ(actualMessages.size(), expectedMessages.size())
				<< "The number of messages the actual and expected don't match." << std::endl;
			hadError = true;
		}

		auto messageCount = std::min(actualMessages.size(), expectedMessages.size());

		for (size_t i = 0; i < messageCount; ++i)
		{
			auto& act = actualMessages[i];
			auto& exp = expectedMessages[i];
			if (act.Level != exp.Level || std::strcmp(act.Message, exp.Message) != 0)
			{
				hadError = true;
				EXPECT_EQ(act.Level, exp.Level)
					<< "Mismatch at message " << i + 1 << std::endl
					<< ": Expected message type: " << ToString(exp.Level) << std::endl
					<< "Actual message type: " << ToString(act.Level) << std::endl;

				EXPECT_STREQ(act.Message, exp.Message)
					<< "Mismatch at message " << i + 1 << std::endl
					<< ": Expected message: " << exp.Message << std::endl
					<< "Actual message: " << act.Message << std::endl;
			}
		}

		if (hadError)
		{
			std::string str;
			for (auto& msg : actualMessages)
			{
				str.append(msg.ToString());
				str.push_back('\n');
			}
			FAIL() << std::endl
				<< "Expected AST:" << std::endl
				<< expectedAst << std::endl
				<< "Actual AST:" << std::endl
				<< actualAstStr << std::endl
				<< "Messages:" << std::endl
				<< str << std::endl;
		}
	}
};

using CompilationPtr = std::unique_ptr<CompilationUnit>;
using ASTNodePtr = std::unique_ptr<ASTNode>;

class HXSLTestBase
{
public:
	std::string LoadStringFromFile(const std::string& filename)
	{
		std::ifstream file(filename);
		if (!file.is_open())
		{
			std::cerr << "Error opening file: " << filename << std::endl;
			return "";
		}
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}

	void PrepareTestData(const std::string& data, Parser& parser, TokenStream& stream, CompilationPtr& compilation)
	{
		Parser::InitializeSubSystems();

		compilation = std::make_unique<CompilationUnit>();

		LexerState state = LexerState(compilation.get(), data.c_str(), data.length());
		stream = TokenStream(state, HXSLLexerConfig::Instance());
		parser = Parser(stream, compilation.get());
	}

	virtual void Act() = 0;

	virtual ~HXSLTestBase() = default;
};

class ASTTestBase : public HXSLTestBase, public ::testing::TestWithParam<std::tuple<std::string, std::string>>
{
	std::string input;
	TestFileContent expect;

protected:
	ASTTestBase() : HXSLTestBase()
	{
	}

	void SetUp() override
	{
		expect = ParseTestFile(std::get<0>(GetParam()));
	}

	virtual std::tuple<CompilationPtr, ASTNodePtr> ActCore(const std::string& input) = 0;

	void Act() override
	{
		for (auto& content : expect.content)
		{
			auto tuple = ActCore(content.input);
			CompilationPtr compilation = std::move(std::get<0>(tuple));
			ASTNodePtr target = std::move(std::get<1>(tuple));
			if (compilation)
			{
				ASTValidatorVisitor validator;
				validator.Validate(compilation.get(), target.get(), content);
				ActPost(compilation.get(), target.get(), content);
			}
		}
	}

	virtual void ActPost(ILogger* log, ASTNode* actualAst, const ExpectFileSubContent& expected)
	{
	}
};

#endif