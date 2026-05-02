#pragma once

#include <memory>
#include <istream>
#include <sstream>

#include "Token.hpp"

/**
 * Alphabet: alphadigits | _ | + | - | * | / | = | ' '
 * whitespaces are just skipped, remaining in the same state
 */
class LexScanner
{
private:
    // enum class eAlphabet
    // {
    //     LETTERS,
    //     NUMBERS,
    //     DOT,
    //     OPERATORS,
    //     PARENTHESES, // symbols?
    //     WHITE_SPACE,
    //     SYMBOLS,
    // };

    enum class eState
    {
        START = 0,
        END,
        SUM_OP_PLUS,
        SUM_OP_MINUS,
        MUL_OP_MUL,
        MUL_OP_DIV,
        PARENTHESES_LEFT,
        PARENTHESES_RIGHT,
        INT,
        REAL,
        SYMBOL,
        ERROR,
    };

    std::unique_ptr<std::istream> m_pInput = nullptr;
    eState                        m_state  = eState::START;
    size_t                        m_pos    = 0;
    Token                         m_lastToken;
    std::ostringstream            m_curTokenValue;
    char                          m_lookahead    = 0;
    bool                          m_hasLookahead = false;
    bool                          m_eof          = false;

    char peek_();
    char get_();
    void unget_(const char c);

    void stateStart_(const char c);
    bool stateInt_(const char c);
    bool stateReal_(const char c);
    bool stateSymbol_(const char c);
    // etc...

    bool stateFinal_(const eTOKENS type);

public:
    LexScanner(std::unique_ptr<std::istream> pInput);

    Token nextToken();
    bool  next();

    inline Token lastToken() const noexcept;
};

inline Token LexScanner::lastToken() const noexcept
{
    return m_lastToken;
}
