#include <gtest/gtest.h>

#include <Solver.hpp>
#include <ParserLL1.hpp>
#include <array>

class TestSolver : public ::testing::TestWithParam<std::tuple<std::string, std::string, std::string>>
{
public:
    const std::string line   = std::get<0>(GetParam());
    const std::string sym    = std::get<1>(GetParam());
    const std::string expAST = std::get<2>(GetParam());
};

TEST_P(TestSolver, eval)
{
    LexScanner scanner(std::make_unique<std::istringstream>(line.data()));
    ParserLL1  parser(scanner);
    Solver     solver(std::make_shared<SymbolTable>());

    ASSERT_TRUE(parser.parse());

    AST& ast = parser.ast();

    ASSERT_TRUE(solver.solve(ast, sym));
    // EXPECT_STREQ(ast.to_string().c_str(), expAST.data());
    EXPECT_STREQ(solver.solution().c_str(), expAST.c_str());
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
        std::make_tuple("2*x=1", "x", "x = 0.5"),
        std::make_tuple("2*x/2=1", "x", "x = 1")

            ));

class TestSolverError : public ::testing::TestWithParam<std::tuple<std::string, std::string>>
{
public:
    const std::string line = std::get<0>(GetParam());
    const std::string sym  = std::get<1>(GetParam());
};

TEST_P(TestSolverError, eval_error)
{
    LexScanner scanner(std::make_unique<std::istringstream>(line.data()));
    ParserLL1  parser(scanner);
    Solver     solver(std::make_shared<SymbolTable>());

    ASSERT_TRUE(parser.parse());

    AST& ast = parser.ast();

    ASSERT_FALSE(solver.solve(ast, sym));
    ASSERT_STREQ(solver.solution().c_str(), "");
}

INSTANTIATE_TEST_SUITE_P(
    SolverTestSuite,
    TestSolverError,
    ::testing::Values(
        std::make_tuple("x=1", "y")

            ));

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
