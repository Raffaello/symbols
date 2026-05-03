#include "LexScanner.hpp"
#include "ParserLL1.hpp"
#include "Interpreter.hpp"

#include <iostream>
#include <format>

int main()
{
    // LexScanner  lex_scanner(std::make_unique<std::stringstream>("10 + 2 - X1 * 1 / 2 + (a - b)"));
    LexScanner  lex_scanner(std::make_unique<std::stringstream>("10 + 2 - 10 * 1 / 2 + (1.1 - 2.1)"));    // 12 - 5 + -1.0 => 6s
    ParserLL1   parser(lex_scanner);
    Interpreter interpreter;

    if (!parser.parse())
        return -1;

    parser.ast().print();

    if (!interpreter.eval(parser.ast()))
        return -1;

    std::cout << std::format("- lastValue: {}\n", interpreter.lastValue());
    return 0;
}
