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

TEST_P(TestSolver, solver)
{
    LexScanner scanner(std::make_unique<std::istringstream>(line.data()));
    ParserLL1  parser(scanner);
    Solver     solver(std::make_shared<SymbolTable>());

    ASSERT_TRUE(parser.parse());

    AST& ast = parser.ast();

    ASSERT_TRUE(solver.solve(ast, sym));
    EXPECT_STREQ(solver.solution().c_str(), expAST.c_str());
}

INSTANTIATE_TEST_SUITE_P(
    SolverTestSuite,
    TestSolver,
    ::testing::Values(
        std::make_tuple("-x=1", "x", "x = -1"),

        std::make_tuple("x=1", "x", "x = 1"),
        std::make_tuple("+x=1", "x", "x = 1"),
        std::make_tuple("1=x", "x", "x = 1"),
        std::make_tuple("1+0=x", "x", "x = 1"),
        std::make_tuple("1*1=x", "x", "x = 1"),

        std::make_tuple("2*x=1", "x", "x = 0.5"),
        std::make_tuple("2*x/2=1", "x", "x = 1"),
        std::make_tuple("2*x/-2=-1", "x", "x = 1"),

        std::make_tuple("(2+x)*(3-2)+x=0", "x", "x = -1"),
        std::make_tuple("(2+x)*(3-2)+x*1+0=0", "x", "x = -1"),
        std::make_tuple("(2+x)*(3-2)+x=-2*x", "x", "x = -0.5"),
        std::make_tuple("(2+x)*(3-2)+x*1+0=-2*x", "x", "x = -0.5"),
        std::make_tuple("(2+x)*(3-2)+x*1+0=2*-x", "x", "x = -0.5"),
        std::make_tuple("(2+x)*(3-2)+x*1+0=-2*-x*1", "x", "no solution"),
        std::make_tuple("(2+x)*(3-2)+x*1+0=-2*-x*-1", "x", "x = -0.5"),

        std::make_tuple("11*x=0", "x", "x = 0"),
        // POW
        std::make_tuple("x^1 = 0", "x", "x = 0"),
        std::make_tuple("x^1 = 1", "x", "x = 1"),
        std::make_tuple("x^2 = 1", "x", "x = -1, x = 1"),
        std::make_tuple("x^(1+1) = 1", "x", "x = -1, x = 1"),
        std::make_tuple("(x+1)^1 = 1", "x", "x = 0"),
        std::make_tuple("(x+1)^0 = 1", "x", "inf solutions"),
        std::make_tuple("(x+1)^2 = 1", "x", "x = -2, x = 0"),

        std::make_tuple("(x+0)^2 = 1", "x", "x = -1, x = 1"),
        std::make_tuple("x^0 = 1", "x", "inf solutions"),
        std::make_tuple("x^0 + x = 1", "x", "x = 0"),

        std::make_tuple("(x+1)^3 = 1", "x", "x = 0"),
        std::make_tuple("(x+1)^3 = 0", "x", "x = -1"),
        std::make_tuple("x^3+ 2*x^2 -5*x^1 - 6*x^0 = 0", "x", "x = -3, x = -1, x = 2")

        // std::make_tuple("(2*x+1)^3 = 1", "x", "x = 0, x = ..."),
        // std::make_tuple("(x+1)^4 = 1", "x", "x = 0, x = ..."),

        // std::make_tuple("a=x", "x", "x = a"),
        // std::make_tuple("a+1+0=x", "x", "x = a + 1"),
        // std::make_tuple("a+1*1=x", "x", "x = a + 1"),

        // std::make_tuple("x+1+a = 0", "x", "..")

        ));

class TestSolverSymbolSubstitution : public TestSolver
{
public:
};

TEST_P(TestSolverSymbolSubstitution, Symbol_Simple_Substitution)
{
    auto       pSymbolTable = std::make_shared<SymbolTable>();
    LexScanner scanner(std::make_unique<std::istringstream>(line.data()));
    ParserLL1  parser(scanner);
    Solver     solver(pSymbolTable);

    (*pSymbolTable)["a"] = mp::mpq_rational{10};
    (*pSymbolTable)["b"] = mp::mpq_rational{5};


    ASSERT_TRUE(parser.parse());

    AST& ast = parser.ast();

    ASSERT_TRUE(solver.solve(ast, sym));

    EXPECT_STREQ(solver.solution().c_str(), expAST.c_str());
}

INSTANTIATE_TEST_SUITE_P(
    SolverTestSuite,
    TestSolverSymbolSubstitution,
    ::testing::Values(
        std::make_tuple("a = x", "x", "x = 10"),
        std::make_tuple("b = x", "x", "x = 5"),
        std::make_tuple("a+b = x", "x", "x = 15"),
        std::make_tuple("a-b = x", "x", "x = 5"),
        std::make_tuple("a-2*b = x", "x", "x = 0"),
        std::make_tuple("a/b = x", "x", "x = 2"),
        std::make_tuple("a*2+a^2 -10 -b*b = x - 3", "x", "x = 88")

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
        std::make_tuple("x=1", "y"),

        // POW
        std::make_tuple("x^x=0", "x"),
        std::make_tuple("a^x=0", "x"),
        std::make_tuple("x^a=0", "x"),
        std::make_tuple("1^x=0", "x"),
        std::make_tuple("x+1+a = 0", "x"),
        std::make_tuple("x+ 2*a = 0", "x"),
        std::make_tuple("x=1/0", "x"),

        std::make_tuple("x=2^0.5", "x"),       // TODO: this should be solvable, need to simplify expression first
        std::make_tuple("x=(2^0.5)^2", "x")    // TODO: this should be solvable, need to simplify expression first

        ));

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
