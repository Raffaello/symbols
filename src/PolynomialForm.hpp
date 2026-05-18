#pragma once

#include "AST.hpp"
#include "SymbolTable.hpp"
#include "multi_precision.hpp"

#include <vector>
#include <string>
#include <memory>

class PolynomialForm
{
private:
    std::shared_ptr<SymbolTable> m_pSymbolTable;

    int                   m_degree = -2;
    std::vector<mp_num_t> m_coeffs;    // coeffs are stored in reverse order (c + bx + ax^2 + ...)

    bool collect_poly_(const AST::INode* node, PolynomialForm& pf, std::string_view symbol);
    bool collect_poly_num_(const AST::INode* node, PolynomialForm& pf);
    bool collect_poly_sym_(const AST::INode* node, PolynomialForm& pf, std::string_view symbol);
    bool collect_poly_uny_(const AST::INode* node, PolynomialForm& pf, std::string_view symbol);
    bool collect_poly_expr_(const AST::INode* node, PolynomialForm& pf, std::string_view symbol);

public:
    PolynomialForm(const std::shared_ptr<SymbolTable>& pSymbolTable);

    mp_num_t& operator[](size_t index);

    bool analyze(const AST::INode* node, const std::string& symbol);
    int  degree() noexcept;

    inline size_t size() const noexcept;
};

size_t PolynomialForm::size() const noexcept
{
    return m_coeffs.size();
}
