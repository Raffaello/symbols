#pragma once

#include "Token.hpp"

#include <list>

class Parser
{
private:
    void expr_();
    void term_();
    void factor_();

public:
    bool parse(const std::list<Token>& tokens);
};
