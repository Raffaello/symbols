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
    // TODO: avoid to const_cast, but for now is ok
    auto node_ = const_cast<AST::INode*>(node);
    auto bin   = dynamic_cast<AST::NodeBin*>(node_);
    if (bin->op != AST::eOperators::EQUAL)
        return false;

    // LHS - RHS = 0
    // expr: LHS - RHS
    std::unique_ptr<AST::INode> n    = AST::NodeBin::make(AST::eOperators::SUB, std::move(bin->l), std::move(bin->r));
    auto                        nbin = dynamic_cast<AST::NodeBin*>(n.get());

    PolynomialForm pf(m_pSymbolTable);
    bool           res = pf.analyze(n.get(), std::string(for_symbol));
    bin->l             = std::move(nbin->l);
    bin->r             = std::move(nbin->r);

    if (!res)
        return false;

    std::vector<mp_t> sols;
    switch (pf.degree())
    {
    case -1:
        return false;

    case 0:    // no variables
        if (pf[0] == 0)
            m_solution = std::format("inf solutions");
        else
            m_solution = std::format("no solution");

        return true;

    case 1:    // linear
        sols.emplace_back(-pf[0] / pf[1]);
        break;
    case 2:
    {
        const mp_t a = pf[2];
        const mp_t b = pf[1];
        const mp_t c = pf[0];

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
    break;

    case 3:
    {
        // Cardano's formula
        const mp_t a = pf[2] / pf[3];
        const mp_t b = pf[1] / pf[3];
        const mp_t c = pf[0] / pf[3];

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

    // the operator here is = as it is an equation
    if (!solve_equation_(ast.getRoot(), for_symbol))
    {
        std::cerr << std::format("ERROR: unable to solve equation: [{}, {}]\n", ast.to_string(), for_symbol);
        return false;
    }

    return true;
}
