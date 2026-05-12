#include "PolynomialForm.hpp"

#include <iostream>
#include <stdexcept>
#include <cassert>
#include <format>

PolynomialForm::PolynomialForm(const std::shared_ptr<SymbolTable>& pSymbolTable) : m_pSymbolTable(pSymbolTable)
{
    if (m_pSymbolTable == nullptr)
        throw std::invalid_argument("symbol table is null");
}

double& PolynomialForm::operator[](size_t index)
{
    if (m_coeffs.size() < index + 1)
        m_coeffs.resize(index + 1);

    return m_coeffs[index];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PolynomialForm::collect_poly_(const AST::INode* node, PolynomialForm& pf, std::string_view symbol)
{
    if (node->is_num())
        return collect_poly_num_(node, pf);
    else if (node->is_symbol())
        return collect_poly_sym_(node, pf, symbol);
    else if (node->is_unary())
        return collect_poly_uny_(node, pf, symbol);
    else if (node->is_expr())
        return collect_poly_expr_(node, pf, symbol);

    return false;
}

bool PolynomialForm::collect_poly_num_(const AST::INode* node, PolynomialForm& pf)
{
    double d = 0.0;
    if (!AST::LeafNum::getValue(node, d))
    {
        std::cerr << "ERROR: unable to get num\n";
        return false;
    }

    pf[0] += d;
    return true;
}

bool PolynomialForm::collect_poly_sym_(const AST::INode* node, PolynomialForm& pf, std::string_view symbol)
{
    if (!node->is_symbol(symbol))
    {
        double d;
        auto   sym_value = AST::LeafSymbol::getValue(node);
        if (!m_pSymbolTable->getSymbol(sym_value, d))
        {
            std::cerr << std::format("ERROR: unable to get symbol '{}'\n", sym_value);
            return false;
        }

        std::cout << std::format("Symbol: {} = {}\n", sym_value, d);
        pf[0] += d;
        return true;
    }

    // otherwise is the symbol to solve for
    pf[1] += 1;
    return true;
}

bool PolynomialForm::collect_poly_uny_(const AST::INode* node, PolynomialForm& pf, std::string_view symbol)
{
    if (auto uny = dynamic_cast<const AST::NodeUnary*>(node))
    {
        if (uny->negate)
        {
            PolynomialForm pf2(m_pSymbolTable);
            if (!collect_poly_(uny->n.get(), pf2, symbol))
                return false;

            for (size_t i = 0; i < pf2.size(); ++i)
                pf[i] -= pf2[i];
        }

        return true;
    }

    return false;
}

bool PolynomialForm::collect_poly_expr_(const AST::INode* node, PolynomialForm& pf, std::string_view symbol)
{
    if (auto expr = dynamic_cast<const AST::NodeBin*>(node))
    {
        switch (expr->op)
        {
            using enum AST::eOperators;

        case ADD:
        {
            PolynomialForm pf2(m_pSymbolTable);
            if (!collect_poly_(expr->l.get(), pf, symbol))
                return false;
            if (!collect_poly_(expr->r.get(), pf2, symbol))
                return false;

            for (size_t i = 0; i < pf2.size(); ++i)
                pf[i] += pf2[i];
            return true;
        }

        case SUB:
        {
            PolynomialForm pf2(m_pSymbolTable);
            if (!collect_poly_(expr->l.get(), pf, symbol))
                return false;
            if (!collect_poly_(expr->r.get(), pf2, symbol))
                return false;

            for (size_t i = 0; i < pf2.size(); ++i)
                pf[i] -= pf2[i];

            return true;
        }

        case MUL:
        {
            PolynomialForm pf1(m_pSymbolTable), pf2(m_pSymbolTable);
            if (!collect_poly_(expr->l.get(), pf1, symbol))
                return false;
            if (!collect_poly_(expr->r.get(), pf2, symbol))
                return false;

            int          deg1  = pf1.degree();    // pf1.coeffs.size() - 1;
            int          deg2  = pf2.degree();    // pf2.coeffs.size() - 1;
            const size_t max_c = deg1 + deg2 + 1;
            for (size_t i = 0; i < pf1.size(); ++i)
            {
                for (size_t j = 0; j < pf2.size(); ++j)
                    pf[j + i] += pf1[i] * pf2[j];
            }

            return true;
        }
        break;

        case DIV:
        {
            PolynomialForm pf2(m_pSymbolTable);
            if (!collect_poly_(expr->l.get(), pf, symbol))
                return false;
            if (!collect_poly_(expr->r.get(), pf2, symbol))
                return false;

            const int deg2 = pf2.degree();
            if (deg2 > 0)
            {
                std::cerr << "ERROR: rational expression not implemented yet\n";
                return false;
            }

            assert(deg2 == 0);
            for (size_t i = 0; i < pf.size(); ++i)
                pf[i] /= pf2[deg2];

            return true;
        }
        break;

        case POW:
        {
            PolynomialForm pf1(m_pSymbolTable);
            PolynomialForm pf2(m_pSymbolTable);
            if (!collect_poly_(expr->l.get(), pf1, symbol))
                return false;
            if (!collect_poly_(expr->r.get(), pf2, symbol))
                return false;

            int deg2 = pf2.degree();
            if (deg2 > 0)
            {
                std::cerr << "ERROR: exponent can't be a symbol to solve for\n";
                return false;
            }

            if (pf2[0] == 0.0)
                pf[0] += 1;
            else if (pf2[0] == 1.0)
            {
                for (size_t i = 0; i < pf1.size(); ++i)
                    pf[i] += pf1[i];
            }
            else
            {
                // this can support only x^2 | (x+1)^(1+2)
                // not with a symbol in the RHS
                for (size_t k = 0; k < pf2.size(); ++k)
                {
                    int times = static_cast<int>(std::round(pf2[k]));
                    if (times - pf2[k] != 0.0)
                        return false;

                    assert(times >= 2);
                    --times;
                    for (size_t i = 0; i < pf1.size(); ++i)
                    {
                        for (size_t j = 0; j < pf1.size(); ++j)
                        {
                            const double ct = pf1[i] * pf1[j];
                            for (int t = 0; t < times; ++t)
                                pf[i + j + t] += ct;
                        }
                    }
                }
            }

            return true;
        }

        break;
        }
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PolynomialForm::analyze(const AST::INode* node, const std::string& symbol)
{
    m_degree = -2;
    if (!collect_poly_(node, *this, symbol))
    {
        m_degree = -1;
        m_coeffs.clear();
        return false;
    }

    return true;
}

int PolynomialForm::degree() noexcept
{
    if (m_degree == -1)
        return m_degree;

    // get the first non zero coeffs from reverse (higher)
    m_degree = 0;    // default at this point is a 0=0 solution
    for (size_t i = size(); i > 0; --i)
    {
        const auto i2 = i - 1;
        if (m_coeffs[i2] != 0.0)
        {
            m_degree = i2;
            break;
        }
    }

    return m_degree;
};
