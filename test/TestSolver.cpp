#include <gtest/gtest.h>

#include <Solver.hpp>
#include <ParserLL1.hpp>
#include <array>

class TestSolver : public ::testing::TestWithParam<std::tuple<std::string_view, std::string, std::string>>
{
public:
    const std::string_view line   = std::get<0>(GetParam());
    const std::string      sym    = std::get<1>(GetParam());
    const std::string      expAST = std::get<2>(GetParam());
};

TEST_P(TestSolver, eval)
{
    LexScanner scanner(std::make_unique<std::istringstream>(line.data()));
    ParserLL1  parser(scanner);
    Solver     solver;

    ASSERT_TRUE(parser.parse());

    AST& ast = parser.ast();

    ASSERT_TRUE(solver.solve(ast, sym));
    EXPECT_STREQ(ast.to_string().c_str(), expAST.data());
}

INSTANTIATE_TEST_SUITE_P(
    SolverTestSuite,
    TestSolver,
    ::testing::Values(
        std::make_tuple("x=1", "x", "x = 1"),
        std::make_tuple("1=x", "x", "x = 1"),
        std::make_tuple("1+0=x", "x", "x = 1"),
        std::make_tuple("1*1=x", "x", "x = 1"),
        std::make_tuple("a=x", "x", "x = a"),
        std::make_tuple("a+1+0=x", "x", "x = a + 1"),
        std::make_tuple("a+1*1=x", "x", "x = a + 1"),
        std::make_tuple("2*x=1", "x", "x = -0.5")

            ));

class TestSolverError : public ::testing::TestWithParam<std::string_view>
{
public:
    const std::string_view line = GetParam();
};

TEST_P(TestSolverError, eval_error)
{
    LexScanner scanner(std::make_unique<std::istringstream>(line.data()));
    ParserLL1  parser(scanner);
    Solver     solver;

    ASSERT_TRUE(parser.parse());

    FAIL();

    // ASSERT_FALSE(solver.eval(parser.ast()));
}

INSTANTIATE_TEST_SUITE_P(
    SolverTestSuite,
    TestSolverError,
    ::testing::Values(

        ));

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
