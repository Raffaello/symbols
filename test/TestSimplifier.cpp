#include <gtest/gtest.h>

#include <ParserLL1.hpp>
#include <Simplifier.hpp>

class TestSimplifier : public ::testing::TestWithParam<std::tuple<std::string, std::string>>
{
public:
    const std::string line   = std::get<0>(GetParam());
    const std::string expAST = std::get<1>(GetParam());
};

TEST_P(TestSimplifier, reduce)
{
    LexScanner scanner(std::make_unique<std::istringstream>(line.data()));
    ParserLL1  parser(scanner);

    ASSERT_TRUE(parser.parse());
    auto& ast = parser.ast();
    ast.print();

    EXPECT_TRUE(Simplifier::reduce(ast, true));
    EXPECT_STREQ(ast.to_string().c_str(), expAST.data());
}

INSTANTIATE_TEST_SUITE_P(
    SimplifierTestSuite,
    TestSimplifier,
    ::testing::Values(
        std::make_tuple("x+1+2*3", "x + 7"),
        std::make_tuple("-(-5)", "5"),
        std::make_tuple("+(-(+(-(-5))))", "-5"),
        std::make_tuple("10/2", "5"),
        std::make_tuple("2^0", "1"),
        std::make_tuple("2^3", "8"),
        std::make_tuple("2^3.1", "2^(31/10)"),
        std::make_tuple("(2+3)*4", "20"),
        std::make_tuple("2^1", "2"),
        std::make_tuple("10/4", "(5/2)"),
        std::make_tuple("x+0", "x"),
        std::make_tuple("x-0", "x"),
        std::make_tuple("x*1", "x"),
        std::make_tuple("x/1", "x"),
        std::make_tuple("x^0", "1"),    // 0^0 = 1 in this case
        std::make_tuple("x^1", "x"),
        std::make_tuple("0+x", "x"),
        std::make_tuple("0-x", "-x"),
        std::make_tuple("1*x", "x"),
        std::make_tuple("1/x", "1 / x"),
        std::make_tuple("0^x", "0^x"),    // 0^0 = 1 in this case
        std::make_tuple("1^x", "1"),
        std::make_tuple("0*x", "0"),
        std::make_tuple("x*0", "0"),

        std::make_tuple("0+x+0", "x"),
        std::make_tuple("0-x-0", "-x"),
        std::make_tuple("0-x+0", "-x"),
        std::make_tuple("0+x-0", "x"),
        std::make_tuple("1*x*1", "x"),
        std::make_tuple("1*x/1", "x"),

        std::make_tuple("0-x*1", "-x"),
        std::make_tuple("0+x/1", "x"),
        std::make_tuple("1*x+0", "x"),


        std::make_tuple("x*2", "x * 2"),
        std::make_tuple("2*x*2", "x * 4"),

        std::make_tuple("x^2^3", "x^8"),
        std::make_tuple("2^x^3", "2^(x^3)"),
        std::make_tuple("(x^2)^3", "x^6"),
        std::make_tuple("(2^x)^3", "(2^x)^3"),
        std::make_tuple("x/2/3", "x / 6"),
        std::make_tuple("2/x/3", "(2 / x) / 3"),
        std::make_tuple("x*2*3", "x * 6"),
        std::make_tuple("2*x*3", "x * 6"),
        std::make_tuple("x-2-3", "x - 5"),
        std::make_tuple("2-x-3", "(-x) - 1"),    // TODO: "(-x) - 1" => -1 - x is simplified more, need to sort by polynomial degree too
        std::make_tuple("x+2+3", "x + 5"),
        std::make_tuple("2+x+3", "x + 5"),

        std::make_tuple("2+x*4", "2 + (x * 4)"),

        std::make_tuple("x+x", "x * 2"),
        std::make_tuple("x-x", "0"),
        std::make_tuple("x*x", "x^2"),
        std::make_tuple("x/x", "1"),    // assuming x!= 0
        std::make_tuple("x^x", "x^x"),

        std::make_tuple("-1*x", "-x"),
        std::make_tuple("x*-1", "-x"),
        std::make_tuple("2+x*-1", "2 - x"),
        std::make_tuple("2-x*1", "2 - x"),
        std::make_tuple("2-x*0", "2"),
        std::make_tuple("2+(-x)*0", "2"),

        std::make_tuple("-(2+3)", "-5"),
        std::make_tuple("-(3-4)", "1"),

        std::make_tuple("+(1-(+x))", "1 - x"),
        std::make_tuple("-(1-x)", "x - 1"),
        std::make_tuple("-(x-1)", "1 - x"),
        std::make_tuple("+(-x+1)", "1 - x"),
        std::make_tuple("-(-x+1)", "x - 1"),
        std::make_tuple("(-x+1)*-1", "x - 1"),
        std::make_tuple("-1*(-x+1)", "x - 1"),
        std::make_tuple("(1+-x)", "1 - x"),

        std::make_tuple("(x+y)+3", "(x + y) + 3"),

        std::make_tuple("3*x*4", "x * 12"),
        std::make_tuple("-3*x*4*-1", "x * 12"),

        std::make_tuple("(x*x)^2", "x^4"),
        std::make_tuple("x*x^2", "x^3"),
        std::make_tuple("x*(x^2)", "x^3"),
        std::make_tuple("x^2*x", "x^3"),
        std::make_tuple("2^x*x", "(2^x) * x"),
        std::make_tuple("x*x^4", "x^5"),
        // std::make_tuple("x*x^n", "x^(n+1)"),    // TODO

        std::make_tuple("x*(x*2)", "(x^2) * 2"),
        std::make_tuple("x*(2*x)", "(x^2) * 2"),
        std::make_tuple("(x*2)*x", "(x^2) * 2"),
        std::make_tuple("(2*x)*x", "(x^2) * 2"),

        std::make_tuple("x^1*(x*2)", "(x^2) * 2"),

        std::make_tuple("x*(x/2)", "(x^2) / 2"),
        std::make_tuple("(x/2)*x", "(x^2) / 2"),
        std::make_tuple("x*(2/x)", "2"),    // assuming x!=0
        std::make_tuple("(2/x)*x", "2"),    // assuming x!=0

        std::make_tuple("-x*x^2", "-(x^3)"),
        std::make_tuple("-x*(x^2)", "-(x^3)"),
        std::make_tuple("x*-x^2", "-(x^3)"),
        std::make_tuple("x/-x^1", "-1"),    // x != 0
        // std::make_tuple("x/x^2", "x^-1") // TODO
        // std::make_tuple("x/-x^2", "-(x^-1)")    // TODO

        std::make_tuple("-x*-x", "x^2"),
        std::make_tuple("-x/-x", "1"),
        std::make_tuple("-x*-x*-x", "-(x^3)"),
        std::make_tuple("-x*-x*-x*-x", "x^4"),
        // std::make_tuple("x^2/x", "x"), // TODO
        // std::make_tuple("-x*-x*-x*-x/-x", "-x^3") // TODO

        std::make_tuple("x=1", "x - 1 = 0")

            ));

class TestSimplifier2 : public ::testing::TestWithParam<std::tuple<std::string, bool, std::string>>
{
public:
    const std::string line   = std::get<0>(GetParam());
    const bool        expRes = std::get<1>(GetParam());
    const std::string expAST = std::get<2>(GetParam());
};

TEST_P(TestSimplifier2, reduce_no_equation)
{
    LexScanner scanner(std::make_unique<std::istringstream>(line.data()));
    ParserLL1  parser(scanner);

    ASSERT_TRUE(parser.parse());
    auto& ast = parser.ast();
    ast.print();

    EXPECT_EQ(expRes, Simplifier::reduce(ast, false));
    EXPECT_STREQ(ast.to_string().c_str(), expAST.data());
}

INSTANTIATE_TEST_SUITE_P(
    SimplifierTestSuite,
    TestSimplifier2,
    ::testing::Values(
        std::make_tuple("x=1", false, "x = 1"),
        std::make_tuple("x=1+2", false, "x = 1 + 2")

            ));

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
