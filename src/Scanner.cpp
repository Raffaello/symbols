#include "Scanner.hpp"

#include <cctype>
#include <iostream>
#include <format>

// #include <stdexcept>

// bool isDigit(const char c)
// {
//     return
// }

// bool isOperator(const char c)
// {
//     return
// }

// bool isVariable(const char c)
// {
//     return std::isalpha(static_cast<unsigned int>(c));
// }

int Scanner::extractDigit_(const std::string_view line, const int start_pos)
{
    int  j   = start_pos + 1;
    bool dot = false;
    for (; j < line.size(); ++j)
    {
        const unsigned int c2 = line[j];
        if (!std::isdigit(c2))
        {
            if (c2 == '.')
            {
                if (!dot)
                {
                    dot = true;
                    continue;
                }
                else
                {
                    std::cerr << std::format("Error '.' at position: {}\n", j);
                    return -1;
                }
            }

            break;
        }
    }

    return j;
}

int Scanner::extractSymbol_(const std::string_view line, const int start_pos)
{
    int j = start_pos + 1;
    for (; j < line.size(); ++j)
    {
        const unsigned int c2 = line[j];
        if (!(std::isalpha(c2) || std::isdigit(c2) || c2 == '_'))
            break;
    }

    return j;
}

std::list<Token> Scanner::tokenize(const std::string_view line)
{
    std::list<Token> res;
    Token            t;

    for (int i = 0; i < line.size();)
    {
        const char         ch = line[i];
        const unsigned int c  = ch;

        // skip whitespaces
        if (std::isspace(c))
        {
            ++i;
            continue;
        }

        if (std::isdigit(c))
        {
            int j = extractDigit_(line, i);
            if (j == -1)
                goto TOKENIZE_ERROR;

            t.token = eTOKENS::NUM;
            t.value = line.substr(i, j - i);
            i       = j;
            res.push_back(t);
        }
        else if (ch == '+' || ch == '-')
        {
            t.token = eTOKENS::SUM_OP;
            t.value = ch;
            ++i;
            res.push_back(t);
        }
        else if (ch == '*' || ch == '/')
        {
            t.token = eTOKENS::MUL_OP;
            t.value = ch;
            ++i;
            res.push_back(t);
        }
        else if (ch == '(')

        {
            t.token = eTOKENS::LEFT_PARENTHESES;
            t.value = ch;
            ++i;
            res.push_back(t);
        }
        else if (ch == ')')
        {
            t.token = eTOKENS::RIGHT_PARENTHESES;
            t.value = ch;
            ++i;
            res.push_back(t);
        }
        else if (std::isalpha(c))
        {
            int j   = extractSymbol_(line, i);
            t.token = eTOKENS::SYMBOL;
            t.value = line.substr(i, j - i);
            i       = j;
            res.push_back(t);
        }
        else
        {
        TOKENIZE_ERROR:
            std::cerr << std::format("Unknown char: {} at pos: {}\n", ch, i);
            res.clear();
            break;
        }
    }

    return res;
}
