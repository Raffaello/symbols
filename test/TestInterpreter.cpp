#include <gtest/gtest.h>

#include <Interpreter.hpp>
#include <array>

class TestInterpreter : public ::testing::TestWithParam<std::tuple<std::string_view>>
{
public:
    const std::string_view line = std::get<0>(GetParam());
};

TEST_P(TestInterpreter, parser)
{
    // LexScanner scanner(std::make_unique<std::istringstream>(line.data()));
    // ParserLL1  parser(scanner);

    // TODO
    FAIL();
}

INSTANTIATE_TEST_SUITE_P(
    InterpreterTestSuite,
    TestInterpreter,
    ::testing::Values(
        std::make_tuple("1")));

class TestInterpreterError : public ::testing::TestWithParam<std::string_view>
{
public:
    const std::string_view line = GetParam();
};

TEST_P(TestInterpreterError, parser_error)
{
    // LexScanner scanner(std::make_unique<std::istringstream>(line.data()));
    // ParserLL1  parser(scanner);

    // TODO
    FAIL();
}

INSTANTIATE_TEST_SUITE_P(
    InterpreterTestSuite,
    TestInterpreterError,
    ::testing::Values(
        ""));

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
