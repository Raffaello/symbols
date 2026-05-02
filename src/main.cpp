#include "LexScanner.hpp"

#include <iostream>
#include <format>

int main()
{
    LexScanner lex_scanner(std::make_unique<std::stringstream>("10 + 2 - X1 * 1 / 2 + (a - b)"));

    while (lex_scanner.next())
    {
        std::cout << std::format("Token: type: {}, value={}\n", (int) lex_scanner.lastToken().type, lex_scanner.lastToken().value);
    }

    return 0;
}
