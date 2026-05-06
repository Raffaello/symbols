#pragma once

#include "AST.hpp"

#include <sstream>
#include <string>

/**
 * @brief Solver for 1 variable e.g. -x = x+2 => -2x=2 => x = -1
 *
 * TODO: for now just 1 variable is fine.
 * TODO: should have symbol table? no
 * TODO: later try to solve this: ax + b = 0 => x = b/a [b is not know, a is not known] so are just really symbols without a value
 */
class Solver
{
private:
    // std::stringstream m_solution;

    bool has_symbol_(const INode* node, const std::string_view symbol) const noexcept;

    bool is_equation_(const INode* node) const noexcept;
    bool is_expr_(const INode* node) const noexcept;
    bool is_unary_(const INode* node) const noexcept;
    bool is_symbol_(const INode* node) const noexcept;
    bool is_symbol_(const INode* node, const std::string_view symbol) const noexcept;
    bool is_num_(const INode* node) const noexcept;


    bool                solve_equation_(INode* node, const std::string_view for_symbol);
    std::optional<bool> solve_expr_(std::unique_ptr<INode>& node, const std::string_view for_symbol);
    bool                solve_unary_(std::unique_ptr<INode>& node, const std::string_view for_symbol);

public:
    bool solve(AST& ast, const std::string_view for_symbol);

    // inline std::string solution() const noexcept;
};

// inline std::string Solver::solution() const noexcept
// {
//     return m_solution.str();
// }
