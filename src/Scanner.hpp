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
 * TODO: create thre NFA -> DFA first. otherwise is just messy
 */
class Scanner
{
private:
    bool isDigit_(const char c);
    bool isSymbol_(const char c);
    bool isDelimiter(const char c);
    // bool isOperator(const char c);

    int extractDigit_(const std::string_view line, const int start_pos, bool dot);
    int extractSymbol_(const std::string_view line, const int start_pos);

public:
    std::list<Token> tokenize(const std::string_view line);
};
