#include <gtest/gtest.h>

#include <LexScanner.hpp>
#include <array>

class TestLexScanner : public ::testing::TestWithParam<std::tuple<std::string_view, std::vector<Token>>>
{
public:
    const std::string_view   line         = std::get<0>(GetParam());
    const std::vector<Token> expTokens    = std::get<1>(GetParam());
    const int                expNumTokens = expTokens.size();
};

TEST_P(TestLexScanner, tokenizer)
{
    LexScanner scanner(std::make_unique<std::istringstream>(line.data()));

    int i = 0;
    while (scanner.next())
    {
        const auto t = scanner.lastToken();

        ASSERT_LT(i, expNumTokens);
        EXPECT_EQ(t.type, expTokens[i].type);
        EXPECT_STRCASEEQ(t.value.c_str(), expTokens[i].value.c_str());
        ++i;
    }

    EXPECT_EQ(i, expNumTokens);
}

INSTANTIATE_TEST_SUITE_P(
    LexScannerTestSuite,
    TestLexScanner,
    ::testing::Values(
        std::make_tuple("1", std::vector<Token>{
                                 {.type = eTOKENS::NUM, .value = "1"},
                                 {.type = eTOKENS::END, .value = "" },
}),
        std::make_tuple("123", std::vector<Token>{
                                   {.type = eTOKENS::NUM, .value = "123"},
                                   {.type = eTOKENS::END, .value = ""},
                               }),
        std::make_tuple("-1", std::vector<Token>{
                                  {.type = eTOKENS::SUM_OP, .value = "-"},
                                  {.type = eTOKENS::NUM, .value = "1"},
                                  {.type = eTOKENS::END, .value = ""},
                              }),
        std::make_tuple("3.14", std::vector<Token>{
                                    {.type = eTOKENS::NUM, .value = "3.14"},
                                    {.type = eTOKENS::END, .value = ""},
                                }),
        std::make_tuple(".1", std::vector<Token>{
                                  {.type = eTOKENS::NUM, .value = ".1"},
                                  {.type = eTOKENS::END, .value = ""},
                              }),
        std::make_tuple("_1", std::vector<Token>{
                                  {.type = eTOKENS::SYMBOL, .value = "_1"},
                                  {.type = eTOKENS::END, .value = ""},
                              }),
        std::make_tuple("1 + 2", std::vector<Token>{
                                     {.type = eTOKENS::NUM, .value = "1"},
                                     {.type = eTOKENS::SUM_OP, .value = "+"},
                                     {.type = eTOKENS::NUM, .value = "2"},
                                     {.type = eTOKENS::END, .value = ""},
                                 }),
        std::make_tuple(" 1  + \t 2 ", std::vector<Token>{
                                           {.type = eTOKENS::NUM, .value = "1"},
                                           {.type = eTOKENS::SUM_OP, .value = "+"},
                                           {.type = eTOKENS::NUM, .value = "2"},
                                           {.type = eTOKENS::END, .value = ""},
                                       }),
        std::make_tuple("1+2", std::vector<Token>{
                                   {.type = eTOKENS::NUM, .value = "1"},
                                   {.type = eTOKENS::SUM_OP, .value = "+"},
                                   {.type = eTOKENS::NUM, .value = "2"},
                                   {.type = eTOKENS::END, .value = ""},
                               }),
        std::make_tuple("1 + x_1", std::vector<Token>{
                                       {.type = eTOKENS::NUM, .value = "1"},
                                       {.type = eTOKENS::SUM_OP, .value = "+"},
                                       {.type = eTOKENS::SYMBOL, .value = "x_1"},
                                       {.type = eTOKENS::END, .value = ""},
                                   }),
        std::make_tuple("(1)", std::vector<Token>{
                                   {.type = eTOKENS::LEFT_PARENTHESES, .value = "("},
                                   {.type = eTOKENS::NUM, .value = "1"},
                                   {.type = eTOKENS::RIGHT_PARENTHESES, .value = ")"},
                                   {.type = eTOKENS::END, .value = ""},
                               }),
        std::make_tuple("(a)", std::vector<Token>{
                                   {.type = eTOKENS::LEFT_PARENTHESES, .value = "("},
                                   {.type = eTOKENS::SYMBOL, .value = "a"},
                                   {.type = eTOKENS::RIGHT_PARENTHESES, .value = ")"},
                                   {.type = eTOKENS::END, .value = ""},
                               }),
        std::make_tuple("((a)", std::vector<Token>{
                                    {.type = eTOKENS::LEFT_PARENTHESES, .value = "("},
                                    {.type = eTOKENS::LEFT_PARENTHESES, .value = "("},
                                    {.type = eTOKENS::SYMBOL, .value = "a"},
                                    {.type = eTOKENS::RIGHT_PARENTHESES, .value = ")"},
                                    {.type = eTOKENS::END, .value = ""},
                                }),
        std::make_tuple("(a))", std::vector<Token>{
                                    {.type = eTOKENS::LEFT_PARENTHESES, .value = "("},
                                    {.type = eTOKENS::SYMBOL, .value = "a"},
                                    {.type = eTOKENS::RIGHT_PARENTHESES, .value = ")"},
                                    {.type = eTOKENS::RIGHT_PARENTHESES, .value = ")"},
                                    {.type = eTOKENS::END, .value = ""},
                                }),
        std::make_tuple(")(+-/*", std::vector<Token>{
                                      {.type = eTOKENS::RIGHT_PARENTHESES, .value = ")"},
                                      {.type = eTOKENS::LEFT_PARENTHESES, .value = "("},
                                      {.type = eTOKENS::SUM_OP, .value = "+"},
                                      {.type = eTOKENS::SUM_OP, .value = "-"},
                                      {.type = eTOKENS::MUL_OP, .value = "/"},
                                      {.type = eTOKENS::MUL_OP, .value = "*"},
                                      {.type = eTOKENS::END, .value = ""},
                                  }),
        std::make_tuple("(a", std::vector<Token>{
                                  {.type = eTOKENS::LEFT_PARENTHESES, .value = "("},
                                  {.type = eTOKENS::SYMBOL, .value = "a"},
                                  {.type = eTOKENS::END, .value = ""},
                              }),
        std::make_tuple(")a", std::vector<Token>{
                                  {.type = eTOKENS::RIGHT_PARENTHESES, .value = ")"},
                                  {.type = eTOKENS::SYMBOL, .value = "a"},
                                  {.type = eTOKENS::END, .value = ""},
                              }),
        std::make_tuple("=", std::vector<Token>{
                                 {.type = eTOKENS::EQUAL, .value = "="},
                                 {.type = eTOKENS::END, .value = ""},
                             }),
        std::make_tuple("9=", std::vector<Token>{
                                  {.type = eTOKENS::NUM, .value = "9"},
                                  {.type = eTOKENS::EQUAL, .value = "="},
                                  {.type = eTOKENS::END, .value = ""},
                              }),
        std::make_tuple("x=1", std::vector<Token>{
                                   {.type = eTOKENS::SYMBOL, .value = "x"},
                                   {.type = eTOKENS::EQUAL, .value = "="},
                                   {.type = eTOKENS::NUM, .value = "1"},
                                   {.type = eTOKENS::END, .value = ""},
                               }),
        std::make_tuple(")1(a", std::vector<Token>{
                                    {.type = eTOKENS::RIGHT_PARENTHESES, .value = ")"},
                                    {.type = eTOKENS::NUM, .value = "1"},
                                    {.type = eTOKENS::LEFT_PARENTHESES, .value = "("},
                                    {.type = eTOKENS::SYMBOL, .value = "a"},
                                    {.type = eTOKENS::END, .value = ""},
                                }),
        std::make_tuple(")1.2(a", std::vector<Token>{
                                      {.type = eTOKENS::RIGHT_PARENTHESES, .value = ")"},
                                      {.type = eTOKENS::NUM, .value = "1.2"},
                                      {.type = eTOKENS::LEFT_PARENTHESES, .value = "("},
                                      {.type = eTOKENS::SYMBOL, .value = "a"},
                                      {.type = eTOKENS::END, .value = ""},
                                  }),
        std::make_tuple(")1.2_(_", std::vector<Token>{
                                       {.type = eTOKENS::RIGHT_PARENTHESES, .value = ")"},
                                       {.type = eTOKENS::NUM, .value = "1.2"},
                                       {.type = eTOKENS::SYMBOL, .value = "_"},
                                       {.type = eTOKENS::LEFT_PARENTHESES, .value = "("},
                                       {.type = eTOKENS::SYMBOL, .value = "_"},
                                       {.type = eTOKENS::END, .value = ""},
                                   }),
        std::make_tuple("_.1", std::vector<Token>{
                                   {.type = eTOKENS::SYMBOL, .value = "_"},
                                   {.type = eTOKENS::NUM, .value = ".1"},
                                   {.type = eTOKENS::END, .value = ""},
                               }),
        std::make_tuple(")a_(_", std::vector<Token>{
                                     {.type = eTOKENS::RIGHT_PARENTHESES, .value = ")"},
                                     {.type = eTOKENS::SYMBOL, .value = "a_"},
                                     {.type = eTOKENS::LEFT_PARENTHESES, .value = "("},
                                     {.type = eTOKENS::SYMBOL, .value = "_"},
                                     {.type = eTOKENS::END, .value = ""},
                                 }),
        std::make_tuple(")a4(1", std::vector<Token>{
                                     {.type = eTOKENS::RIGHT_PARENTHESES, .value = ")"},
                                     {.type = eTOKENS::SYMBOL, .value = "a4"},
                                     {.type = eTOKENS::LEFT_PARENTHESES, .value = "("},
                                     {.type = eTOKENS::NUM, .value = "1"},
                                     {.type = eTOKENS::END, .value = ""},
                                 }),
        std::make_tuple("10 + 2 - X1 * 1 / 2 + (a - b)", std::vector<Token>{
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
                                                             {.type = eTOKENS::END, .value = ""},
                                                         }),
        std::make_tuple("^", std::vector<Token>{
                                 {.type = eTOKENS::POW_OP, .value = "^"},
                                 {.type = eTOKENS::END, .value = ""},
                             }),
        std::make_tuple("2^b", std::vector<Token>{
                                   {.type = eTOKENS::NUM, .value = "2"},
                                   {.type = eTOKENS::POW_OP, .value = "^"},
                                   {.type = eTOKENS::SYMBOL, .value = "b"},
                                   {.type = eTOKENS::END, .value = ""},
                               })

            ));

class TestLexScannerError : public ::testing::TestWithParam<std::tuple<std::string_view>>
{
public:
    const std::string_view line = std::get<0>(GetParam());
};

TEST_P(TestLexScannerError, tokenizer_error)
{
    LexScanner scanner(std::make_unique<std::istringstream>(line.data()));

    EXPECT_FALSE(scanner.next());
    const auto t = scanner.lastToken();
    EXPECT_EQ(t.type, eTOKENS::ERROR);
}

INSTANTIATE_TEST_SUITE_P(
    LexScannerTestSuite,
    TestLexScannerError,
    ::testing::Values(
        std::make_tuple("1.1.1"),
        std::make_tuple("._1"),
        std::make_tuple("!")));

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
