#pragma once

#include "AST.hpp"
#include "SymbolTable.hpp"
#include "PolynomialForm.hpp"

#include <sstream>
#include <string>
#include <memory>
#include <string_view>

/**
 * @brief Solver for 1 variable e.g. -x = x+2 => -2x=2 => x = -1
 *
 * TODO: for now just 1 variable is fine.
 * TODO: later try to solve this: ax + b = 0 => x = b/a [b is not know, a is not known] so are just really symbols without a value
 */
class Solver
{
public:
private:
    std::shared_ptr<SymbolTable> m_pSymbolTable;    // TODO: not used yet as it is too simple:
                                                    // can be used only for known symbols for substitution into numbers
                                                    // as a pre step changing the AST, (or when navigating it)

    std::string m_solution;

    // TODO: for_symbol would be better as a INode (LeafSymbol type), right?
    bool solve_equation_(AST::INode* node, const std::string_view for_symbol);

public:
    Solver(const std::shared_ptr<SymbolTable>& pSymbolTable);

    bool solve(AST& ast, const std::string_view for_symbol);

    inline std::string solution() const noexcept;
};

inline std::string Solver::solution() const noexcept
{
    return m_solution;
}
