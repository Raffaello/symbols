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
public:
    // bool isDigit(const char c);
    // bool isOperator(const char c);
    // bool isVariable(const char c);

    std::list<Token> tokenize(const std::string_view line);
};
