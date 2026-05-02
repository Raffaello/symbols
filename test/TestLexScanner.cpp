#include <gtest/gtest.h>

#include <LexScanner.hpp>
#include <array>

class TestLexScanner : public ::testing::TestWithParam<std::tuple<std::string_view, int, std::vector<Token>>>
{
public:
    const std::string_view   line         = std::get<0>(GetParam());
    const int                expNumTokens = std::get<1>(GetParam());
    const std::vector<Token> expTokens    = std::get<2>(GetParam());
};

TEST_P(TestLexScanner, tokenizer)
{
    LexScanner scanner(std::move(std::make_unique<std::istringstream>(line.data())));

    ASSERT_EQ(expNumTokens, expTokens.size());

    int i = 0;
    while (scanner.next())
    {
        const auto t = scanner.lastToken();
        EXPECT_EQ(t.type, expTokens[i].type);
        EXPECT_STRCASEEQ(t.value.c_str(), expTokens[i].value.c_str());
        ++i;
    }

    EXPECT_EQ(i, expNumTokens);
}

INSTANTIATE_TEST_SUITE_P(
    ScannerTestSuite,
    TestLexScanner,
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
        std::make_tuple("(1)", 3, std::vector<Token>{
                                      {.type = eTOKENS::LEFT_PARENTHESES, .value = "("},
                                      {.type = eTOKENS::NUM, .value = "1"},
                                      {.type = eTOKENS::RIGHT_PARENTHESES, .value = ")"},
                                  }),
        std::make_tuple("(a)", 3, std::vector<Token>{
                                      {.type = eTOKENS::LEFT_PARENTHESES, .value = "("},
                                      {.type = eTOKENS::SYMBOL, .value = "a"},
                                      {.type = eTOKENS::RIGHT_PARENTHESES, .value = ")"},
                                  }),
        std::make_tuple("((a)", 4, std::vector<Token>{
                                       {.type = eTOKENS::LEFT_PARENTHESES, .value = "("},
                                       {.type = eTOKENS::LEFT_PARENTHESES, .value = "("},
                                       {.type = eTOKENS::SYMBOL, .value = "a"},
                                       {.type = eTOKENS::RIGHT_PARENTHESES, .value = ")"},
                                   }),
        std::make_tuple("(a))", 4, std::vector<Token>{
                                       {.type = eTOKENS::LEFT_PARENTHESES, .value = "("},
                                       {.type = eTOKENS::SYMBOL, .value = "a"},
                                       {.type = eTOKENS::RIGHT_PARENTHESES, .value = ")"},
                                       {.type = eTOKENS::RIGHT_PARENTHESES, .value = ")"},
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

class TestLexScannerError : public ::testing::TestWithParam<std::tuple<std::string_view>>
{
public:
    const std::string_view line = std::get<0>(GetParam());
};

TEST_P(TestLexScannerError, tokenizer_error)
{
    LexScanner scanner(std::move(std::make_unique<std::istringstream>(line.data())));

    EXPECT_FALSE(scanner.next());
    const auto t = scanner.lastToken();
    EXPECT_EQ(t.type, eTOKENS::ERROR);
}

INSTANTIATE_TEST_SUITE_P(
    ScannerTestSuite,
    TestLexScannerError,
    ::testing::Values(
        std::make_tuple("1.1.1"),
        std::make_tuple("._1"),
        std::make_tuple("_.1"),
        std::make_tuple("a(")));    // TODO these could be 2 valid tokens, the parser instead should report error missing operator

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
