#include "Solver.hpp"

#include <iostream>
#include <format>
#include <cmath>
#include <stdexcept>
#include <cassert>

Solver::Solver(const std::shared_ptr<SymbolTable>& pSymbolTable) : m_pSymbolTable(pSymbolTable)
{
    if (m_pSymbolTable == nullptr)
        throw std::invalid_argument("symbol table is null");
}

Solver::PolynomialForm Solver::analyze_poly_(const AST::INode* node, std::string_view symbol)
{
    // coeffs are stored in reverse order
    PolynomialForm pf{
        .degree = 0,
        .coeffs = {},
    };

    if (!collect_poly_(node, pf, symbol))
    {
        pf.degree = -1;
        pf.coeffs.clear();
        return pf;
    }

    // get the first non zero coeffs from reverse (higher)
    pf.degree = 0;    // default at this point is a 0=0 solution
    for (size_t i = pf.coeffs.size(); i > 0; --i)
    {
        const auto i2 = i - 1;
        if (pf.coeffs[i2] != 0.0)
        {
            pf.degree = i2;
            break;
        }
    }

    return pf;
}

bool Solver::collect_poly_(const AST::INode* node, PolynomialForm& pf, std::string_view symbol)
{
    if (is_num_(node))
        return collect_poly_num_(node, pf);
    else if (is_symbol_(node))
        return collect_poly_sym_(node, pf, symbol);
    else if (is_unary_(node))
        return collect_poly_uny_(node, pf, symbol);
    else if (is_expr_(node))
        return collect_poly_expr_(node, pf, symbol);

    return false;
}

bool Solver::collect_poly_num_(const AST::INode* node, PolynomialForm& pf)
{
    double d = 0.0;
    if (!AST::LeafNum::getValue(node, d))
    {
        std::cerr << "ERROR: unable to get num\n";
        return false;
    }

    pf[0] += d;
    // if (coeffs.size() < 1)
    //     coeffs.push_back(d);
    // else
    //     coeffs[0] += d;

    return true;
}

bool Solver::collect_poly_sym_(const AST::INode* node, PolynomialForm& pf, std::string_view symbol)
{
    if (!is_symbol_(node, symbol))
    {
        auto   sym_value = AST::LeafSymbol::getValue(node);
        double d;
        if (!m_pSymbolTable->getSymbol(sym_value, d))
        {
            std::cerr << std::format("ERROR: unable to get symbol '{}'\n", sym_value);
            return false;
        }

        std::cout << std::format("Symbol: {} = {}\n", sym_value, d);
        // if (coeffs.size() == 0)
        //     coeffs.push_back(d);
        // else
        //     coeffs[0] += d;
        pf[0] += d;

        return true;
    }

    // otherwise is the symbol to solve for
    // switch (coeffs.size())
    // {
    // case 0:
    //     coeffs.push_back(0);
    //     [[fallthrough]];
    // case 1:
    //     coeffs.push_back(1);
    //     break;

    // default:
    //     coeffs[1] += 1;
    //     break;
    // }

    pf[1] += 1;

    return true;
}

bool Solver::collect_poly_uny_(const AST::INode* node, PolynomialForm& pf, std::string_view symbol)
{
    if (auto uny = dynamic_cast<const AST::NodeUnary*>(node))
    {
        if (uny->negate)
        {
            // std::vector<double> coeffs2;
            PolynomialForm pf2;
            if (!collect_poly_(uny->n.get(), pf2, symbol))
                return false;

            // if (coeffs2.size() > coeffs.size())
            //     coeffs.resize(coeffs2.size());

            for (size_t i = 0; i < pf2.coeffs.size(); ++i)
                pf[i] -= pf2[i];
        }

        return true;
    }

    return false;
}

