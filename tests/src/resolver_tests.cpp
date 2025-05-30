#include "common.hpp"

class ResolverTest : public HXSLTestBase, public ::testing::TestWithParam<std::tuple<std::string, std::string>>
{
	std::string input;
	TestFileContent expect;

protected:
	ResolverTest() : HXSLTestBase()
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