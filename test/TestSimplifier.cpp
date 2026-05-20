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
        std::make_tuple("x+1+2*3", "(x + 1) + 6")    // it should be at least x + 7, but what is the point for this rule?

        ));

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
