#include "Solver.hpp"

#include <iostream>
#include <format>
#include <cmath>
#include <stdexcept>
#include <cassert>
#include <algorithm>

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

    switch (pf.degree())
    {
    case -1:
        break;
    case 0:    // no variables
        if (pf[0] == 0)
            m_solution = std::format("inf solutions");
        else
            m_solution = std::format("no solution");

        return true;
    case 1:    // linear
    {
        double s = -pf[0] / pf[1];

        // To avoid having -0 as it is just 0
        if (s == 0.0)
            s = std::fabs(s);

        m_solution = std::format("{} = {}", for_symbol, s);
    }
        return true;

    case 2:
    {
        const double a = pf[2];
        const double b = pf[1];
        const double c = pf[0];

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
    case 3:
    {
        // Cardano's formula
        std::vector<double> s;

        const double a = pf[2] / pf[3];
        const double b = pf[1] / pf[3];
        const double c = pf[0] / pf[3];
        // const double d = pf[0] / pf[3];

        const double aa = a * a;
        const double p  = b - aa / 3.0;
        const double q  = 2.0 * a * aa / 27.0 - a * b / 3.0 + c;

        const double p3    = p * p * p;
        const double delta = (q * q) / 4.0 + p3 / 27.0;

        if (delta > 0.0)
        {
            // one real solution, two complex
            const double sq_delta = std::sqrt(delta);
            const double u        = std::cbrt(-q / 2.0 + sq_delta);
            const double v        = std::cbrt(-q / 2.0 - sq_delta);
            const double y        = u + v;

            s.push_back(y - (a / 3.0));
            // return true;
        }
        else if (delta < 0.0)
        {
            const double r   = 2.0 * std::sqrt(-p / 3.0);
            const double phi = std::acos((-q / 2.0) / std::sqrt(-p3 / 27.0));

            s.push_back(r * std::cos(phi / 3.0) - a / 3.0);
            s.push_back(r * std::cos((phi + 2.0 * M_PI) / 3.0) - a / 3.0);
            s.push_back(r * std::cos((phi + 4.0 * M_PI) / 3.0) - a / 3.0);
        }
        else
        {
            const double u = std::cbrt(-q / 2.0);
            s.push_back((2.0 * u) - a / 3.0);
            s.push_back((-u) - a / 3.0);
        }

        // round the solution for eventual numeric errors
        for (int i = 0; i < s.size(); ++i)
        {
            const double near = std::round(s[i]);
            if (std::fabs(s[i] - near) < SOLVER_EPSILON)
                s[i] = near;
        }

        s.erase(std::unique(s.begin(), s.end()), s.end());
        std::sort(s.begin(), s.end() /*, std::greater<>()*/);

        m_solution = "";
        for (auto& d : s)
            m_solution = m_solution + std::format("{} = {}, ", for_symbol, d);

        m_solution.pop_back();
        m_solution.pop_back();
        std::cout << m_solution << "\n";
        return true;
    }
    break;

    default:
        // Newton's method
        // todo
        std::cout << std::format("Not implemented to solve polynomial of degree {}\n", pf.degree());
        return false;
        break;
    }

    return false;
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
