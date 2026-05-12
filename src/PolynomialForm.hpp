#pragma once

#include "AST.hpp"
#include "SymbolTable.hpp"

#include <vector>
#include <string>
#include <memory>

// TODO: create a compute degree function here, so doesn't have to do every single time in the solver,
//       also the operation to add a coeffs, if it has to push it back or update what is already there (operator[])

class PolynomialForm
{
private:
    std::shared_ptr<SymbolTable> m_pSymbolTable;

    bool collect_poly_(const AST::INode* node, PolynomialForm& pf, std::string_view symbol);
    bool collect_poly_num_(const AST::INode* node, PolynomialForm& pf);
    bool collect_poly_sym_(const AST::INode* node, PolynomialForm& pf, std::string_view symbol);
    bool collect_poly_uny_(const AST::INode* node, PolynomialForm& pf, std::string_view symbol);
    bool collect_poly_expr_(const AST::INode* node, PolynomialForm& pf, std::string_view symbol);

public:                            // todo remove this public
    int                 degree;
    std::vector<double> coeffs;    // coeffs are stored in reverse order (c + bx + ax^2 + ...)

public:
    PolynomialForm(const std::shared_ptr<SymbolTable>& pSymbolTable);

    bool analyze(const AST::INode* node, const std::string& symbol);

    double& operator[](size_t index)
    {
        if (coeffs.size() < index + 1)
            coeffs.resize(index + 1);

        return coeffs[index];
    }

    // inline int degree() noexcept
    // {
    //     degree = 0;    // default at this point is a 0=0 solution
    //     for (size_t i = coeffs.size(); i > 0; --i)
    //     {
    //         const auto i2 = i - 1;
    //         if (coeffs[i2] != 0.0)
    //         {
    //             degree = i2;
    //             break;
    //         }
    //     }

    // return degree;
    // };
};
