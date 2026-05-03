#include <gtest/gtest.h>

#include <ParserLL1.hpp>
#include <array>

class TestParserLL1 : public ::testing::TestWithParam<std::tuple<std::string_view>>
{
public:
    const std::string_view line = std::get<0>(GetParam());
};

TEST_P(TestParserLL1, parser)
{
    LexScanner scanner(std::make_unique<std::istringstream>(line.data()));
    ParserLL1  parser(scanner);

    ASSERT_TRUE(parser.parse());

    auto& ast = parser.ast();
    // TODO: check the AST
    ast.print();
}

INSTANTIATE_TEST_SUITE_P(
    ParserLL1TestSuite,
    TestParserLL1,
    ::testing::Values(
        std::make_tuple("1"),
        std::make_tuple("x"),
        std::make_tuple("(1)"),
        std::make_tuple("(x)"),
        std::make_tuple("((x))"),
        std::make_tuple("1+1"),
        std::make_tuple("1-1"),
        std::make_tuple("1+a"),
        std::make_tuple("a-123"),
        std::make_tuple("(a-b)"),
        std::make_tuple("(a-b + c - 1)"),
        std::make_tuple("(a-b) + (c - 1)"),
        std::make_tuple("(a*b) + (c / 1)"),
        std::make_tuple("(a*b  +  c / 1)"),
        std::make_tuple("-1"),
        std::make_tuple("+1"),
        std::make_tuple("+a"),
        std::make_tuple("-a"),
        std::make_tuple("-(+(-(+(-1))))"),
        std::make_tuple("-(+(-(+(-a))))")));

class TestParserLL1Error : public ::testing::TestWithParam<std::string_view>
{
public:
    const std::string_view line = GetParam();
};

TEST_P(TestParserLL1Error, parser_error)
{
    LexScanner scanner(std::make_unique<std::istringstream>(line.data()));
    ParserLL1  parser(scanner);

    ASSERT_FALSE(parser.parse());
}

INSTANTIATE_TEST_SUITE_P(
    ParserLL1TestSuite,
    TestParserLL1Error,
    ::testing::Values(
        "(",
        "(1",
        "(x",
        "(x",
        "(x+",
        "(2-",
        "(2*",
        "(x/",
        "--1",
        "+-1",
        "++1",
        "+-a",
        "++a",
        "--a"));

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
