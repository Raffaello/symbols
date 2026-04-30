#pragma once

#include <string_view>
#include <list>

#include "Token.hpp"

/**
 * @brief
 *
 *
 *
 * TOKENS: [digits],[letters as variables] + - * / ^
 *
 */
class Scanner
{
private:
    // bool isDigit(const char c);
    // bool isOperator(const char c);
    // bool isVariable(const char c);

    int extractDigit_(const std::string_view line, const int start_pos);
    int extractSymbol_(const std::string_view line, const int start_pos);


public:
    std::list<Token> tokenize(const std::string_view line);
};
