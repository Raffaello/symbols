#pragma once

#include <string>

enum class eTOKENS
{
    ERROR = -1,
    NUM   = 0,
    SUM_OP,    // +|-
    MUL_OP,    // *|/
    LEFT_PARENTHESES,
    RIGHT_PARENTHESES,
    SYMBOL,
    END,
};

struct Token
{
    eTOKENS     type = eTOKENS::ERROR;
    std::string value;
};
