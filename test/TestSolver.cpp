#include <gtest/gtest.h>

#include <Solver.hpp>
#include <ParserLL1.hpp>
#include <array>

class TestSolver : public ::testing::TestWithParam<std::tuple<std::string_view, double, std::string, std::string>>
{
public:
    const std::string_view line   = std::get<0>(GetParam());
    const double           expVal = std::get<1>(GetParam());
    const std::string      sym    = std::get<2>(GetParam());
    const std::string      expr   = std::get<3>(GetParam());
};

TEST_P(TestSolver, eval)
{
    LexScanner scanner(std::make_unique<std::istringstream>(line.data()));
    ParserLL1  parser(scanner);
    Solver     solver;

    ASSERT_TRUE(parser.parse());
    parser.ast().print();

    FAIL();

    // ASSERT_TRUE(solver.eval(parser.ast()));

    // // TODO: it should have the exact value, so it is needed to use the GNU MP/GNU MPFR
    // ASSERT_NEAR(solver.lastValue(), expVal, 1e-6);
    // // ASSERT_EQ(solver.lastValue(), expVal);
    // ASSERT_STREQ(solver.lastExpr().data(), expr.c_str());

    // if (!sym.empty())
    // {
    //     // TODO: it should have the exact value, so it is needed to use the GNU MP/GNU MPFR
    //     ASSERT_NEAR(solver.symbolTable().at(sym), expVal, 1e-6);
    //     // ASSERT_EQ(solver.symbolTable().at(sym), expVal);
    // }
}

INSTANTIATE_TEST_SUITE_P(
    SolverTestSuite,
    TestSolver,
    ::testing::Values(

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
