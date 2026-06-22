#include "Solver.hpp"
#include "formatters.hpp"
#include "mp_t.hpp"


#include <iostream>
#include <format>
#include <cmath>
#include <stdexcept>
#include <cassert>
#include <algorithm>
#include <numbers>

Solver::Solver(const std::shared_ptr<SymbolTable>& pSymbolTable) : m_pSymbolTable(pSymbolTable)
{
    if (m_pSymbolTable == nullptr)
        throw std::invalid_argument("symbol table is null");
}

bool Solver::solve_equation_(const AST::INode* node, const std::string_view for_symbol)
{
    PolynomialForm pf(m_pSymbolTable);
    bool           res = pf.analyze(node, std::string(for_symbol));

    if (!res)
        return false;

    if (!pf.simplify())
        return false;

    ast_num_t         v;
    std::vector<mp_t> sols;
    switch (pf.degree())
    {
    case -1:
        return false;

    case 0:    // no variables
    {
        if (pf[0].getRoot()->is_num())
        {
            if (!AST::LeafNum::getValue(pf[0].getRoot(), v))
                return false;
            if (v == 0)
                m_solution = std::format("inf solutions");
            else
                m_solution = std::format("no solution");
        }
        else if (pf[0].getRoot()->is_symbol())
            m_solution = std::format("{}", AST::LeafSymbol::getValue(pf[0].getRoot()));
        else
        {
            m_solution = std::format("is_expr={}, is_unary={} ????", pf[0].getRoot()->is_expr(), pf[0].getRoot()->is_unary());
            return false;
        }

        return true;
    }

    case 1:    // linear
        if (pf[0].getRoot()->is_num() && pf[1].getRoot()->is_num())
        {
            ast_num_t a, b;
            if (!AST::LeafNum::getValue(pf[0].getRoot(), a) || !AST::LeafNum::getValue(pf[1].getRoot(), b))
                return false;

            if (b.is_zero())
            {
                m_solution = std::format("no solution");
                return true;
            }
            else
                sols.emplace_back(-mp_t(a) / b);
        }
        else
        {
            if (pf[0].getRoot()->is_num())
            {
                ast_num_t v_;
                if (!AST::LeafNum::getValue(pf[0].getRoot(), v_))
                    return false;

                mp_t v     = v_;
                m_solution = std::format("{} = {} / ({})", for_symbol, -v, pf[1].to_string());
            }
            else if (pf[1].getRoot()->is_num())
            {
                ast_num_t v_;
                if (!AST::LeafNum::getValue(pf[1].getRoot(), v_))
                    return false;

                mp_t v = -mp_t(v_);
                if (v == 1)
                    m_solution = std::format("{} = {}", for_symbol, pf[0].to_string());
                else if (v == -1)
                {
                    if (pf[0].getRoot()->is_unary())
                    {
                        auto* uny = dynamic_cast<AST::NodeUnary*>(pf[0].getRoot());
                        if (uny->negate)
                            m_solution = std::format("{} = {}", for_symbol, AST::to_string(uny->n.get()));
                        else
                            m_solution = std::format("{} = -({})", for_symbol, AST::to_string(uny->n.get()));
                    }
                    else if (pf[0].getRoot()->is_symbol())
                        m_solution = std::format("{} = -{}", for_symbol, pf[0].to_string());
                    else
                        m_solution = std::format("{} = -({})", for_symbol, pf[0].to_string());
                }
                else
                    m_solution = std::format("{} = ({}) / {}", for_symbol, pf[0].to_string(), v);
            }
            else
                m_solution = std::format("{} = ({}) / ({})", for_symbol, pf[0].to_string(), pf[1].to_string());

            return true;
        }
        break;
    case 2:
    {
        if (pf[0].getRoot() == nullptr)
            pf[0].setRoot(AST::LeafNum::make(0));
        if (pf[1].getRoot() == nullptr)
            pf[1].setRoot(AST::LeafNum::make(0));
        if (pf[2].getRoot() == nullptr)
            pf[1].setRoot(AST::LeafNum::make(0));

        if (!pf[0].getRoot()->is_num() || !pf[1].getRoot()->is_num() || !pf[2].getRoot()->is_num())
        {
            m_solution = std::format("TODO: degree 3: pf[0], pf[1],pf[2] are not only numbers");
            return false;
        }
        else
        {
            ast_num_t a_, b_, c_;
            if (!AST::LeafNum::getValue(pf[2].getRoot(), a_) ||
                !AST::LeafNum::getValue(pf[1].getRoot(), b_) ||
                !AST::LeafNum::getValue(pf[0].getRoot(), c_))
                return false;

            const mp_t a     = a_;
            const mp_t b     = b_;
            const mp_t c     = c_;
            const mp_t delta = (b * b) - (a * c * 4);
            if (delta < 0)
            {
                m_solution = "no real solutions, complex roots not supported yet";
                return true;
            }

            const mp_t sq_delta = mp_t::sqrt(delta);
            const mp_t a2       = a * 2;
            // sol 1
            sols.emplace_back((-b + sq_delta) / a2);
            // sol 2
            if (!delta.isZero())
                sols.emplace_back((-b - sq_delta) / a2);
        }
    }
    break;
    case 3:
    {
        if (!pf[0].getRoot()->is_num() || !pf[1].getRoot()->is_num() || !pf[2].getRoot()->is_num() || !pf[3].getRoot()->is_num())
        {
            m_solution = std::format("TODO: degree 3; pf[0], pf[1], pf[2], pf[3] are not only numbers");
            return false;
        }

        // Cardano's formula
        ast_num_t a_, b_, c_, d_;
        if (!AST::LeafNum::getValue(pf[2].getRoot(), a_) ||
            !AST::LeafNum::getValue(pf[1].getRoot(), b_) ||
            !AST::LeafNum::getValue(pf[0].getRoot(), c_) ||
            !AST::LeafNum::getValue(pf[3].getRoot(), d_))
            return false;

        const mp_t a = mp_t(a_) / d_;
        const mp_t b = mp_t(b_) / d_;
        const mp_t c = mp_t(c_) / d_;

        const mp_t aa = a * a;
        const mp_t p  = b - (aa / 3);
        const mp_t q  = (a * 2) * (aa / 27) - a * (b / 3) + c;

        const mp_t p3    = p * p * p;
        const mp_t delta = (q * q) / 4 + p3 / 27;

        const mp_t a_3 = a / 3;
        const mp_t q_2 = q / 2;
        if (delta < 0)
        {
            mp::mpfr_float PI_;
            mpfr_const_pi(PI_.backend().data(), MPFR_RNDN);
            mp_t PI = PI_;

            const mp_t r     = mp_t::sqrt(-p / 3) * 2;
            const mp_t denom = mp_t::sqrt(-p3 / 27);
            mp_t       z     = -q_2 / denom;
            z.clamp(-1, +1);

            const mp_t phi = mp_t::acos(z);
            sols.emplace_back(r * mp_t::cos(phi / 3) - a / 3);
            sols.emplace_back(r * mp_t::cos((phi + (2 * PI)) / 3) - a_3);
            sols.emplace_back(r * mp_t::cos((phi + (4 * PI)) / 3) - a_3);
        }
        else if (delta.isZero())
        {
            const mp_t u = mp_t::cbrt(-q_2);
            sols.emplace_back((u * 2) - a_3);
            sols.emplace_back((-u) - a_3);
        }
        else    // if (delta > 0.0)
        {
            // one real solution, two complex
            const mp_t sq_delta = mp_t::sqrt(delta);

            const mp_t u = mp_t::cbrt(-q_2 + sq_delta);
            const mp_t v = mp_t::cbrt(-q_2 - sq_delta);
            const mp_t y = u + v;

            sols.emplace_back(y - (a_3));
        }
    }
    break;

    default:
        // Newton's method
        // todo
        std::cerr << std::format("CRITICAL: Not implemented to solve polynomial of degree {}\n", pf.degree());
        return false;
        break;
    }

    // round the solution for eventual numeric errors
    for (int i = 0; i < sols.size(); ++i)
    {
        sols[i].roundNear();
        // To avoid having -0 as it is just 0
        if (sols[i].isZero())
            sols[i] = 0;
    }

    std::sort(sols.begin(), sols.end() /*, std::greater<>()*/);
    sols.erase(std::unique(sols.begin(), sols.end()), sols.end());

    m_solution = "";
    for (auto& d : sols)
    {
        // check it is not weird rational
        m_solution = m_solution + std::format("{} = {}, ", for_symbol, d);
    }

    m_solution.pop_back();
    m_solution.pop_back();
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Solver::solve(AST& ast, const std::string_view for_symbol)
{
    m_solution = "";
    if (!ast.has_symbol(for_symbol))
    {
        std::cerr << std::format("ERROR: Symbol to solve for '{}' not found in {}\n", for_symbol, ast.to_string());
        return false;
    }

    if (!ast.isEquation())
    {
        std::cerr << std::format("ERROR: {} is not an equation!\n", ast.to_string());
        return false;
    }

    if (!ast.convertToExpression())
    {
        std::cerr << std::format("ERROR: {} unable to convert to expression\n", ast.to_string());
        return false;
    }

    // the operator here was = as it is an equation, converted to an expression LHS - RHS (= 0)
    if (!solve_equation_(ast.getRoot(), for_symbol))
    {
        std::cerr << std::format("ERROR: unable to solve equation: [{}, {}]\n", ast.to_string(), for_symbol);
        return false;
    }

    return true;
}
