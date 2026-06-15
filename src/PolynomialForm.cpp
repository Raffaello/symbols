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

AST& PolynomialForm::operator[](size_t index)
{
    if (m_coeffs.size() < index + 1)
    {
        m_coeffs.resize(index + 1);
        for (size_t i = 0; i < index; ++i)
        {
            if (m_coeffs[i].getRoot() == nullptr)
                m_coeffs[i].setRoot(std::move(AST::LeafNum::make(0)));
        }

        m_coeffs[index].setRoot(std::move(AST::LeafNum::make(0)));
    }

    return m_coeffs[index];
}

bool PolynomialForm::add(size_t index, PolynomialForm& pf, std::unique_ptr<AST::INode> pNode)
{
    assert(pf[index].getRoot() != nullptr);
    assert(pNode != nullptr);

    if (pf[index].getRoot()->is_num() && pNode->is_num())
    {
        ast_num_t a, b;
        if (!AST::LeafNum::getValue(pf[index].getRoot(), a) || !AST::LeafNum::getValue(pNode.get(), b))
            return false;

        pf[index].setRoot(AST::LeafNum::make(a + b));
    }
    else
    {
        ast_num_t v;
        if (pf[index].getRoot()->is_num() && AST::LeafNum::getValue(pf[index].getRoot(), v) && v.is_zero())
            pf[index].setRoot(std::move(pNode));
        else if (pNode->is_num() && AST::LeafNum::getValue(pNode.get(), v) && v.is_zero())
            return true;
        else
        {
            pf[index].setRoot(AST::NodeBin::make(
                AST::eOperators::ADD,
                pf[index].cloneRoot(),
                std::move(pNode)));
        }
    }

    return true;
}

bool PolynomialForm::sub(size_t index, PolynomialForm& pf, std::unique_ptr<AST::INode> pNode)
{
    assert(pf[index].getRoot());
    assert(pNode != nullptr);

    if (pf[index].getRoot()->is_num() && pNode->is_num())
    {
        ast_num_t a, b;
        if (!AST::LeafNum::getValue(pf[index].getRoot(), a) || !AST::LeafNum::getValue(pNode.get(), b))
            return false;

        pf[index].setRoot(AST::LeafNum::make(a - b));
    }
    else
        pf[index].setRoot(AST::NodeBin::make(
            AST::eOperators::SUB,
            pf[index].cloneRoot(),
            std::move(pNode)));

    return true;
}

bool PolynomialForm::mul(size_t index, PolynomialForm& pf, std::unique_ptr<AST::INode> pNode)
{
    assert(pf[index].getRoot());
    assert(pNode != nullptr);

    if (pf[index].getRoot()->is_num() && pNode->is_num())
    {
        ast_num_t a, b;
        if (!AST::LeafNum::getValue(pf[index].getRoot(), a) || !AST::LeafNum::getValue(pNode.get(), b))
            return false;

        pf[index].setRoot(AST::LeafNum::make(a * b));
    }
    else
        pf[index].setRoot(AST::NodeBin::make(
            AST::eOperators::MUL,
            pf[index].cloneRoot(),
            std::move(pNode)));

    return true;
}

