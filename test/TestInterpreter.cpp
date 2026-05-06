#include <gtest/gtest.h>

#include <Interpreter.hpp>
#include <ParserLL1.hpp>
#include <array>

class TestInterpreter : public ::testing::TestWithParam<std::tuple<std::string_view, double, std::string, std::string, double>>
{
public:
    const std::string_view line      = std::get<0>(GetParam());
    const double           expVal    = std::get<1>(GetParam());
    const std::string      sym       = std::get<2>(GetParam());
    const std::string      expr      = std::get<3>(GetParam());
    const double           expSymVal = std::get<4>(GetParam());
};

TEST_P(TestInterpreter, eval)
{
    LexScanner  scanner(std::make_unique<std::istringstream>(line.data()));
    ParserLL1   parser(scanner);
    Interpreter interpreter;

    ASSERT_TRUE(parser.parse());
    parser.ast().print();
    ASSERT_TRUE(interpreter.eval(parser.ast()));

    // TODO: it should have the exact value, so it is needed to use the GNU MP/GNU MPFR
    ASSERT_NEAR(interpreter.lastValue(), expVal, 1e-6);
    // ASSERT_EQ(interpreter.lastValue(), expVal);
    ASSERT_STREQ(interpreter.lastExpr().data(), expr.c_str());

    if (!sym.empty())
    {
        // TODO: it should have the exact value, so it is needed to use the GNU MP/GNU MPFR
        ASSERT_NEAR(interpreter.symbolTable().at(sym), expSymVal, 1e-6);
        // ASSERT_EQ(interpreter.symbolTable().at(sym), expVal);
    }
}

INSTANTIATE_TEST_SUITE_P(
    InterpreterTestSuite,
    TestInterpreter,
    ::testing::Values(
        std::make_tuple("1", 1.0, "", "1", 0),
        std::make_tuple("(1)", 1.0, "", "1", 0),
        std::make_tuple("1+1", 2.0, "", "1 + 1 = 2", 0),
        std::make_tuple("1-1", 0.0, "", "1 - 1 = 0", 0),
        std::make_tuple("-1", -1.0, "", "-(1)", 0),
        std::make_tuple("+1", 1.0, "", "1", 0),
        std::make_tuple("-(+(-(+(-1))))", -1.0, "", "-(1)", 0),
        std::make_tuple("10 + 2 - 10 * 1 / 2 + (3.14 - .14)", 10.0, "", "7 + 3 = 10", 0),
        std::make_tuple("x=1", 1.0, "x", "x = 1", 1),
        std::make_tuple("x=(1 * 10)   / 2. - 4.1", 0.9, "x", "x = 0.9000000000000004", 0.9),
        std::make_tuple("2^2", 4.0, "", "2 ^ 2 = 4", 0),
        std::make_tuple("(1+1)^(2*1)", 4.0, "", "2 ^ 2 = 4", 0),
        std::make_tuple("1+2*2^2", 9.0, "", "1 + 8 = 9", 0),
        std::make_tuple("-2^2", -4.0, "", "-(2 ^ 2 = 4)", 0),
        std::make_tuple("2^-2", 0.25, "", "2 ^ -2 = 0.25", 0),
        std::make_tuple("2^3^2", 512, "", "2 ^ 9 = 512", 0),
        std::make_tuple("2*3^2+1", 19, "", "18 + 1 = 19", 0),
        std::make_tuple("2*3, 5*6", 5 * 6, "", "2 * 3 = 6, 5 * 6 = 30", 0),
        std::make_tuple("x=5, x*6", 30, "x", "x = 5, 5 * 6 = 30", 5)


            ));

class TestInterpreterError : public ::testing::TestWithParam<std::string_view>
{
public:
    const std::string_view line = GetParam();
};

TEST_P(TestInterpreterError, eval_error)
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
        "1+x",         // TODO: x is not defined (x=1; 1+x) that could be 1 line valid eventually, but not sure if it works as a multiline parser at the moment.
        "1+x = 1+2"    // TODO: equation not supported yet

        ));

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
