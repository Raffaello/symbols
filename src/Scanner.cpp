#include "Scanner.hpp"

#include <cctype>
#include <iostream>
#include <format>

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
            int j = i + 1;
            for (; j < line.size(); ++j)
            {
                const unsigned int c2 = line[j];
                if (!std::isdigit(c2))
                    break;
            }

            t.token = eTOKENS::DIGIT;
            t.value = line.substr(i, j - i);
            i       = j;
            res.push_back(t);
        }
        else if (ch == '+' || ch == '-' || ch == '*' || ch == '/')
        {
            t.token = eTOKENS::OPERATOR;
            t.value = ch;
            ++i;
            res.push_back(t);
        }
        else if (ch == '(' || ch == ')')
        {
            t.token = eTOKENS::PARENTHESES;
            t.value = ch;
            ++i;
            res.push_back(t);
        }
        else if (std::isalpha(c))
        {
            int j = i + 1;
            for (; j < line.size(); ++j)
            {
                const unsigned int c2 = line[j];
                if (!(std::isalpha(c2) || std::isdigit(c2) || c2 == '_'))
                    break;
            }

            t.token = eTOKENS::SYMBOL;
            t.value = line.substr(i, j - i);
            i       = j;
            res.push_back(t);
        }
        else
        {
            std::cerr << std::format("Unknown char: {} at pos: {}\n", ch, i);
            res.clear();
            break;
        }
    }

    return res;
}
