#include "LexScanner.hpp"
#include "ParserLL1.hpp"
#include "Interpreter.hpp"

#include <iostream>
#include <format>

int main()
{
    LexScanner lex_a(std::make_unique<std::stringstream>("a = 1.2 - .1"));
    LexScanner lex_b(std::make_unique<std::stringstream>("b = 2 + .1"));
    LexScanner lex_scanner0(std::make_unique<std::stringstream>("X1 = (10 * 1) + 5 - 5 / 1"));
    LexScanner lex_scanner(std::make_unique<std::stringstream>("10 + 2 - X1 * 1 / 2 + (a - b)"));
    // LexScanner  lex_scanner(std::make_unique<std::stringstream>("10 + 2 - 10 * 1 / 2 + (1.1 - 2.1)"));    // 12 - 5 + -1.0 => 6s

    ParserLL1 parser(lex_scanner);
    ParserLL1 p0(lex_scanner0);
    ParserLL1 pa(lex_a);
    ParserLL1 pb(lex_b);

    Interpreter interpreter;

    if (!pa.parse() || !pb.parse())
        return -1;

    if (!p0.parse())
        return -1;

    if (!parser.parse())
        return -1;

    p0.ast().print();
    std::cout << "\n";
    parser.ast().print();

    if (!interpreter.eval(pa.ast()) || !interpreter.eval(pb.ast()))
        return -1;

    if (!interpreter.eval(p0.ast()))
        return -1;

    if (!interpreter.eval(parser.ast()))
        return -1;

    std::cout << std::format("\n- lastValue: {}\n", interpreter.lastValue());

    for (const auto& [k, v] : interpreter.symbolTable())
    {
        std::cout << std::format("--| {} = {} |\n", k, v);
    }
    return 0;
}
