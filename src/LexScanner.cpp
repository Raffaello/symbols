#include "LexScanner.hpp"

#include <iostream>
#include <format>
#include <string>
#include <cctype>
#include <sstream>

LexScanner::LexScanner(std::unique_ptr<std::istream> pInput) : m_pInput(std::move(pInput))
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
        return m_lookahead;
    }

    const char c = m_pInput->get();

    // nothing more to read
    if (m_pInput->eof())
    {
        m_eof = true;
        return EOF;
    }

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

void LexScanner::stateStart_(const char c)
{
    if (c == '.')
        m_state = eState::REAL;
    else if (std::isdigit(c))
        m_state = eState::INT;
    else if (c == '_' || std::isalpha(c))
        m_state = eState::SYMBOL;
    else if (c == '(')
        m_state = eState::PARENTHESES_LEFT;
    else if (c == ')')
        m_state = eState::PARENTHESES_RIGHT;
    else if (c == '+')
        m_state = eState::SUM_OP_PLUS;
    else if (c == '-')
        m_state = eState::SUM_OP_MINUS;
    else if (c == '*')
        m_state = eState::MUL_OP_MUL;
    else if (c == '/')
        m_state = eState::MUL_OP_DIV;
    else if (std::isspace(c))
        return;
    else
        m_state = eState::ERROR;

    m_curTokenValue << c;
}

bool LexScanner::stateInt_(const char c)
{
    if (c == '.')
    {
        m_curTokenValue << c;
        m_state = eState::REAL;
        return true;
    }
    else if (std::isdigit(c))
    {
        m_curTokenValue << c;
        return true;
    }
    else if (std::isspace(c))
        return false;
    else if (std::isalpha(c) || c == '(' || c == '_')
    {
        m_state = eState::ERROR;
        m_curTokenValue << c;
        return true;
    }

    unget_(c);
    return false;
}

bool LexScanner::stateReal_(const char c)
{
    if (std::isdigit(c))
    {
        m_curTokenValue << c;
        return true;
    }
    else if (std::isspace(c))
        return false;
    else if (std::isalpha(c) || c == '(' || c == '.' || c == '_')
    {
        m_state = eState::ERROR;
        m_curTokenValue << c;
        return true;
    }

    unget_(c);
    return false;
}

bool LexScanner::stateSymbol_(const char c)
{
    if (c == '_' || std::isalnum(c))
    {
        m_curTokenValue << c;
        return true;
    }
    else if (std::isspace(c))
        return false;
    else if (c == '(' || c == '.')
    {
        m_state = eState::ERROR;
        m_curTokenValue << c;
        return true;
    }

    unget_(c);
    return false;
}

bool LexScanner::stateFinal_(const eTOKENS type)
{
    m_lastToken.type  = type;
    m_lastToken.value = m_curTokenValue.str();
    return true;
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

        // // nothing more to read
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

bool LexScanner::next()
{
    if (m_eof)
        return false;

    if (m_state == eState::ERROR || m_state == eState::END)
        return false;

    m_curTokenValue.str("");
    m_curTokenValue.clear();
    m_state = eState::START;

    do
    {
        const char c = get_();
        switch (m_state)
        {
            using enum eState;

        default:
            [[fallthrough]];
        case ERROR:
        {
            m_lastToken.type  = eTOKENS::ERROR;
            m_lastToken.value = std::format("unknown char {} in {}", c, m_curTokenValue.str());
            std::cerr << m_lastToken.value;
            return false;
        }
        case START:
            stateStart_(c);
            break;
        case END:
        {
            stateFinal_(eTOKENS::END);
            return false;
        }
        case SUM_OP_PLUS:
            [[fallthrough]];
        case SUM_OP_MINUS:
            unget_(c);
            return stateFinal_(eTOKENS::SUM_OP);
        case MUL_OP_MUL:
            [[fallthrough]];
        case MUL_OP_DIV:
            unget_(c);
            return stateFinal_(eTOKENS::MUL_OP);
        case PARENTHESES_LEFT:
            unget_(c);
            return stateFinal_(eTOKENS::LEFT_PARENTHESES);
        case PARENTHESES_RIGHT:
            unget_(c);
            return stateFinal_(eTOKENS::RIGHT_PARENTHESES);
        case INT:
            if (!stateInt_(c))
                return stateFinal_(eTOKENS::NUM);
            break;
        case REAL:
            if (!stateReal_(c))
                return stateFinal_(eTOKENS::NUM);
            break;
        case SYMBOL:
            if (!stateSymbol_(c))
                return stateFinal_(eTOKENS::SYMBOL);
            break;
        }
    }
    while (!m_eof);

    m_lastToken.type  = eTOKENS::ERROR;
    m_lastToken.value = std::format("unable to parse {}", m_curTokenValue.str());
    std::cerr << m_lastToken.value;
    return false;
}
