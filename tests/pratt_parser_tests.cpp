#include <gtest/gtest.h>
#include "parsers/parser.h"
#include "parsers/pratt_parser.hpp"

using namespace HXSL;

TEST(ParserTest, CanParseSingleExpression) 
{
    Parser::InitializeSubSystems();
   
    pool_ptr<Compilation> compilation = make_pool_ptr<NodeType_Compilation>();

    std::string test_source = "x + y;";  

    LexerState state = LexerState(compilation.get(), test_source.data(), test_source.length());
    TokenStream stream = TokenStream(state, HXSLLexerConfig::Instance());

    Parser parser = Parser(stream, compilation.get());
    stream.TryAdvance(); // initial advance to emulate parser behavior.

    ExpressionPtr expr;
    PrattParser::ParseExpression(parser, stream, compilation.get(), expr);
}


int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}