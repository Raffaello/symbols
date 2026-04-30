#include "Scanner.hpp"

int main()
{
    Scanner scanner;

    const char* line = "10 + 2 - X1 * 1 / 2 + (a - b)";

    auto res = scanner.tokenize(line);


    return 0;
}
