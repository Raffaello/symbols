#pragma once

#include <memory>
#include <istream>
#include <sstream>
#include <cctype>
#include <cstdint>

#include "Token.hpp"

/**
 * Alphabet: alphadigits | _ | + | - | * | / | = | ' '
 * whitespaces are just skipped, remaining in the same state
 *
 * TODO: [optional] add parsing '\n' (or also as NEWLINE) also the end of statement';'
 *       Track the line number while scanning
 */
class LexScanner
{
private:
    static constexpr uint8_t EOF_ = 0xFF;
    enum class eState
    {
        START = 0,
        SUM_OP_PLUS,
        SUM_OP_MINUS,
        MUL_OP_MUL,
        MUL_OP_DIV,
        POW_OP,
        COMMA_OP,
        PARENTHESES_LEFT,
        PARENTHESES_RIGHT,
        EQUAL,
        INT,
        PRE_REAL,
        REAL,
        SYMBOL,
        ERROR,
        END,
    };

    std::unique_ptr<std::istream> m_pInput = nullptr;
    eState                        m_state  = eState::START;
    size_t                        m_pos    = 0;
    Token                         m_lastToken;
    std::ostringstream            m_curTokenValue;

    void clearCurTokenValue_() noexcept;

    uint8_t peek_();
    uint8_t get_();
    void    unget_(const uint8_t c);

    inline bool isDigit_(const uint8_t c) const noexcept;
    inline bool isAlpha_(const uint8_t c) const noexcept;
    inline bool isAlNum_(const uint8_t c) const noexcept;
    inline bool isSpace_(const uint8_t c) const noexcept;

    void stateStart_(const uint8_t c);
    void statePreReal_(const uint8_t c);

    // Final states return true if it isn't over yet
    bool stateInt_(const uint8_t c);
    bool stateReal_(const uint8_t c);
    bool stateSymbol_(const uint8_t c);

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

inline bool LexScanner::isDigit_(const uint8_t c) const noexcept
{
    return std::isdigit(static_cast<uint8_t>(c));
}

inline bool LexScanner::isAlpha_(const uint8_t c) const noexcept
{
    return std::isalpha(static_cast<uint8_t>(c));
}

inline bool LexScanner::isAlNum_(const uint8_t c) const noexcept
{
    return std::isalnum(static_cast<uint8_t>(c));
}

inline bool LexScanner::isSpace_(const uint8_t c) const noexcept
{
    return std::isspace(static_cast<uint8_t>(c));
}
