#include <gtest/gtest.h>

#include <Interpreter.hpp>
#include <ParserLL1.hpp>
#include <array>

class TestInterpreter : public ::testing::TestWithParam<std::tuple<std::string_view, double>>
{
public:
    const std::string_view line   = std::get<0>(GetParam());
    const double           expVal = std::get<1>(GetParam());
};

TEST_P(TestInterpreter, parser)
{
    LexScanner  scanner(std::make_unique<std::istringstream>(line.data()));
    ParserLL1   parser(scanner);
    Interpreter interpreter;

    ASSERT_TRUE(parser.parse());
    ASSERT_TRUE(interpreter.eval(parser.ast()));

    // TODO: it should have the exact value, so it is needed to use the GNU MP
    ASSERT_NEAR(interpreter.lastValue(), expVal, 1e-6);
    // ASSERT_EQ(interpreter.lastValue(), expVal);
}

INSTANTIATE_TEST_SUITE_P(
    InterpreterTestSuite,
    TestInterpreter,
    ::testing::Values(
        std::make_tuple("1", 1.0),
        std::make_tuple("(1)", 1.0),
        std::make_tuple("1+1", 2.0),
        std::make_tuple("1-1", 0.0),
        std::make_tuple("-1", -1.0),
        std::make_tuple("+1", 1.0),
        std::make_tuple("-(+(-(+(-1))))", -1.0),
        std::make_tuple("10 + 2 - 10 * 1 / 2 + (3.14 - .14)", 10.0),
        std::make_tuple("x=1", 1.0),
        std::make_tuple("x=1 * 10  / 2. - 4.1", 0.9)));

class TestInterpreterError : public ::testing::TestWithParam<std::string_view>
{
public:
    const std::string_view line = GetParam();
};

TEST_P(TestInterpreterError, parser_error)
{
    LexScanner  scanner(std::make_unique<std::istringstream>(line.data()));
    ParserLL1   parser(scanner);
    Interpreter interpreter;

    ASSERT_TRUE(parser.parse());
    ASSERT_FALSE(interpreter.eval(parser.ast()));
}

INSTANTIATE_TEST_SUITE_P(
    InterpreterTestSuite,
    TestInterpreterError,
    ::testing::Values(
        "1+x"));    // TODO: x is not defined (x=1; 1+x) that could be 1 line valid eventually, but not sure if it works as a multiline parser at the moment.

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
