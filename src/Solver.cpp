#include "Solver.hpp"

#include <iostream>
#include <format>
#include <cmath>
#include <stdexcept>

Solver::Solver(const std::shared_ptr<SymbolTable>& pSymbolTable) : m_pSymbolTable(pSymbolTable)
{
    if (m_pSymbolTable == nullptr)
        throw std::invalid_argument("symbol table is null");
}

Solver::PolynomialForm Solver::analyze_poly_(const AST::INode* node, std::string_view symbol) const noexcept
{
    // coeffs are stored in reverse order
    PolynomialForm pf{
        .degree = 0,
        .coeffs = {},
    };

    if (!collect_poly_(node, pf.coeffs, symbol))
    {
        pf.degree = 0;
        pf.coeffs.clear();
        return pf;
    }

    pf.degree = static_cast<int>(pf.coeffs.size()) - 1;
    return pf;
}

bool Solver::collect_poly_(const AST::INode* node, std::vector<double>& coeffs, std::string_view symbol)
{
    if (is_num_(node))
    {
        double d = 0.0;
        if (!AST::LeafNum::getValue(node, d))
        {
            std::cerr << "ERROR: unable to get num\n";
            return false;
        }

        if (coeffs.size() < 1)
            coeffs.push_back(d);
        else
            coeffs[0] += d;

        return true;
    }
    else if (is_symbol_(node))
    {
        // if (std::string(symbol) == AST::LeafSymbol::getValue(node))
        // {
        //     // coeffs_x
        // }

        switch (coeffs.size())
        {
        case 0:
            coeffs.push_back(0);
            [[fallthrough]];
        case 1:
            coeffs.push_back(1);
            break;

        case 2:
            [[fallthrough]];
        default:
            coeffs[1] += 1;
            break;
        }

        return true;
    }
    else if (is_unary_(node))
    {
        if (auto uny = dynamic_cast<const AST::NodeUnary*>(node))
        {
            if (uny->negate)
            {
                std::vector<double> coeffs2;
                if (!collect_poly_(uny->n.get(), coeffs2, symbol))
                    return false;

                if (coeffs2.size() > coeffs.size())
                    coeffs.resize(coeffs2.size());

                for (size_t i = 0; i < coeffs2.size(); ++i)
                    coeffs[i] -= coeffs2[i];
            }

            return true;
        }
    }
    else if (is_expr_(node))
    {
        if (auto expr = dynamic_cast<const AST::NodeBin*>(node))
        {
            switch (expr->op)
            {
                using enum AST::eOperators;

            case ADD:
                [[fallthrough]];
            case SUB:
            {
                // std::vector<double> coeffs2;
                if (!collect_poly_(expr->l.get(), coeffs, symbol))
                    return false;
                if (!collect_poly_(expr->r.get(), coeffs, symbol))
                    return false;

                // if (coeffs2.size() > coeffs.size())
                //     coeffs.resize(coeffs2.size());
                // for (size_t i = 0; i < coeffs2.size(); ++i)
                //     coeffs[i] += coeffs2[i];
                return true;
            }

            // case SUB:
            // {
            //     // std::vector<double> coeffs2;
            //     if (!collect_poly_(expr->l.get(), coeffs, symbol))
            //         return false;
            //     if (!collect_poly_(expr->r.get(), coeffs, symbol))
            //         return false;

            // // if (coeffs2.size() > coeffs.size())
            //     // coeffs.resize(coeffs2.size());
            // // for (size_t i = 0; i < coeffs2.size(); ++i)
            //     // coeffs[i] -= coeffs2[i];

            // return true;
            // }
            case MUL:
            {
                const AST::INode* l;
                const AST::INode* r;
                if (is_num_(expr->l.get()))
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
                    if (coeffs.size() == 0)
                        coeffs.push_back(dlr);
                    else
                        coeffs[0] += dlr;

                    return true;
                }
                // e.g. 2*x | x*2
                if (is_num_(l) && is_symbol_(r))
                {
                    double d;
                    if (!AST::LeafNum::getValue(l, d))
                    {
                        std::cerr << "ERROR: unable to get number\n";
                        return false;
                    }

                    switch (coeffs.size())
                    {
                    case 0:
                        coeffs.push_back(0);
                        [[fallthrough]];
                    case 1:
                        coeffs.push_back(d);
                        break;
                    default:
                    case 2:
                        coeffs[1] += d;
                    }

                    return true;
                }

                if (is_expr_(l))
                    if (!collect_poly_(l, coeffs, symbol))
                        return false;

                if (is_expr_(r))
                    if (!collect_poly_(r, coeffs, symbol))
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

                    // if(dr == 0.0)
                    // {
                    // division by zero
                    // }
                    const double dlr = dl / dr;
                    if (coeffs.size() == 0)
                        coeffs.push_back(dlr);
                    else
                        coeffs[0] += dlr;

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

                    if (!collect_poly_(expr->l.get(), coeffs, symbol))
                        return false;

                    for (auto& c : coeffs)
                        c /= d;

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
                    //     coeffs[1] += d;
                    // }

                    return true;
                }

                // if (is_expr_(l))
                //     if (!collect_poly_(l, coeffs, symbol))
                //         return false;

                // if (is_expr_(r))
                //     if (!collect_poly_(r, coeffs, symbol))
                //         return false;
            }
            break;
            case POW:
                // TODO
                break;
            }
        }
    }

    return false;
}