bool PolynomialForm::div(size_t index, PolynomialForm& pf, std::unique_ptr<AST::INode> pNode)
{
    assert(pf[index].getRoot());
    assert(pNode != nullptr);

    if (pf[index].getRoot()->is_num() && pNode->is_num())
    {
        ast_num_t a, b;
        if (!AST::LeafNum::getValue(pf[index].getRoot(), a) || !AST::LeafNum::getValue(pNode.get(), b))
            return false;

        if (b.is_zero())
            return false;

        pf[index].setRoot(AST::LeafNum::make(a / b));
    }
    else
        pf[index].setRoot(AST::NodeBin::make(
            AST::eOperators::DIV,
            pf[index].cloneRoot(),
            std::move(pNode)));

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PolynomialForm::collect_poly_(const AST::INode* pNode, PolynomialForm& pf, std::string_view symbol)
{
    if (pNode->is_num())
        return collect_poly_num_(pNode, pf);
    else if (pNode->is_symbol())
        return collect_poly_sym_(pNode, pf, symbol);
    else if (pNode->is_unary())
        return collect_poly_uny_(pNode, pf, symbol);
    else if (pNode->is_expr())
        return collect_poly_expr_(pNode, pf, symbol);

    return false;
}

bool PolynomialForm::collect_poly_num_(const AST::INode* pNode, PolynomialForm& pf)
{
    ast_num_t d = 0.0;
    if (!AST::LeafNum::getValue(pNode, d))
    {
        std::cerr << "ERROR: unable to get num\n";
        return false;
    }

    add(0, pf, AST::LeafNum::make(d));
    return true;
}

bool PolynomialForm::collect_poly_sym_(const AST::INode* pNode, PolynomialForm& pf, std::string_view symbol)
{
    assert(pNode->is_symbol());

    if (pNode->is_symbol(symbol))
    {
        // otherwise is the symbol to solve for
        add(1, pf, AST::LeafNum::make(1));
        return true;
    }

    // symbolic constant
    mp_t d;
    auto sym_value = AST::LeafSymbol::getValue(pNode);
    if (pf.m_pSymbolTable->getSymbol(sym_value, d))
    {
        add(0, pf, AST::LeafNum::make(d));
        return true;
    }

    add(0, pf, AST::LeafSymbol::make(AST::LeafSymbol::getValue(pNode)));
    return true;
}

bool PolynomialForm::collect_poly_uny_(const AST::INode* pNode, PolynomialForm& pf, std::string_view symbol)
{
    if (auto uny = dynamic_cast<const AST::NodeUnary*>(pNode))
    {
        PolynomialForm pf2(pf.m_pSymbolTable);
        if (!collect_poly_(uny->n.get(), pf2, symbol))
            return false;

        if (uny->negate)
        {
            for (size_t i = 0; i < pf2.size(); ++i)
                sub(i, pf, pf2[i].cloneRoot());
        }
        else
        {
            for (size_t i = 0; i < pf2.size(); ++i)
                add(i, pf, pf2[i].cloneRoot());
        }

        return true;
    }

    return false;
}

bool PolynomialForm::collect_poly_expr_(const AST::INode* pNode, PolynomialForm& pf, std::string_view symbol)
{
    if (auto expr = dynamic_cast<const AST::NodeBin*>(pNode))
    {
        switch (expr->op)
        {
            using enum AST::eOperators;

        case ADD:
        {
            PolynomialForm pf2(pf.m_pSymbolTable);
            if (!collect_poly_(expr->l.get(), pf, symbol))
                return false;
            if (!collect_poly_(expr->r.get(), pf2, symbol))
                return false;

            for (size_t i = 0; i < pf2.size(); ++i)
                add(i, pf, pf2[i].cloneRoot());

            return true;
        }

        case SUB:
        {
            PolynomialForm pf2(pf.m_pSymbolTable);
            if (!collect_poly_(expr->l.get(), pf, symbol))
                return false;
            if (!collect_poly_(expr->r.get(), pf2, symbol))
                return false;

            for (size_t i = 0; i < pf2.size(); ++i)
                sub(i, pf, pf2[i].cloneRoot());

            return true;
        }

        case MUL:
        {
            PolynomialForm pf1(pf.m_pSymbolTable), pf2(pf.m_pSymbolTable);
            if (!collect_poly_(expr->l.get(), pf1, symbol))
                return false;
            if (!collect_poly_(expr->r.get(), pf2, symbol))
                return false;

            for (size_t i = 0; i < pf1.size(); ++i)
            {
                for (size_t j = 0; j < pf2.size(); ++j)
                {
                    add(j + i,
                        pf,
                        AST::NodeBin::make(
                            AST::eOperators::MUL,
                            pf1[i].cloneRoot(),
                            pf2[j].cloneRoot()));
                }
            }

            return true;
        }
        break;

        case DIV:
        {
            PolynomialForm pf2(pf.m_pSymbolTable);
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
                if (!div(i, pf, pf2[0].cloneRoot()))
                    return false;

            return true;
        }
        break;

        case POW:
        {
            PolynomialForm pf1(pf.m_pSymbolTable);
            PolynomialForm pf2(pf.m_pSymbolTable);
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

            assert(pf2[0].getRoot() != nullptr);
            if (pf2[0].getRoot()->is_num())
            {
                ast_num_t v;
                AST::LeafNum::getValue(pf2[0].getRoot(), v);
                if (v == 0)
                    add(0, pf, AST::LeafNum::make(1));
                else if (v == 1)
                {
                    for (size_t i = 0; i < pf1.size(); ++i)
                        add(i, pf, pf1[i].cloneRoot());
                }
                else
                {
                    // General integer exponentiation via repeated multiplication
                    auto [exponent, r] = mp_t::convert_to_mpz_int(v);
                    if (r != 0)
                    {
                        std::cerr << "ERROR exponent must be an integer\n";
                        return false;
                    }

                    if (exponent < 0)
                    {
                        std::cerr << "ERROR: negative exponents are not supported in polynomial form\n";
                        return false;
                    }

                    assert(exponent >= 2);
                    PolynomialForm result(pf.m_pSymbolTable);
                    result[0].setRoot(AST::LeafNum::make(1));
                    for (mp::mpz_int e = 0; e < exponent; ++e)
                    {
                        PolynomialForm tmp(pf.m_pSymbolTable);
                        for (size_t i = 0; i < result.size(); ++i)
                        {
                            for (size_t j = 0; j < pf1.size(); ++j)
                            {
                                add(
                                    i + j,
                                    tmp,
                                    AST::NodeBin::make(
                                        AST::eOperators::MUL,
                                        result[i].cloneRoot(),
                                        pf1[j].cloneRoot()));
                            }
                        }

                        result = std::move(tmp);
                    }

                    for (size_t i = 0; i < result.size(); ++i)
                        add(i, pf, result[i].cloneRoot());
                }
            }
            else if (pf2[0].getRoot()->is_symbol())
            {
                // pf1 polynomial form, pf2 a symbol, e.g. x^a, (1+x)^a
                // TODO: this is not polynomial, and must be upgraded to a general symbolic expression
                std::cerr << std::format("DEBUG: pf2[0] not num? is_expr={}, is_unary={}, is_symbol={} => {}\n", pf2[0].getRoot()->is_expr(), pf2[0].getRoot()->is_unary(), pf2[0].getRoot()->is_symbol(), pf2[0].to_string());
                return false;
            }
            else
            {
                // pf2[0] not a num, maybe a symbol? leave as it is. for now
                // TODO: this is not polynomial, and must be upgraded to a general symbolic expression
                std::cerr << std::format("DEBUG: pf2[0] not num? is_expr={}, is_unary={}, is_symbol={} => {}\n", pf2[0].getRoot()->is_expr(), pf2[0].getRoot()->is_unary(), pf2[0].getRoot()->is_symbol(), pf2[0].to_string());
                return false;
            }

            return true;
        }

        break;
        }
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PolynomialForm::analyze(const AST::INode* pNode, const std::string& symbol)
{
    m_degree = -2;
    m_coeffs.clear();

    if (!AST::has_symbol(pNode, symbol))
    {
        m_degree = -1;
        return false;
    }

    if (!collect_poly_(pNode, *this, symbol))
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
        const auto  i2    = i - 1;
        const auto* pRoot = m_coeffs[i2].getRoot();
        if (pRoot == nullptr)
            continue;

        // if it has been simplified to zero, just move on...
        ast_num_t v;
        if (pRoot->is_num() && AST::LeafNum::getValue(pRoot, v) && v.is_zero())
            continue;

        m_degree = i2;
        break;
    }

    return m_degree;
}

bool PolynomialForm::simplify()
{
    for (size_t i = 0; i < size(); ++i)
    {
        if (!Simplifier::reduce(m_coeffs[i], false))
        {
            std::cerr << std::format("ERROR: unable to simplify term of degree {}: {}\n", i, m_coeffs[i].to_string());
            return false;
        }
    }

    return true;
}
