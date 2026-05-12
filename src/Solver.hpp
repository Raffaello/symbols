#pragma once

#include "AST.hpp"
#include "SymbolTable.hpp"

#include <sstream>
#include <string>
#include <vector>
#include <memory>

/**
 * @brief Solver for 1 variable e.g. -x = x+2 => -2x=2 => x = -1
 *
 * TODO: for now just 1 variable is fine.
 * TODO: later try to solve this: ax + b = 0 => x = b/a [b is not know, a is not known] so are just really symbols without a value
 */
class Solver
{
public:
    // TODO: create a compute degree function here, so doesn't have to do every single time in the solver,
    //       also the operation to add a coeffs, if it has to push it back or update what is already there (operator[])
    struct PolynomialForm
    {
        // private:
        int                 degree;
        std::vector<double> coeffs;    // coeffs are stored in reverse order (c + bx + ax^2 + ...)

                                       // public:
        double& operator[](size_t index)
        {
            if (coeffs.size() < index + 1)
                coeffs.resize(index + 1);

            return coeffs[index];
        }

        // TODO: later
        // inline degree()
        // {

        // }
    };

private:
    std::shared_ptr<SymbolTable>
        m_pSymbolTable;    // TODO: not used yet as it is too simple:
                           // can be used only for known symbols for substitution into numbers
                           // as a pre step changing the AST, (or when navigating it)

    std::string m_solution;

    PolynomialForm analyze_poly_(const AST::INode* node, std::string_view symbol);
    bool           collect_poly_(const AST::INode* node, PolynomialForm& pf, std::string_view symbol);
    bool           collect_poly_num_(const AST::INode* node, PolynomialForm& pf);
    bool           collect_poly_sym_(const AST::INode* node, PolynomialForm& pf, std::string_view symbol);
    bool           collect_poly_uny_(const AST::INode* node, PolynomialForm& pf, std::string_view symbol);
    bool           collect_poly_expr_(const AST::INode* node, PolynomialForm& pf, std::string_view symbol);

    bool has_symbol_(const AST::INode* node, const std::string_view symbol) const noexcept;

    static bool is_equation_(const AST::INode* node);
    static bool is_expr_(const AST::INode* node);
    static bool is_unary_(const AST::INode* node);
    static bool is_symbol_(const AST::INode* node);
    static bool is_symbol_(const AST::INode* node, const std::string_view symbol);
    static bool is_num_(const AST::INode* node);

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
