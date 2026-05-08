#pragma once

#include "AST.hpp"
#include "SymbolTable.hpp"

#include <sstream>
#include <string>
#include <vector>
#include <memory>

/**
 * TODO: This is to be rewritten as it is wrong.
 *       Most Likely have a better AST specific for the solver
 *       need to work out how to manipulate the whole AST, for e.g. dividing by 2 all the equation, like to solve the case 2*x=1
 *       Also need to have a better grammar about to condense symbols: 2*x or x*2 => 2x, x+x => 2x etc..
 *       at the moment the syntax 2x is not neither supported and it should instead
 *
 * @brief Solver for 1 variable e.g. -x = x+2 => -2x=2 => x = -1
 *
 * TODO: for now just 1 variable is fine.
 * TODO: should have symbol table? no
 * TODO: later try to solve this: ax + b = 0 => x = b/a [b is not know, a is not known] so are just really symbols without a value
 */
class Solver
{
public:
    struct PolynomialForm
    {
        int                 degree;
        std::vector<double> coeffs;    // coeffs are stored in reverse order (c + bx + ax^2 + ...)
    };

private:
    std::shared_ptr<SymbolTable> m_pSymbolTable;
    std::string                  m_solution;

    PolynomialForm analyze_poly_(const AST::INode* node, std::string_view symbol) const noexcept;
    static bool    collect_poly_(const AST::INode* node, std::vector<double>& coeffs, std::string_view symbol);

    bool has_symbol_(const AST::INode* node, const std::string_view symbol) const noexcept;

    static bool is_equation_(const AST::INode* node);
    static bool is_expr_(const AST::INode* node);
    static bool is_unary_(const AST::INode* node);
    static bool is_symbol_(const AST::INode* node);
    static bool is_symbol_(const AST::INode* node, const std::string_view symbol);
    static bool is_num_(const AST::INode* node);


    std::unique_ptr<AST::INode> simplify_(std::unique_ptr<AST::INode>& node);
    std::unique_ptr<AST::INode> simplifyExpr_(std::unique_ptr<AST::INode>& node);
    std::unique_ptr<AST::INode> simplifyExprSumOrMulOrPow_(std::unique_ptr<AST::INode>& node);


    bool                solve_equation_(AST::INode* node, const std::string_view for_symbol);
    std::optional<bool> solve_expr_(std::unique_ptr<AST::INode>& node, const std::string_view for_symbol);
    bool                solve_unary_(std::unique_ptr<AST::INode>& node, const std::string_view for_symbol);

public:
    Solver(const std::shared_ptr<SymbolTable>& pSymbolTable);

    bool solve(AST& ast, const std::string_view for_symbol);

    inline std::string solution() const noexcept;
};

inline std::string Solver::solution() const noexcept
{
    return m_solution;
}
