#include "Scanner.hpp"

#include <cctype>
#include <iostream>
#include <format>

// #include <stdexcept>

bool Scanner::isDigit_(const char c)
{
    return std::isdigit(c) || c == '.';
}

bool Scanner::isSymbol_(const char c)
{
    return std::isalpha(c) || c == '_';
}

bool Scanner::isDelimiter(const char c)
{
    return std::isspace(c) || c == ')' || c == '+' || c == '-' || c == '*' || c == '/';
}

// bool isOperator(const char c)
// {
//     return
// }


int Scanner::extractDigit_(const std::string_view line, const int start_pos, bool dot)
{
    int j = start_pos + 1;
    for (; j < line.size(); ++j)
    {
        const char c = line[j];

        if (!std::isdigit(c))
        {
            if (c == '.')
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
            else if (isDelimiter(c))
            {
                // ++j;
                break;
            }

            std::cerr << std::format("Error '{}' at position: {}\n", c, j);
            return -1;
        }
    }

    return j;
}

int Scanner::extractSymbol_(const std::string_view line, const int start_pos)
{
    int j = start_pos + 1;
    for (; j < line.size(); ++j)
    {
        const char c = line[j];
        if (!(isSymbol_(c) || std::isdigit(c)))
        {
            if (isDelimiter(c))
                break;
            else
            {
                std::cerr << std::format("Error '{}' at position: {}\n", c, j);
                return -1;
            }
        }
    }

    return j;
}

std::list<Token> Scanner::tokenize(const std::string_view line)
{
    std::list<Token> res;
    Token            t;

    for (int i = 0; i < line.size();)
    {
        const char c = line[i];

        // skip whitespaces
        if (std::isspace(c))
        {
            ++i;
            continue;
        }

        if (isDigit_(c))
        {
            int j = extractDigit_(line, i, c == '.');
            if (j == -1)
                goto TOKENIZE_ERROR;

            t.type  = eTOKENS::NUM;
            t.value = line.substr(i, j - i);
            i       = j;
            res.push_back(t);
        }
        else if (c == '+' || c == '-')
        {
            t.type  = eTOKENS::SUM_OP;
            t.value = c;
            ++i;
            res.push_back(t);
        }
        else if (c == '*' || c == '/')
        {
            t.type  = eTOKENS::MUL_OP;
            t.value = c;
            ++i;
            res.push_back(t);
        }
        else if (c == '(')
        {
            t.type  = eTOKENS::LEFT_PARENTHESES;
            t.value = c;
            ++i;
            res.push_back(t);
        }
        else if (c == ')')
        {
            t.type  = eTOKENS::RIGHT_PARENTHESES;
            t.value = c;
            ++i;
            res.push_back(t);
        }
        else if (isSymbol_(c))
        {
            int j = extractSymbol_(line, i);
            if (j == -1)
                goto TOKENIZE_ERROR;

            t.type  = eTOKENS::SYMBOL;
            t.value = line.substr(i, j - i);
            i       = j;
            res.push_back(t);
        }
        else
        {
        TOKENIZE_ERROR:
            std::cerr << std::format("Unknown char: {} at pos: {}\n", c, i);
            res.clear();
            break;
        }
    }

    return res;
}
