#include "LexScanner.hpp"

#include <iostream>
#include <format>
#include <string>
#include <cctype>
#include <sstream>

LexScanner::LexScanner(std::unique_ptr<std::istream>& pInput) : m_pInput(std::move(pInput))
{
}

char LexScanner::peek_()
{
    if (m_hasLookahead)
        return m_lookahead;

    return m_pInput->peek();
}

char LexScanner::get_()
{
    if (m_hasLookahead)
    {
        m_hasLookahead = false;
        return m_hasLookahead;
    }

    const char c = m_pInput->get();

    // check for errors
    if (m_pInput->bad() || m_pInput->fail())
    {
        const std::string e = std::format("unable to read at pos: {}", m_pos);
        std::cerr << e;
        throw std::runtime_error(e);
    }

    ++m_pos;

    return c;
}

void LexScanner::unget_(const char c)
{
    if (m_hasLookahead)
        throw std::runtime_error("already unget once...");

    m_lookahead    = c;
    m_hasLookahead = true;
    --m_pos;
}

Token LexScanner::nextToken()
{
    // TODO: use the state instead
    //       flat the loop
    //       return bool and use lastToken() to retrive the last token
    //       set the state to start when a succesful token has been found
    //       otherwise Error
    //       when is on END it won't anymore re-start the state.

    // Equivalent to DFA State END or ERROR
    if (m_lastToken.type == eTOKENS::END || m_lastToken.type == eTOKENS::ERROR)
        return lastToken();

    do
    {
        const char c = get_();

        // nothing more to read
        if (m_pInput->eof())
        {
            m_lastToken.type  = eTOKENS::END;
            m_lastToken.value = "";
            return lastToken();
        }

        // skip whitespaces
        if (std::isspace(c))
            continue;

        // SUM_OP
        if (c == '+' || c == '-')
        {
            m_lastToken.type  = eTOKENS::SUM_OP;
            m_lastToken.value = c;

            return lastToken();
        }
        // MUL_OP
        else if (c == '*' || c == '/')
        {
            m_lastToken.type  = eTOKENS::MUL_OP;
            m_lastToken.value = c;

            return lastToken();
        }
        // PARENTHESES
        else if (c == '(')
        {
            m_lastToken.type  = eTOKENS::LEFT_PARENTHESES;
            m_lastToken.value = c;

            return lastToken();
        }
        else if (c == ')')
        {
            m_lastToken.type  = eTOKENS::RIGHT_PARENTHESES;
            m_lastToken.value = c;

            return lastToken();
        }
        // NUM
        // TODO: need to flat the loop and store the current state of the DFA
        else if (c == '.' || std::isdigit(c))
        {
            char               c2 = c;
            std::ostringstream buf;
            do
            {
                buf << c2;
                c2 = get_();
            }
            while (std::isdigit(c2));
            unget_(c2);

            m_lastToken.type  = eTOKENS::NUM;
            m_lastToken.value = buf.str();
            return lastToken();
        }
        // SYMBOL
        else if (c == '_' || std::isalpha(c))
        {
            char               c2 = c;
            std::ostringstream buf;

            do
            {
                buf << c2;
                c2 = get_();
            }
            while (c2 == '_' || std::isalnum(c));
            unget_(c2);

            m_lastToken.type  = eTOKENS::SYMBOL;
            m_lastToken.value = buf.str();
            return lastToken();
        }
        else
        {
            m_lastToken.type  = eTOKENS::ERROR;
            m_lastToken.value = "";
            return lastToken();
        }
    }
    while (true);
}
