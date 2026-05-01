#pragma once

#include <memory>
#include <istream>

#include "Token.hpp"

class LexScanner
{
private:
    enum class eState
    {
        START = 0,
        SUM_OP,
        MUL_OP,
        PARENTHESES,
        NUM1,
        NUM2,
        SYMBOL1,
        SYMBOL2,
        ERROR,
    };

    std::unique_ptr<std::istream> m_pInput = nullptr;
    eState                        m_state  = eState::START;
    size_t                        m_pos    = 0;
    Token                         m_lastToken;
    char                          m_lookahead    = 0;
    bool                          m_hasLookahead = false;

    char peek_();
    char get_();
    void unget_(const char c);

    bool stateStart();
    bool stateNum();
    // etc...

public:
    LexScanner(std::unique_ptr<std::istream>& pInput);

    Token nextToken();

    inline Token lastToken() const noexcept;
};

inline Token LexScanner::lastToken() const noexcept
{
    return m_lastToken;
}
