#pragma once

#include <memory>
#include <istream>
#include <sstream>
#include <cctype>

#include "Token.hpp"

/**
 * Alphabet: alphadigits | _ | + | - | * | / | = | ' '
 * whitespaces are just skipped, remaining in the same state
 *
 * TODO: add parsing '\n' and EOF as a END token (or also as NEWLINE) also the end of statement';'
 *       Track the line number while scanning
 *       modify the parser to deal with "end of statement"
 */
class LexScanner
{
private:
    enum class eState
    {
        START = 0,
        SUM_OP_PLUS,
        SUM_OP_MINUS,
        MUL_OP_MUL,
        MUL_OP_DIV,
        PARENTHESES_LEFT,
        PARENTHESES_RIGHT,
        EQUAL,
        INT,
        PRE_REAL,
        REAL,
        SYMBOL,
        ERROR,
    };

    std::unique_ptr<std::istream> m_pInput = nullptr;
    eState                        m_state  = eState::START;
    size_t                        m_pos    = 0;
    Token                         m_lastToken;
    std::ostringstream            m_curTokenValue;
    bool                          m_eof = false;    // TODO: move in as a state

    char peek_();
    char get_();
    void unget_(const char c);

    inline bool isDigit_(const char c) const noexcept;
    inline bool isAlpha_(const char c) const noexcept;
    inline bool isAlNum_(const char c) const noexcept;
    inline bool isSpace_(const char c) const noexcept;

    void stateStart_(const char c);
    void statePreReal_(const char c);

    // Final states return true if it isn't over yet
    bool stateInt_(const char c);
    bool stateReal_(const char c);
    bool stateSymbol_(const char c);

    bool stateFinal_(const eTOKENS type);

public:
    LexScanner(std::unique_ptr<std::istream> pInput);

    void setInput(std::unique_ptr<std::istream> pInput);
    bool next();

    inline Token lastToken() const noexcept;
};

inline Token LexScanner::lastToken() const noexcept
{
    return m_lastToken;
}

inline bool LexScanner::isDigit_(const char c) const noexcept
{
    return std::isdigit(static_cast<unsigned char>(c));
}

inline bool LexScanner::isAlpha_(const char c) const noexcept
{
    return std::isalpha(static_cast<unsigned char>(c));
}

inline bool LexScanner::isAlNum_(const char c) const noexcept
{
    return std::isalnum(static_cast<unsigned char>(c));
}

inline bool LexScanner::isSpace_(const char c) const noexcept
{
    return std::isspace(static_cast<unsigned char>(c));
}
