#include "LexScanner.hpp"

#include <iostream>
#include <format>
#include <string>
#include <sstream>

LexScanner::LexScanner(std::unique_ptr<std::istream> pInput)
{
    setInput(std::move(pInput));
}

void LexScanner::setInput(std::unique_ptr<std::istream> pInput)
{
    m_pInput    = std::move(pInput);
    m_eof       = false;
    m_state     = eState::START;
    m_lastToken = Token();
    m_curTokenValue.clear();
    m_curTokenValue.str("");
    m_pos = 0;
}

char LexScanner::peek_()
{
    return m_pInput->peek();
}

char LexScanner::get_()
{
    const char c = m_pInput->get();

    // nothing more to read
    if (m_pInput->eof())
    {
        m_eof = true;
        return -1;    // EOF;
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
    m_pInput->unget();
    --m_pos;
}

void LexScanner::stateStart_(const char c)
{
    if (c == '.')
        m_state = eState::PRE_REAL;
    else if (isDigit_(c))
        m_state = eState::INT;
    else if (c == '_' || isAlpha_(c))
        m_state = eState::SYMBOL;
    else if (c == '=')
        m_state = eState::EQUAL;
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
    else if (isSpace_(c))
        return;
    else
    {
        m_state           = eState::ERROR;
        m_lastToken.type  = eTOKENS::ERROR;
        m_lastToken.value = std::format("unknown char '{}' in '{}' at pos: {}'", c, m_curTokenValue.str(), m_pos);
    }

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
    else if (isDigit_(c))
    {
        m_curTokenValue << c;
        return true;
    }

    unget_(c);
    return false;
}

void LexScanner::statePreReal_(const char c)
{
    if (isDigit_(c))
        m_state = eState::REAL;
    else
        m_state = eState::ERROR;

    m_curTokenValue << c;
}

bool LexScanner::stateReal_(const char c)
{
    if (isDigit_(c))
    {
        m_curTokenValue << c;
        return true;
    }
    else if (c == '.')
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
    if (c == '_' || isAlNum_(c))
    {
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

bool LexScanner::next()
{
    if (m_pInput == nullptr)
        return false;

    if (m_eof)
        return false;

    if (m_state == eState::ERROR)
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
            std::cerr << std::format("ERROR: {}\n", m_lastToken.value);
            return false;
        }
        case START:
            stateStart_(c);
            break;
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
        case EQUAL:
            unget_(c);
            return stateFinal_(eTOKENS::EQUAL);
        case INT:
            if (!stateInt_(c))
                return stateFinal_(eTOKENS::NUM);
            break;
        case PRE_REAL:
            statePreReal_(c);
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
