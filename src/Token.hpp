#pragma once

#include <string>

enum class eTOKENS
{
    UNKNOWN = -1,
    DIGIT   = 0,
    OPERATOR,
    PARENTHESES,
    SYMBOL,
};

struct Token
{
    // public:
    eTOKENS     token;
    std::string value;
};
