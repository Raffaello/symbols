#pragma once

#include "AST.hpp"

/**
 * @brief Solver for 1 variable e.g. -x = x+2 => -2x=2 => x = -1
 * TODO: for now just 1 variable is fine.
 * TODO: should have symbol table? no
 * TODO: later try to solve this: ax + b = 0 => x = b/a [b is not know, a is not known] so are just really symbols without a value
 */
class Solver
{
private:
    bool has_symbol_(const INode* node, const std::string_view symbol) const noexcept;
    bool is_equation(const INode* node) const noexcept;

public:
    bool solve(AST& ast, std::string for_symbol);
};
