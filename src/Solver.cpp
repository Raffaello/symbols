#include "Solver.hpp"

#include <iostream>
#include <format>

bool Solver::has_symbol_(const INode* node, const std::string_view symbol) const noexcept
{
    if (auto sym = dynamic_cast<const LeafSymbol*>(node))
    {
        if (sym->value == symbol)
            return true;
        else
            return false;
    }

    if (auto bin = dynamic_cast<const NodeBin*>(node))
    {
        bool l = has_symbol_(bin->l.get(), symbol);
        if (l)
            return true;

        return has_symbol_(bin->r.get(), symbol);
    }

    return false;
}

bool Solver::is_equation(const INode* node) const noexcept
{
    if (auto bin = dynamic_cast<const NodeBin*>(node))
        return bin->token.type == eTOKENS::EQUAL;

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Solver::solve(AST& ast, std::string for_symbol)
{
    if (!has_symbol_(ast.getRoot(), for_symbol))
    {
        std::cerr << std::format("Symbol to solve for '{}' not found!\n", for_symbol);
        return false;
    }

    if (!is_equation(ast.getRoot()))
    {
        std::cerr << std::format("ERROR: {} is not an equation!\n", ast.to_string());
        return false;
    }

    // TODO
    return false;
}
