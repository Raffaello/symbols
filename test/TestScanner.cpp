#include <gtest/gtest.h>

#include <Scanner.hpp>
#include <array>

class TestScanner : public ::testing::TestWithParam<std::tuple<std::string_view, int, std::vector<Token>>>
{
public:
    const std::string_view   line         = std::get<0>(GetParam());
    const int                expNumTokens = std::get<1>(GetParam());
    const std::vector<Token> expTokens    = std::get<2>(GetParam());
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
        EXPECT_EQ(r.type, expTokens[i].type);
        EXPECT_STRCASEEQ(r.value.c_str(), expTokens[i].value.c_str());
        ++i;
    }
}

INSTANTIATE_TEST_SUITE_P(
    ScannerTestSuite,
    TestScanner,
    ::testing::Values(
        std::make_tuple("1", 1, std::vector<Token>{
                                    {.type = eTOKENS::NUM, .value = "1"},
}),
        std::make_tuple("-1", 2, std::vector<Token>{
                                     {.type = eTOKENS::SUM_OP, .value = "-"},
                                     {.type = eTOKENS::NUM, .value = "1"},
                                 }),
        std::make_tuple("3.14", 1, std::vector<Token>{
                                       {.type = eTOKENS::NUM, .value = "3.14"},
                                   }),
        std::make_tuple(".1", 1, std::vector<Token>{
                                     {.type = eTOKENS::NUM, .value = ".1"},
                                 }),
        std::make_tuple("_1", 1, std::vector<Token>{
                                     {.type = eTOKENS::SYMBOL, .value = "_1"},
                                 }),
        std::make_tuple("1 + 2", 3, std::vector<Token>{
                                        {.type = eTOKENS::NUM, .value = "1"},
                                        {.type = eTOKENS::SUM_OP, .value = "+"},
                                        {.type = eTOKENS::NUM, .value = "2"},
                                    }),
        std::make_tuple(" 1  + \t 2 ", 3, std::vector<Token>{
                                              {.type = eTOKENS::NUM, .value = "1"},
                                              {.type = eTOKENS::SUM_OP, .value = "+"},
                                              {.type = eTOKENS::NUM, .value = "2"},
                                          }),
        std::make_tuple("1+2", 3, std::vector<Token>{
                                      {.type = eTOKENS::NUM, .value = "1"},
                                      {.type = eTOKENS::SUM_OP, .value = "+"},
                                      {.type = eTOKENS::NUM, .value = "2"},
                                  }),
        std::make_tuple("1 + x_1", 3, std::vector<Token>{
                                          {.type = eTOKENS::NUM, .value = "1"},
                                          {.type = eTOKENS::SUM_OP, .value = "+"},
                                          {.type = eTOKENS::SYMBOL, .value = "x_1"},
                                      }),
        std::make_tuple("10 + 2 - X1 * 1 / 2 + (a - b)", 15, std::vector<Token>{
                                                                 {.type = eTOKENS::NUM, .value = "10"},
                                                                 {.type = eTOKENS::SUM_OP, .value = "+"},
                                                                 {.type = eTOKENS::NUM, .value = "2"},
                                                                 {.type = eTOKENS::SUM_OP, .value = "-"},
                                                                 {.type = eTOKENS::SYMBOL, .value = "X1"},
                                                                 {.type = eTOKENS::MUL_OP, .value = "*"},
                                                                 {.type = eTOKENS::NUM, .value = "1"},
                                                                 {.type = eTOKENS::MUL_OP, .value = "/"},
                                                                 {.type = eTOKENS::NUM, .value = "2"},
                                                                 {.type = eTOKENS::SUM_OP, .value = "+"},
                                                                 {.type = eTOKENS::LEFT_PARENTHESES, .value = "("},
                                                                 {.type = eTOKENS::SYMBOL, .value = "a"},
                                                                 {.type = eTOKENS::SUM_OP, .value = "-"},
                                                                 {.type = eTOKENS::SYMBOL, .value = "b"},
                                                                 {.type = eTOKENS::RIGHT_PARENTHESES, .value = ")"},
                                                             })));

class TestScannerError : public ::testing::TestWithParam<std::tuple<std::string_view>>
{
public:
    const std::string_view line = std::get<0>(GetParam());
};

TEST_P(TestScannerError, tokenizer_error)
{
    Scanner scanner;

    auto res = scanner.tokenize(line);
    ASSERT_EQ(res.size(), 0);
}

INSTANTIATE_TEST_SUITE_P(
    ScannerTestSuite,
    TestScannerError,
    ::testing::Values(
        std::make_tuple("1.1.1"),
        std::make_tuple("._1"),
        std::make_tuple("_.1")));

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