bool Solver::collect_poly_expr_(const AST::INode* node, PolynomialForm& pf, std::string_view symbol)
{
    if (auto expr = dynamic_cast<const AST::NodeBin*>(node))
    {
        switch (expr->op)
        {
            using enum AST::eOperators;

        case ADD:
        {
            // std::vector<double> coeffs2;
            PolynomialForm pf2;
            if (!collect_poly_(expr->l.get(), pf, symbol))
                return false;
            if (!collect_poly_(expr->r.get(), pf2, symbol))
                return false;

            // if (coeffs2.size() > coeffs.size())
            //     coeffs.resize(coeffs2.size());
            for (size_t i = 0; i < pf2.coeffs.size(); ++i)
                pf[i] += pf2[i];
            return true;
        }

        case SUB:
        {
            // std::vector<double> coeffs2;
            PolynomialForm pf2;
            if (!collect_poly_(expr->l.get(), pf, symbol))
                return false;
            if (!collect_poly_(expr->r.get(), pf2, symbol))
                return false;

            // if (coeffs2.size() > coeffs.size())
            //     coeffs.resize(coeffs2.size());
            for (size_t i = 0; i < pf2.coeffs.size(); ++i)
                pf[i] -= pf2[i];

            return true;
        }

        case MUL:
        {
            const AST::INode* l;
            const AST::INode* r;

            if (is_expr_(expr->l.get()) ||
                is_expr_(expr->r.get()) ||
                is_unary_(expr->l.get()) ||
                is_unary_(expr->r.get()))
            {
                // std::vector<double> c1;
                // std::vector<double> c2;
                PolynomialForm pf1, pf2;
                if (!collect_poly_(expr->l.get(), pf1, symbol))
                    return false;
                if (!collect_poly_(expr->r.get(), pf2, symbol))
                    return false;

                int          deg1  = pf1.coeffs.size() - 1;
                int          deg2  = pf2.coeffs.size() - 1;
                const size_t max_c = deg1 + deg2 + 1;
                // if (pf.coeffs.size() < max_c)
                //     coeffs.resize(max_c);

                for (size_t i = 0; i < pf1.coeffs.size(); ++i)
                {
                    for (size_t j = 0; j < pf2.coeffs.size(); ++j)
                        pf[j + i] += pf1[i] * pf2[j];
                }

                return true;
            }
            else if (is_num_(expr->l.get()))
            {
                l = expr->l.get();
                r = expr->r.get();
            }
            else if (is_num_(expr->r.get()))
            {
                l = expr->r.get();
                r = expr->l.get();
            }
            else
                return false;

            // e.g. 1*1
            if (is_num_(l) && is_num_(r))
            {
                double dl;
                double dr;
                if (!AST::LeafNum::getValue(l, dl) || !AST::LeafNum::getValue(r, dr))
                {
                    std::cerr << "ERROR: unable to get numbers\n";
                    return false;
                }

                const double dlr = dl * dr;
                // if (coeffs.size() == 0)
                //     coeffs.push_back(dlr);
                // else
                pf[0] += dlr;

                return true;
            }
            // e.g. 2*x | x*2
            if (is_num_(l) && is_symbol_(r))
            {
                if (!is_symbol_(r, symbol))
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

                // switch (coeffs.size())
                // {
                // case 0:
                //     coeffs.push_back(0);
                //     [[fallthrough]];
                // case 1:
                //     coeffs.push_back(d);
                //     break;
                // default:
                // case 2:
                // coeffs[1] += d;
                // }

                pf[1] += d;
                return true;
            }

            if (is_expr_(l))
                if (!collect_poly_(l, pf, symbol))
                    return false;

            if (is_expr_(r))
                if (!collect_poly_(r, pf, symbol))
                    return false;
        }
        break;

        case DIV:
        {
            // e.g. 1/1
            if (is_num_(expr->l.get()) && is_num_(expr->r.get()))
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

                const double dlr = dl / dr;
                // if (coeffs.size() == 0)
                //     coeffs.push_back(dlr);
                // else
                //     coeffs[0] += dlr;

                pf[0] += dlr;
                return true;
            }
            // e.g. [expr]/2
            if (is_num_(expr->r.get()))
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

            if (is_expr_(expr->l.get()) ||
                is_expr_(expr->r.get()) ||
                is_unary_(expr->l.get()) ||
                is_unary_(expr->r.get()))
            {
                // TODO: not sure if it is ok, need to double check it.

                // TODO: used pf instead of 2 vectors and 2 degrees,
                //        and also create the compute degree function inside pf directly
                // std::vector<double> c1;
                // std::vector<double> c2;
                PolynomialForm pf1, pf2;
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
                    // if (coeffs.size() < c1.size())
                    //     coeffs.resize(c1.size());

                    for (size_t i = 0; i < pf1.coeffs.size(); ++i)
                        pf[i] += 1;
                }
                else if (pf2[0] == 1.0)
                {
                    // if (coeffs.size() < c1.size())
                    //     coeffs.resize(c1.size());

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
                                {
                                    const int i2 = i + j + t;
                                    // if (coeffs.size() < i2 + 1)
                                    //     coeffs.resize(i2 + 1);

                                    pf[i2] += ct;
                                }
                            }
                        }
                    }
                }

                return true;
            }
            // e.g 2^x | 2^2
            else if (is_num_(expr->l.get()))
            {
                // TODO e.g 2^x
                if (is_symbol_(expr->r.get()))
                {
                    std::cerr << "ERROR: POW operator not fully supported in solver yet! Can't elevate for a symbol\n";
                    return false;
                }
                // e.g 2^2
                else if (is_num_(expr->r.get()))
                {
                    double dl;
                    double dr;
                    if (!AST::LeafNum::getValue(expr->l.get(), dl) || !AST::LeafNum::getValue(expr->r.get(), dr))
                    {
                        std::cerr << "ERROR: unable to get numbers\n";
                        return false;
                    }

                    const double dlr = std::pow(dl, dr);
                    // if (coeffs.size() == 0)
                    //     coeffs.push_back(dlr);
                    // else
                    //     coeffs[0] += dlr;

                    pf[0] += dlr;
                    return true;
                }
            }
            // e.g x^2 || x^x
            else if (is_symbol_(expr->l.get(), symbol))
            {
                // TODO
                if (is_symbol_(expr->r.get()))
                {
                    std::cerr << "ERROR: POW operator not fully supported in solver yet! Can't elevate for a symbol\n";
                    return false;
                }
                else if (is_num_(expr->r.get()))
                {
                    // // TODO
                    // return false;

                    double d;
                    if (!AST::LeafNum::getValue(expr->r.get(), d))
                        return false;

                    int di = static_cast<int>(std::round(d));
                    if (d - di != 0)
                    {
                        std::cout << "ERROR: this can solve only integer exponential at the moment\n";
                        return false;
                    }

                    // if (coeffs.size() < di + 1)
                    //     coeffs.resize(di + 1);    // di as an index, so di + 1

                    // coeffs[di] += 1;
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

bool Solver::has_symbol_(const AST::INode* node, const std::string_view symbol) const noexcept
{
    if (is_symbol_(node, symbol))
        return true;
    else if (auto uny = dynamic_cast<const AST::NodeUnary*>(node))
    {
        if (has_symbol_(uny->n.get(), symbol))
            return true;
    }
    else if (auto bin = dynamic_cast<const AST::NodeBin*>(node))
    {
        if (has_symbol_(bin->l.get(), symbol))
            return true;

        return has_symbol_(bin->r.get(), symbol);
    }

    return false;
}

bool Solver::is_equation_(const AST::INode* node)
{
    if (auto bin = dynamic_cast<const AST::NodeBin*>(node))
        return bin->op == AST::eOperators::EQUAL;

    return false;
}

bool Solver::is_expr_(const AST::INode* node)
{
    if (auto bin = dynamic_cast<const AST::NodeBin*>(node))
        return bin->op == AST::eOperators::ADD ||
               bin->op == AST::eOperators::SUB ||
               bin->op == AST::eOperators::MUL ||
               bin->op == AST::eOperators::DIV ||
               bin->op == AST::eOperators::POW;

    return false;
}

bool Solver::is_unary_(const AST::INode* node)
{
    return dynamic_cast<const AST::NodeUnary*>(node) != nullptr;
}

bool Solver::is_symbol_(const AST::INode* node)
{
    return dynamic_cast<const AST::LeafSymbol*>(node) != nullptr;
}

bool Solver::is_symbol_(const AST::INode* node, const std::string_view symbol)
{
    if (auto sym = dynamic_cast<const AST::LeafSymbol*>(node))
    {
        if (sym->value == symbol)
            return true;
    }

    return false;
}

bool Solver::is_num_(const AST::INode* node)
{
    if (auto num = dynamic_cast<const AST::LeafNum*>(node))
        return true;

    return false;
}

bool Solver::solve_equation_(AST::INode* node, const std::string_view for_symbol)
{
    auto bin = dynamic_cast<AST::NodeBin*>(node);
    if (bin->op != AST::eOperators::EQUAL)
        return false;

    // LHS - RHS = 0
    // expr: LHS - RHS
    std::unique_ptr<AST::INode> n    = AST::NodeBin::make(AST::eOperators::SUB, std::move(bin->l), std::move(bin->r));
    auto                        nbin = dynamic_cast<AST::NodeBin*>(n.get());

    auto pf = analyze_poly_(n.get(), for_symbol);
    bin->l  = std::move(nbin->l);
    bin->r  = std::move(nbin->r);

    switch (pf.degree)
    {
    case -1:
        break;
    case 0:    // no variables
        if (pf.coeffs[0] == 0)
            m_solution = std::format("inf solutions");
        else
            m_solution = std::format("no solution");

        return true;
    case 1:    // linear
    {
        double s = -pf.coeffs[0] / pf.coeffs[1];

        // To avoid having -0 as it is just 0
        if (s == 0.0)
            s = std::fabs(s);

        m_solution = std::format("{} = {}", for_symbol, s);
    }
        return true;

    case 2:
    {
        const double a = pf.coeffs[2];
        const double b = pf.coeffs[1];
        const double c = pf.coeffs[0];

        const double delta = (b * b) - (4.0 * a * c);

        if (delta < 0.0)
        {
            m_solution = "no real solutions, complex roots not supported yet";
            return true;
        }

        const double        sq_delta = std::sqrt(delta);
        const double        a2       = 2.0 * a;
        std::vector<double> s;
        // sol 1
        const double s1 = (-b + sq_delta) / a2;
        s.push_back(s1);
        // sol 2
        if (delta != 0.0)
        {
            const double s2 = (-b - sq_delta) / a2;
            s.push_back(s2);
        }

        if (s.size() == 2)
            m_solution = std::format("{} = {}, {} = {}", for_symbol, s[0], for_symbol, s[1]);
        else
            m_solution = std::format("{} = {}", for_symbol, s[0]);

        return true;
    }
    break;
    default:
        // Newton's method
        // todo
        break;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Solver::solve(AST& ast, const std::string_view for_symbol)
{
    m_solution = "";
    if (!has_symbol_(ast.getRoot(), for_symbol))
    {
        std::cerr << std::format("Symbol to solve for '{}' not found in {}\n", for_symbol, ast.to_string());
        return false;
    }

    if (!is_equation_(ast.getRoot()))
    {
        std::cerr << std::format("ERROR: {} is not an equation!\n", ast.to_string());
        return false;
    }

    auto pRoot = const_cast<AST::INode*>(ast.getRoot());
    if (auto bin = dynamic_cast<AST::NodeBin*>(pRoot))
    {
        // the operator here is = as it is an equation
        if (!solve_equation_(bin, for_symbol))
        {
            std::cerr << std::format("ERROR: unable to solve equation: [{}, {}]\n", ast.to_string(), for_symbol);
            return false;
        }
    }
    else
    {
        std::cerr << std::format("ERROR: unable to navigate the equation.\n");
        return false;
    }

    return true;
}
