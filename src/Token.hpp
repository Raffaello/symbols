#pragma once

#include <string>

/**
 * @brief TOD: all the operator could be reduced to 1 token: OPERATOR
 *
 */
enum class eTOKENS
{
    ERROR = -1,
    NUM   = 0,    // it could be expanded into INT and REAL
    SUM_OP,       // +|-
    MUL_OP,       // *|/
    POW_OP,       // ^
    COMMA_OP,     // ,
    LEFT_PARENTHESES,
    RIGHT_PARENTHESES,
    EQUAL,
    SYMBOL,
    END,
};

struct Token
{
    eTOKENS     type = eTOKENS::ERROR;
    std::string value;
};
