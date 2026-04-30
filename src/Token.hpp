#pragma once

#include <string>

enum class eTOKENS
{
    UNKNOWN = -1,
    NUM     = 0,
    SUM_OP,    // +|-
    MUL_OP,    // *|/
    LEFT_PARENTHESES,
    RIGHT_PARENTHESES,
    SYMBOL,
};

struct Token
{
    // public:
    eTOKENS     token = eTOKENS::UNKNOWN;
    std::string value;
};
