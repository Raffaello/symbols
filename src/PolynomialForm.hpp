#pragma once

#include "AST.hpp"
#include "SymbolTable.hpp"
#include "mp_t.hpp"
#include "Simplifier.hpp"

#include <vector>
#include <string>
#include <memory>

class PolynomialForm
{
private:
    std::shared_ptr<SymbolTable> m_pSymbolTable;

    int              m_degree = -2;
    std::vector<AST> m_coeffs;    // coeffs are stored in reverse order (c + bx + ax^2 + ...)

    static bool collect_poly_(const AST::INode* node, PolynomialForm& pf, std::string_view symbol);
    static bool collect_poly_num_(const AST::INode* node, PolynomialForm& pf);
    static bool collect_poly_sym_(const AST::INode* node, PolynomialForm& pf, std::string_view symbol);
    static bool collect_poly_uny_(const AST::INode* node, PolynomialForm& pf, std::string_view symbol);
    static bool collect_poly_expr_(const AST::INode* node, PolynomialForm& pf, std::string_view symbol);

    static bool add(size_t index, PolynomialForm& pf, std::unique_ptr<AST::INode> pNode);
    static bool sub(size_t index, PolynomialForm& pf, std::unique_ptr<AST::INode> pNode);
    static bool mul(size_t index, PolynomialForm& pf, std::unique_ptr<AST::INode> pNode);
    static bool div(size_t index, PolynomialForm& pf, std::unique_ptr<AST::INode> pNode);

public:
    PolynomialForm(const std::shared_ptr<SymbolTable>& pSymbolTable);

    AST& operator[](size_t index);


    bool analyze(const AST::INode* node, const std::string& symbol);
    int  degree() noexcept;
    bool simplify();

    inline size_t size() const noexcept;
};

size_t PolynomialForm::size() const noexcept
{
    return m_coeffs.size();
}
