#include "Solver.hpp"
#include "formatters.hpp"
#include "multi_precision.hpp"


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

    std::vector<ast_num_t> sols;
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
        const ast_num_t a = pf[2];
        const ast_num_t b = pf[1];
        const ast_num_t c = pf[0];

        const ast_num_t delta = (b * b) - (4.0 * a * c);

        if (delta < 0.0)
        {
            m_solution = "no real solutions, complex roots not supported yet";
            return true;
        }

        const ast_num_t sq_delta = mp_sqrt(delta);
        const ast_num_t a2       = 2.0 * a;
        // sol 1
        sols.emplace_back((-b + sq_delta) / a2);
        // sol 2
        if (delta != 0.0)
            sols.emplace_back((-b - sq_delta) / a2);
    }
    break;

    case 3:
    {
        // Cardano's formula
        const ast_num_t a = pf[2] / pf[3];
        const ast_num_t b = pf[1] / pf[3];
        const ast_num_t c = pf[0] / pf[3];

        const ast_num_t aa = a * a;
        const ast_num_t p  = b - aa / 3.0;
        const ast_num_t q  = 2.0 * a * aa / 27.0 - a * b / 3.0 + c;

        const ast_num_t p3    = p * p * p;
        const ast_num_t delta = (q * q) / 4.0 + p3 / 27.0;

        const ast_num_t a_3 = a / 3.0;
        const ast_num_t q_2 = q / 2.0;

        if (delta > 0.0)
        {
            // one real solution, two complex
            const ast_num_t sq_delta = mp_sqrt(delta);

            const ast_num_t u = mp_cbrt(ast_num_t(-q_2 + sq_delta));
            const ast_num_t v = mp_cbrt(ast_num_t(-q_2 - sq_delta));
            const ast_num_t y = u + v;

            sols.emplace_back(y - (a_3));
        }
        // else if (mp::fabs(delta) < MPFR_EPSILON)    // if delta is zero (approx.)
        else if (mp_isZero(delta))
        {
            const ast_num_t u = mp_cbrt(ast_num_t(-q_2));
            sols.emplace_back((2.0 * u) - a_3);
            sols.emplace_back((-u) - a_3);
        }
        else    // if (delta < 0.0)
        {
            const mp::mpfr_float PI = std::numbers::pi_v<double>;

            const ast_num_t r     = 2.0 * mp_sqrt(-p / 3.0);
            const ast_num_t denom = mp_sqrt(-p3 / 27.0);
            ast_num_t       z     = -q_2 / denom;
            z                     = mp_clamp(z, ast_num_t(-1), ast_num_t(+1));

            const mp::mpfr_float a_3f = a_3;
            const mp::mpfr_float phi  = mp::acos(mp::mpfr_float(z));

            sols.emplace_back(r * mp::cos(phi / 3.0) - a / 3.0);
            sols.emplace_back(r * mp::cos((phi + 2.0 * PI) / 3.0) - a_3f);
            sols.emplace_back(r * mp::cos((phi + 4.0 * PI) / 3.0) - a_3f);
        }
    }
    break;

    default:
        // Newton's method
        // todo
        std::cout << std::format("Not implemented to solve polynomial of degree {}\n", pf.degree());
        return false;
        break;
    }

    // round the solution for eventual numeric errors
    for (int i = 0; i < sols.size(); ++i)
    {
        const ast_num_t near = mp_roundNear(sols[i]);
        if (mp_isZero(sols[i] - near))
            sols[i] = near;

        // To avoid having -0 as it is just 0
        if (sols[i] == 0.0)
            sols[i] = mp::abs(sols[i]);
    }

    std::sort(sols.begin(), sols.end() /*, std::greater<>()*/);
    sols.erase(std::unique(sols.begin(), sols.end()), sols.end());

    m_solution = "";
    for (auto& d : sols)
        m_solution = m_solution + std::format("{} = {}, ", for_symbol, d);

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
        std::cerr << std::format("Symbol to solve for '{}' not found in {}\n", for_symbol, ast.to_string());
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
