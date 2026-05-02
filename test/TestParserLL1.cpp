#include <gtest/gtest.h>

#include <ParserLL1.hpp>
#include <array>

class TestParserLL1 : public ::testing::TestWithParam<std::tuple<std::string_view, int, std::vector<Token>>>
{
public:
    const std::string_view   line         = std::get<0>(GetParam());
    const int                expNumTokens = std::get<1>(GetParam());
    const std::vector<Token> expTokens    = std::get<2>(GetParam());
};

TEST_P(TestParserLL1, parser)
{
    FAIL();
}

INSTANTIATE_TEST_SUITE_P(
    ParserLL1TestSuite,
    TestParserLL1,
    ::testing::Values(
        std::make_tuple("1", 1, std::vector<Token>{
                                    {.type = eTOKENS::NUM, .value = "1"},
})));

class TestParserLL1Error : public ::testing::TestWithParam<std::tuple<std::string_view>>
{
public:
    const std::string_view line = std::get<0>(GetParam());
};

TEST_P(TestParserLL1Error, parser_error)
{
    FAIL();
}

INSTANTIATE_TEST_SUITE_P(
    ParserLL1TestSuite,
    TestParserLL1Error,
    ::testing::Values(
        std::make_tuple("1.1.1")));

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
