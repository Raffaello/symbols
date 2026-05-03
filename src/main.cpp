#include "LexScanner.hpp"
#include "ParserLL1.hpp"

#include <iostream>
#include <format>

int main()
{
    LexScanner lex_scanner(std::make_unique<std::stringstream>("10 + 2 - X1 * 1 / 2 + (a - b)"));
    ParserLL1  parser(lex_scanner);

    if (!parser.parse())
        return -1;

    parser.ast().print();
    return 0;
}
