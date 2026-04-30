#include <gtest/gtest.h>

#include <Scanner.hpp>
#include <array>

class TestScanner : public ::testing::TestWithParam<std::tuple<std::string_view, int, std::vector<Token>>>
{
public:
    const std::string_view   line         = std::get<0>(GetParam());
    const int                expNumTokens = std::get<1>(GetParam());
    const std::vector<Token> expTokens    = std::get<2>(GetParam());

    TestScanner()
    {
    }
};

TEST_P(TestScanner, tokenizer)
{
    Scanner scanner;

    ASSERT_EQ(expNumTokens, expTokens.size());

    auto res = scanner.tokenize(line);
    ASSERT_EQ(res.size(), expNumTokens);

    int i = 0;
    for (const auto& r : res)
    {
        EXPECT_EQ(r.token, expTokens[i].token);
        EXPECT_STRCASEEQ(r.value.c_str(), expTokens[i].value.c_str());
        ++i;
    }
}

INSTANTIATE_TEST_SUITE_P(
    ScannerTestSuite,
    TestScanner,
    ::testing::Values(
        std::make_tuple("1 + 2", 3, std::vector<Token>{
                                        {.token = eTOKENS::DIGIT,    .value = "1"},
                                        {.token = eTOKENS::OPERATOR, .value = "+"},
                                        {.token = eTOKENS::DIGIT,    .value = "2"},
}),
        std::make_tuple(" 1  + \t 2 ", 3, std::vector<Token>{
                                              {.token = eTOKENS::DIGIT, .value = "1"},
                                              {.token = eTOKENS::OPERATOR, .value = "+"},
                                              {.token = eTOKENS::DIGIT, .value = "2"},
                                          }),
        std::make_tuple("1 + x_1", 3, std::vector<Token>{
                                          {.token = eTOKENS::DIGIT, .value = "1"},
                                          {.token = eTOKENS::OPERATOR, .value = "+"},
                                          {.token = eTOKENS::SYMBOL, .value = "x_1"},
                                      }),
        std::make_tuple("10 + 2 - X1 * 1 / 2 + (a - b)", 15, std::vector<Token>{
                                                                 {.token = eTOKENS::DIGIT, .value = "10"},
                                                                 {.token = eTOKENS::OPERATOR, .value = "+"},
                                                                 {.token = eTOKENS::DIGIT, .value = "2"},
                                                                 {.token = eTOKENS::OPERATOR, .value = "-"},
                                                                 {.token = eTOKENS::SYMBOL, .value = "X1"},
                                                                 {.token = eTOKENS::OPERATOR, .value = "*"},
                                                                 {.token = eTOKENS::DIGIT, .value = "1"},
                                                                 {.token = eTOKENS::OPERATOR, .value = "/"},
                                                                 {.token = eTOKENS::DIGIT, .value = "2"},
                                                                 {.token = eTOKENS::OPERATOR, .value = "+"},
                                                                 {.token = eTOKENS::PARENTHESES, .value = "("},
                                                                 {.token = eTOKENS::SYMBOL, .value = "a"},
                                                                 {.token = eTOKENS::OPERATOR, .value = "-"},
                                                                 {.token = eTOKENS::SYMBOL, .value = "b"},
                                                                 {.token = eTOKENS::PARENTHESES, .value = ")"},
                                                             })));

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
