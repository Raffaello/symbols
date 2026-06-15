#include <gtest/gtest.h>

#include <PolynomialForm.hpp>
#include <LexScanner.hpp>
#include <ParserLL1.hpp>

#include <array>

TEST(PolynomialForm, init)
{
    PolynomialForm pf(std::make_shared<SymbolTable>());

    EXPECT_EQ(pf.degree(), 0);
    EXPECT_EQ(pf.size(), 0);
}

TEST(PolynomialForm, init_error)
{
    EXPECT_THROW([] { PolynomialForm pf(nullptr); }(), std::invalid_argument);
}

TEST(PolynomialForm, operator_brackets)
{
    PolynomialForm pf(std::make_shared<SymbolTable>());

    pf[0].setRoot(AST::LeafNum::make(1));
    pf[2].setRoot(AST::LeafNum::make(2));

    EXPECT_EQ(pf.degree(), 2);
    EXPECT_EQ(pf.size(), 3);

    ast_num_t v;
    auto      exp_v = std::to_array({1, 0, 2});
    EXPECT_TRUE(pf[0].getRoot()->is_num());
    EXPECT_TRUE(AST::LeafNum::getValue(pf[0].getRoot(), v));
    EXPECT_EQ(v, exp_v[0]);
    EXPECT_TRUE(pf[1].getRoot() != nullptr);
    EXPECT_TRUE(pf[1].getRoot()->is_num());
    EXPECT_TRUE(AST::LeafNum::getValue(pf[1].getRoot(), v));
    EXPECT_TRUE(v.is_zero());
    EXPECT_TRUE(pf[2].getRoot()->is_num());
    EXPECT_TRUE(AST::LeafNum::getValue(pf[2].getRoot(), v));
    EXPECT_EQ(v, exp_v[2]);
}

TEST(PolynomialForm, analyze_null_tree)
{
    PolynomialForm pf(std::make_shared<SymbolTable>());
    EXPECT_FALSE(pf.analyze(nullptr, ""));
}

class TestPolynomialForm : public ::testing::TestWithParam<std::tuple<std::string, std::string, int, std::vector<mp_t>>>
{
public:
    const std::string       line   = std::get<0>(GetParam());
    const std::string       sym    = std::get<1>(GetParam());
    const int               degree = std::get<2>(GetParam());
    const std::vector<mp_t> coeffs = std::get<3>(GetParam());
};

TEST_P(TestPolynomialForm, analyze)
{
    LexScanner     scanner(std::make_unique<std::istringstream>(line.data()));
    ParserLL1      parser(scanner);
    PolynomialForm pf(std::make_shared<SymbolTable>());

    ASSERT_TRUE(parser.parse());

    AST& ast = parser.ast();
    ast.print();

    ASSERT_TRUE(pf.analyze(ast.getRoot(), sym));
    ASSERT_EQ(pf.degree(), degree);
    ASSERT_TRUE(pf.simplify());
    for (size_t i = 0; i < pf.size(); ++i)
    {
        ast_num_t v;

        ASSERT_TRUE(pf[i].getRoot()->is_num());
        ASSERT_TRUE(AST::LeafNum::getValue(pf[i].getRoot(), v));
        auto b = coeffs[i].str();
        auto a = v.str();
        EXPECT_EQ(mp_t(v), coeffs[i]);
    }
}

INSTANTIATE_TEST_SUITE_P(
    PolynomialFormTestSuite,
    TestPolynomialForm,
    ::testing::Values(
        std::make_tuple("x - 1", "x", 1, std::vector<mp_t>{-1, 1}),
        std::make_tuple("x + x + x^2 + x*x + 1 -10 + 7 + x^5", "x", 5, std::vector<mp_t>{-2, 2, 2, 0, 0, 1})

            ));

class TestPolynomialFormError : public ::testing::TestWithParam<std::tuple<std::string, std::string>>
{
public:
    const std::string line = std::get<0>(GetParam());
    const std::string sym  = std::get<1>(GetParam());
};

TEST_P(TestPolynomialFormError, _error)
{
    LexScanner     scanner(std::make_unique<std::istringstream>(line.data()));
    ParserLL1      parser(scanner);
    PolynomialForm pf(std::make_shared<SymbolTable>());

    ASSERT_TRUE(parser.parse());
    AST& ast = parser.ast();
    ast.print();

    ASSERT_FALSE(pf.analyze(ast.getRoot(), sym));
    ASSERT_EQ(pf.degree(), -1);
}

INSTANTIATE_TEST_SUITE_P(
    PolynomialFormTestSuite,
    TestPolynomialFormError,
    ::testing::Values(
        std::make_tuple("x - 1", "y")

            ));

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
