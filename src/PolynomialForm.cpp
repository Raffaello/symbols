#include "PolynomialForm.hpp"

#include <iostream>
#include <stdexcept>
#include <cassert>

PolynomialForm::PolynomialForm(const std::shared_ptr<SymbolTable>& pSymbolTable) : m_pSymbolTable(pSymbolTable)
{
    if (m_pSymbolTable == nullptr)
        throw std::invalid_argument("symbol table is null");
}

bool PolynomialForm::analyze(const AST::INode* node, const std::string& symbol)
{
    if (!collect_poly_(node, *this, symbol))
    {
        degree = -1;
        coeffs.clear();
        return false;
    }

    // get the first non zero coeffs from reverse (higher)
    degree = 0;    // default at this point is a 0=0 solution
    for (size_t i = coeffs.size(); i > 0; --i)
    {
        const auto i2 = i - 1;
        if (coeffs[i2] != 0.0)
        {
            degree = i2;
            break;
        }
    }

    return true;
}

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
        auto   sym_value = AST::LeafSymbol::getValue(node);
        double d;
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

            for (size_t i = 0; i < pf2.coeffs.size(); ++i)
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

            for (size_t i = 0; i < pf2.coeffs.size(); ++i)
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

            for (size_t i = 0; i < pf2.coeffs.size(); ++i)
                pf[i] -= pf2[i];

            return true;
        }

        case MUL:
        {
            const AST::INode* l;
            const AST::INode* r;

            if (expr->l->is_expr() ||
                expr->r->is_expr() ||
                expr->l->is_unary() ||
                expr->r->is_unary())
            {
                PolynomialForm pf1(m_pSymbolTable), pf2(m_pSymbolTable);
                if (!collect_poly_(expr->l.get(), pf1, symbol))
                    return false;
                if (!collect_poly_(expr->r.get(), pf2, symbol))
                    return false;

                int          deg1  = pf1.coeffs.size() - 1;
                int          deg2  = pf2.coeffs.size() - 1;
                const size_t max_c = deg1 + deg2 + 1;
                for (size_t i = 0; i < pf1.coeffs.size(); ++i)
                {
                    for (size_t j = 0; j < pf2.coeffs.size(); ++j)
                        pf[j + i] += pf1[i] * pf2[j];
                }

                return true;
            }
            else if (expr->l->is_num())
            {
                l = expr->l.get();
                r = expr->r.get();
            }
            else if (expr->r->is_num())
            {
                l = expr->r.get();
                r = expr->l.get();
            }
            else
                return false;

            // e.g. 1*1
            if (l->is_num() && r->is_num())
            {
                double dl;
                double dr;
                if (!AST::LeafNum::getValue(l, dl) || !AST::LeafNum::getValue(r, dr))
                {
                    std::cerr << "ERROR: unable to get numbers\n";
                    return false;
                }

                const double dlr  = dl * dr;
                pf[0]            += dlr;
                return true;
            }
            // e.g. 2*x | x*2
            if (l->is_num() && r->is_symbol())
            {
                if (!r->is_symbol(symbol))
                {
                    std::cerr << std::format("ERROR: generic symbolic solver not implemented yet: unknown symbol '{}'\n", AST::LeafSymbol::getValue(r));
                    return false;
                }

                double d;
                if (!AST::LeafNum::getValue(l, d))
                {
                    std::cerr << "ERROR: unable to get number\n";
                    return false;
                }

                pf[1] += d;
                return true;
            }

            if (l->is_expr())
                if (!collect_poly_(l, pf, symbol))
                    return false;

            if (r->is_expr())
                if (!collect_poly_(r, pf, symbol))
                    return false;
        }
        break;

        case DIV:
        {
            // e.g. 1/1
            if (expr->l->is_num() && expr->r->is_num())
            {
                double dl;
                double dr;
                if (!AST::LeafNum::getValue(expr->l.get(), dl) || !AST::LeafNum::getValue(expr->r.get(), dr))
                {
                    std::cerr << "ERROR: unable to get numbers\n";
                    return false;
                }

                // division by zero
                // if(dr == 0.0)
                // {
                // }

                const double dlr  = dl / dr;
                pf[0]            += dlr;
                return true;
            }
            // e.g. [expr]/2
            if (expr->r->is_num())
            {
                double d;
                if (!AST::LeafNum::getValue(expr->r.get(), d))
                {
                    std::cerr << "ERROR: unable to get number\n";
                    return false;
                }

                if (!collect_poly_(expr->l.get(), pf, symbol))
                    return false;

                for (auto& c : pf.coeffs)
                    c /= d;

                return true;
            }
        }
        break;
        case POW:
        {
            // TODO: handle special cases, e.g.: x^0 => 1

            if (expr->l->is_expr() ||
                expr->r->is_expr() ||
                expr->l->is_unary() ||
                expr->r->is_unary())
            {
                // TODO: not sure if it is ok, need to double check it.

                // TODO: used pf instead of 2 vectors and 2 degrees,
                //        and also create the compute degree function inside pf directly
                PolynomialForm pf1(m_pSymbolTable), pf2(m_pSymbolTable);
                if (!collect_poly_(expr->l.get(), pf1, symbol))
                    return false;
                if (!collect_poly_(expr->r.get(), pf2, symbol))
                    return false;

                int deg1 = pf1.coeffs.size() - 1;
                int deg2 = pf2.coeffs.size() - 1;

                if (deg2 != 0)
                {
                    std::cerr << "ERROR: e.g. 'x^x' not supported yet, only polynomials  e.g. 'x^2'\n";
                    return false;
                }

                // x^2 => x*x => c[2] += 1
                if (pf2[0] == 0.0)
                {
                    for (size_t i = 0; i < pf1.coeffs.size(); ++i)
                        pf[i] += 1;
                }
                else if (pf2[0] == 1.0)
                {
                    for (size_t i = 0; i < pf1.coeffs.size(); ++i)
                        pf[i] += pf1[i];
                }
                else
                {
                    // this can support only x^2 | (x+1)^(1+2)
                    // not with a symbol in the RHS
                    for (size_t k = 0; k < pf2.coeffs.size(); ++k)
                    {
                        int times = static_cast<int>(std::round(pf2[k]));
                        if (times - pf2[k] != 0.0)
                            return false;

                        assert(times >= 2);
                        --times;
                        for (size_t i = 0; i < pf1.coeffs.size(); ++i)
                        {
                            for (size_t j = 0; j < pf1.coeffs.size(); ++j)
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
            // e.g 2^x | 2^2
            else if (expr->l->is_num())
            {
                // TODO e.g 2^x
                if (expr->r->is_symbol())
                {
                    std::cerr << "ERROR: POW operator not fully supported in solver yet! Can't elevate for a symbol\n";
                    return false;
                }
                // e.g 2^2
                else if (expr->r->is_num())
                {
                    double dl;
                    double dr;
                    if (!AST::LeafNum::getValue(expr->l.get(), dl) || !AST::LeafNum::getValue(expr->r.get(), dr))
                    {
                        std::cerr << "ERROR: unable to get numbers\n";
                        return false;
                    }

                    const double dlr  = std::pow(dl, dr);
                    pf[0]            += dlr;
                    return true;
                }
            }
            // e.g x^2 || x^x
            else if (expr->l->is_symbol(symbol))
            {
                // TODO
                if (expr->r->is_symbol())
                {
                    std::cerr << "ERROR: POW operator not fully supported in solver yet! Can't elevate for a symbol\n";
                    return false;
                }
                else if (expr->r->is_num())
                {
                    double d;
                    if (!AST::LeafNum::getValue(expr->r.get(), d))
                        return false;

                    int di = static_cast<int>(std::round(d));
                    if (d - di != 0)
                    {
                        std::cout << "ERROR: this can solve only integer exponential at the moment\n";
                        return false;
                    }

                    pf[di] += 1;
                    return true;
                }
            }
        }

        break;
        }
    }

    return false;
}
