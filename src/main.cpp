#include "LexScanner.hpp"
#include "ParserLL1.hpp"
#include "Interpreter.hpp"

#include <iostream>
#include <format>

bool eval(ParserLL1& parser, Interpreter& interpreter)
{
    if (!parser.parse())
        return false;

    if (!interpreter.eval(parser.ast()))
        return false;

    parser.ast().print();
    std::cout << "\n";
    std::cout << std::format("\n- lastValue: {}\n", interpreter.lastValue());

    return true;
}

int main()
{
    {
        LexScanner  lex(std::make_unique<std::stringstream>("a = 1.2 - 1"));    // 0.2
        ParserLL1   parser(lex);
        Interpreter interpreter;

        if (!eval(parser, interpreter))
            return -1;

        lex.setInput(std::make_unique<std::stringstream>("b = 2 + .1"));    // 2.1
        if (!eval(parser, interpreter))
            return -1;

        lex.setInput(std::make_unique<std::stringstream>("X1 = (10 * 1) + 5 - 5 / 1"));    // 10
        if (!eval(parser, interpreter))
            return -1;

        lex.setInput(std::make_unique<std::stringstream>("10 + 2 - X1 * 1 / 2 + (a - b)"));    // 12 - 5 - 1.9 = 5.1
        if (!eval(parser, interpreter))
            return -1;

        for (const auto& [k, v] : interpreter.symbolTable())
            std::cout << std::format("--| {} = {} |\n", k, v);
    }

    // REPL test
    LexScanner  lex(std::make_unique<std::stringstream>(""));
    ParserLL1   parser(lex);
    Interpreter interpreter;

    std::cout << std::format("Symbols REPL v0.1.0\n");
    std::cout << std::format("Press Ctrl-D to exit\n\n");
    while (true)
    {
        std::string input;

        std::cout << "\n$> ";
        std::getline(std::cin, input);
        lex.setInput(std::make_unique<std::stringstream>(input));
        if (!parser.parse())
            continue;

        if (!interpreter.eval(parser.ast()))
            continue;

        std::cout << std::format("\n$? := {}\n", interpreter.lastValue());
    }

    return 0;
}
