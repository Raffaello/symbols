#pragma once

#include <string>

enum class eTOKENS
{
    ERROR = -1,
    NUM   = 0,    // it could be expanded into INT and REAL
    SUM_OP,       // +|-
    MUL_OP,       // *|/
    LEFT_PARENTHESES,
    RIGHT_PARENTHESES,
    EQUAL,
    SYMBOL,
    // END, // TODO: this would be better if returned by the lexer as last token after finished tokenizing.
};

struct Token
{
    eTOKENS     type = eTOKENS::ERROR;
    std::string value;
};