bool Solver::has_symbol_(const AST::INode* node, const std::string_view symbol) const noexcept
{
    if (is_symbol_(node, symbol))
        return true;
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

std::unique_ptr<AST::INode> Solver::simplify_(std::unique_ptr<AST::INode>& node)
{
    if (is_num_(node.get()) || is_symbol_(node.get()))
        return std::move(node);

    if (auto bin = dynamic_cast<AST::NodeBin*>(node.get()))
    {
        auto l = simplify_(bin->l);
        auto r = simplify_(bin->r);

        return simplifyExpr_(node);
    }
    else if (auto uni = dynamic_cast<AST::NodeUnary*>(node.get()))
    {
        if (!uni->negate)
            return std::move(uni->n);
    }


    throw std::runtime_error("?");
}

std::unique_ptr<AST::INode> Solver::simplifyExpr_(std::unique_ptr<AST::INode>& node)
{
    auto bin = dynamic_cast<AST::NodeBin*>(node.get());
    if (bin == nullptr)
        throw std::invalid_argument("simplifyExpr_ called without a NodeBin");

    switch (bin->op)
    {
        using enum AST::eOperators;

    case ADD:
        [[fallthrough]];
    case SUB:
        [[fallthrough]];
    case MUL:
        [[fallthrough]];
    case DIV:
        [[fallthrough]];
    case POW:
        return std::move(simplifyExprSumOrMulOrPow_(node));
    }

    std::cerr << std::format("ERROR: unable to simplify expr with operator: {}\n", AST::operator_to_string(bin->op));
    return nullptr;
}

std::unique_ptr<AST::INode> Solver::simplifyExprSumOrMulOrPow_(std::unique_ptr<AST::INode>& node)
{
    auto bin = dynamic_cast<AST::NodeBin*>(node.get());
    if (bin == nullptr)
        throw std::invalid_argument("simplifyExprSumOrMulOrPow_: node is not binary");

    // both are numbers
    if (is_num_(bin->l.get()) && is_num_(bin->r.get()))
    {
        double l;
        double r;
        if (!AST::LeafNum::getValue(bin->l.get(), l) || AST::LeafNum::getValue(bin->r.get(), r))
        {
            std::cerr << std::format("ERROR: unable to extract number\n");
            return nullptr;
        }

        auto n = AST::LeafNum::make(0);
        switch (bin->op)
        {
            using enum AST::eOperators;

        case ADD:
            n->value = l + r;
            break;
        case SUB:
            n->value = l - r;
            break;
        case MUL:
            n->value = l * r;
            break;
        case DIV:
            n->value = l / r;
            break;
        case POW:
            n->value = std::pow(l, r);
            break;

        default:
            std::cerr << std::format("ERROR: unable to simplify expression {} {} {}", l, AST::operator_to_string(bin->op), r);
            return nullptr;
        }

        return std::move(n);
    }

    // both are the same symbol
    if (is_symbol_(bin->l.get()) && is_symbol_(bin->r.get()))
    {
        const char* l = AST::LeafSymbol::getValue(bin->l.get());
        const char* r = AST::LeafSymbol::getValue(bin->r.get());
        if (l == nullptr || r == nullptr)
        {
            std::cerr << std::format("ERROR: unable to extract symbol\n");
            return nullptr;
        }

        // if are not the same just return
        if (std::string(l) != std::string(r))
            return std::move(node);

        // same symbol for e.g:
        // x+x => 2*x
        // x-x => 0
        // x*x => x^2
        // x/x => 1
        // x^x =>
        auto n = AST::LeafNum::make(0);
        switch (bin->op)
        {
            using enum AST::eOperators;

        case ADD:
            n->value = 2;
            bin->op  = AST::eOperators::MUL;
            bin->l   = std::move(n);
            return std::move(node);
        case SUB:
            n->value = 0;
            return std::move(n);
            break;
        case MUL:
            n->value = 2;
            bin->op  = AST::eOperators::POW;
            bin->r   = std::move(n);
            return std::move(node);
        case DIV:
            n->value = 1;
            return std::move(n);
        case POW:
            return std::move(node);

        default:
            std::cerr << std::format("ERROR: unable to simplify expression {} {} {}", l, AST::operator_to_string(bin->op), r);
            return nullptr;
        }
    }

    // otherwise is one symbol and one number e.g : x+1
    return std::move(node);
}

bool Solver::solve_equation_(AST::INode* node, const std::string_view for_symbol)
{
    auto bin = dynamic_cast<AST::NodeBin*>(node);
    if (bin->op != AST::eOperators::EQUAL)
        return false;

    // LHS - RHS = 0
    // expr: LHS - RHS
    std::unique_ptr<AST::INode> n = AST::NodeBin::make(AST::eOperators::SUB, std::move(bin->l), std::move(bin->r));


    auto pf = analyze_poly_(n.get(), for_symbol);

    switch (pf.degree)
    {
    case 0:    // no variables
        if (is_num_(bin->l.get()) && is_num_(bin->r.get()))
        {
            double l;
            double r;

            if (!AST::LeafNum::getValue(bin->l.get(), l) || !AST::LeafNum::getValue(bin->r.get(), r))
            {
                std::cerr << "ERROR: unable to extract numbers\n";
                return false;
            }

            if (std::fabs(l - r) < 1e-6)
                m_solution = std::format("inf solutions");
            else
                m_solution = std::format("no solution");

            return true;
        }
        break;
    case 1:    // linear
        m_solution = std::format("{} = {}", for_symbol, -pf.coeffs[0] / pf.coeffs[1]);
        return true;

    case 2:
        // todo
        break;
    default:
        // todo
        break;
    }

    return false;


    // auto res = solve_expr_(n, for_symbol);


    // return res.has_value() && res.value();
}

std::optional<bool> Solver::solve_expr_(std::unique_ptr<AST::INode>& node, const std::string_view for_symbol)
{
    node = simplify_(node);

    if (auto bin = dynamic_cast<AST::NodeBin*>(node.get()))
    {
        if (bin->op == AST::eOperators::EQUAL)
            return false;

        if (is_expr_(bin->l.get()))
        {
            if (!solve_expr_(bin->l, for_symbol))
                return false;
        }

        if (is_expr_(bin->r.get()))
        {
            if (!solve_expr_(bin->r, for_symbol))
                return false;
        }

        if (is_unary_(bin->l.get()))
        {
            if (!solve_unary_(bin->l, for_symbol))
                return false;
        }

        if (is_unary_(bin->r.get()))
        {
            if (!solve_unary_(bin->r, for_symbol))
                return false;
        }

        if (is_num_(bin->l.get()) && is_num_(bin->r.get()))
        {
            switch (bin->op)
            {
                using enum AST::eOperators;

            default:
                [[fallthrough]];
            case NONE:
                return false;

            case ADD:
            {
                auto n   = std::make_unique<AST::LeafNum>();
                n->value = dynamic_cast<const AST::LeafNum*>(bin->l.get())->value + dynamic_cast<const AST::LeafNum*>(bin->r.get())->value;
                node     = std::move(n);
            }
            break;
            case SUB:
            {
                auto n   = std::make_unique<AST::LeafNum>();
                n->value = dynamic_cast<const AST::LeafNum*>(bin->l.get())->value - dynamic_cast<const AST::LeafNum*>(bin->r.get())->value;
                node     = std::move(n);
            }
            break;
            case MUL:
            {
                auto n   = std::make_unique<AST::LeafNum>();
                n->value = dynamic_cast<const AST::LeafNum*>(bin->l.get())->value * dynamic_cast<const AST::LeafNum*>(bin->r.get())->value;
                node     = std::move(n);
            }
            break;
            case DIV:
            {
                auto n   = std::make_unique<AST::LeafNum>();
                n->value = dynamic_cast<const AST::LeafNum*>(bin->l.get())->value / dynamic_cast<const AST::LeafNum*>(bin->r.get())->value;
                node     = std::move(n);
            }
            break;
            case POW:
            {
                auto n   = std::make_unique<AST::LeafNum>();
                n->value = std::pow(dynamic_cast<const AST::LeafNum*>(bin->l.get())->value, dynamic_cast<const AST::LeafNum*>(bin->r.get())->value);
                node     = std::move(n);
            }
            break;
            }

            return true;
        }
        else
        {
            // This case l and r are not an expression nor a num
            // can be unary or symbols or mixed with number
            // -1 + a  ==> should be rewritten as a - 1
            // or 1 + a ==> can do nothing, this form is final
            // or 2 * x ==> need to check if it is asymbol here
            // return true;

            if (is_symbol_(bin->l.get(), for_symbol))
            {
                // e.g: x + 1
                // (x * (x + 1)) ==> how to resolve this?
            }
            else if (is_symbol_(bin->r.get(), for_symbol))
            {
                // e.g 1 + x
            }
            else
                return std::nullopt;

            // TODO: simplify symbols and so on...
            // return false;
        }
    }

    return false;
}

bool Solver::solve_unary_(std::unique_ptr<AST::INode>& node, const std::string_view for_symbol)
{
    if (auto uni = dynamic_cast<AST::NodeUnary*>(node.get()))
    {
        if (!uni->negate)
        {
            node = std::move(uni->n);
            return true;
        }

        // '-'
        if (is_num_(uni->n.get()))
        {
            auto n   = std::make_unique<AST::LeafNum>();
            n->value = -dynamic_cast<AST::LeafNum*>(uni->n.get())->value;
            node     = std::move(n);

            return true;
        }
        else if (is_unary_(uni->n.get()))
            return solve_unary_(uni->n, for_symbol);
        else if (is_symbol_(uni->n.get()))
        {
            // TODO: if it is -x must multiply everything else for -1
            return true;
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Solver::solve(AST& ast, const std::string_view for_symbol)
{
    m_solution = "";
    if (!has_symbol_(ast.getRoot(), for_symbol))
    {
        std::cerr << std::format("Symbol to solve for '{}' not found!\n", for_symbol);
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
