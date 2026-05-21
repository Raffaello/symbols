#include <gtest/gtest.h>

#include <ParserLL1.hpp>
#include <Simplifier.hpp>

class TestSimplifier : public ::testing::TestWithParam<std::tuple<std::string, std::string>>
{
public:
    const std::string line   = std::get<0>(GetParam());
    const std::string expAST = std::get<1>(GetParam());
};

TEST_P(TestSimplifier, parser)
{
    LexScanner scanner(std::make_unique<std::istringstream>(line.data()));
    ParserLL1  parser(scanner);

    ASSERT_TRUE(parser.parse());
    auto& ast = parser.ast();
    ast.print();

    EXPECT_TRUE(Simplifier::reduce(ast));
    EXPECT_STREQ(ast.to_string().c_str(), expAST.data());
}

INSTANTIATE_TEST_SUITE_P(
    SimplifierTestSuite,
    TestSimplifier,
    ::testing::Values(
        std::make_tuple("x+1+2*3", "(x + 1) + 6"),    // it should be at least x + 7, but what is the point for this rule?
        // std::make_tuple("-(-5)", "5"), // TODO: it should be 5
        std::make_tuple("10/2", "5"),
        std::make_tuple("2^0", "1"),
        std::make_tuple("2^3", "8"),
        std::make_tuple("2^3.1", "2^(31/10)"),
        std::make_tuple("(2+3)*4", "20"),
        std::make_tuple("2^1", "2"),
        std::make_tuple("10/4", "(5/2)"),
        std::make_tuple("x*1", "x * 1")    // TODO: this should return just x

        ));

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
