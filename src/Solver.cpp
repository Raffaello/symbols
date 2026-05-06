#include "Solver.hpp"

#include <iostream>
#include <format>
#include <cmath>

bool Solver::has_symbol_(const INode* node, const std::string_view symbol) const noexcept
{
    if (is_symbol_(node, symbol))
        return true;
    else if (auto bin = dynamic_cast<const NodeBin*>(node))
    {
        bool l = has_symbol_(bin->l.get(), symbol);
        if (l)
            return true;

        return has_symbol_(bin->r.get(), symbol);
    }

    return false;
}

bool Solver::is_equation_(const INode* node) const noexcept
{
    if (auto bin = dynamic_cast<const NodeBin*>(node))
        return bin->token.type == eTOKENS::EQUAL;

    return false;
}

bool Solver::is_expr_(const INode* node) const noexcept
{
    if (auto bin = dynamic_cast<const NodeBin*>(node))
        return bin->token.type == eTOKENS::SUM_OP ||
               bin->token.type == eTOKENS::MUL_OP ||
               bin->token.type == eTOKENS::POW_OP;

    return false;
}

bool Solver::is_unary_(const INode* node) const noexcept
{
    if (auto uni = dynamic_cast<const NodeUnary*>(node))
        return true;
    else
        return false;
}

bool Solver::is_symbol_(const INode* node) const noexcept
{
    if (auto sym = dynamic_cast<const LeafSymbol*>(node))
        return true;

    return false;
}

bool Solver::is_symbol_(const INode* node, const std::string_view symbol) const noexcept
{
    if (auto sym = dynamic_cast<const LeafSymbol*>(node))
    {
        if (sym->value == symbol)
            return true;
    }

    return false;
}

bool Solver::is_num_(const INode* node) const noexcept
{
    if (auto num = dynamic_cast<const LeafNum*>(node))
        return true;

    return false;
}

std::unique_ptr<INode> Solver::simplify_(std::unique_ptr<INode>& node)
{
    if (is_num_(node.get()) || is_symbol_(node.get()))
        return std::move(node);

    // if(is_expr_(node.get()))
    if (auto bin = dynamic_cast<NodeBin*>(node.get()))
    {
        auto l = simplify_(bin->l);
        auto r = simplify_(bin->r);

        return simplifyExpr_(l, r, bin->token);
    }
    else if (auto uni = dynamic_cast<NodeUnary*>(node.get()))
    {
        if (uni->token.value[0] == '+')
            return std::move(uni->n);
    }
    else
        throw std::runtime_error("?");
}

std::unique_ptr<INode> Solver::simplifyExpr_(std::unique_ptr<INode>& left, std::unique_ptr<INode>& right, Token& t)
{
    switch (t.type)
    {
    default:
        nullptr;

    case eTOKENS::SUM_OP:
    case eTOKENS::MUL_OP:
    case eTOKENS::POW_OP:
    {
        if (is_num_(left.get()) && is_num_(right.get()))
        {
            auto l = dynamic_cast<LeafNum*>(left.get())->value;
            auto r = dynamic_cast<LeafNum*>(right.get())->value;
            auto n = std::make_unique<LeafNum>();
            switch (t.value[0])
            {
            default:
                throw std::runtime_error("t.value[0]");
                break;
            case '+':
                n->value = l + r;
                break;
            case '-':
                n->value = l - r;
                break;
            case '*':
                n->value = l * r;
                break;
            case '/':
                n->value = l / r;
                break;
            case '^':
                n->value = std::pow(l, r);
                break;
            }

            return std::move(n);
        }

        if (t.type == eTOKENS::SUM_OP)
        {
            if (auto l = dynamic_cast<LeafNum*>(left.get()))
            {
                if (l->value == 0.0)
                    return std::move(right);
            }
            else if (auto r = dynamic_cast<LeafNum*>(right.get()))
            {
                if (r->value == 0.0)
                    return std::move(left);
            }
        }
        else if (t.type == eTOKENS::MUL_OP)
        {
            if (auto l = dynamic_cast<LeafNum*>(left.get()))
            {
                if (l->value == 1.0)
                    return std::move(right);
            }
            else if (auto r = dynamic_cast<LeafNum*>(right.get()))
            {
                if (r->value == 1.0)
                    return std::move(left);
            }
        }
        else if (t.type == eTOKENS::POW_OP)
        {
            if (auto l = dynamic_cast<LeafNum*>(left.get()))    // 1^x
            {
                if (l->value == 1.0)
                    return std::move(left);                           // returning 1.0
            }
            else if (auto r = dynamic_cast<LeafNum*>(right.get()))    // x^1
            {
                if (r->value == 1.0)
                    return std::move(left);    // returning x
            }
        }
    }
    break;
    }

    auto n   = std::make_unique<NodeBin>();
    n->token = t;
    n->l     = std::move(left);
    n->r     = std::move(right);
    return std::move(n);
}

bool Solver::solve_equation_(INode* node, const std::string_view for_symbol)
{
    if (auto node_eq = dynamic_cast<NodeBin*>(node))
    {
        if (node_eq->token.type != eTOKENS::EQUAL)
            return false;

        const INode* l = nullptr;
        const INode* r = nullptr;

        if (is_symbol_(node_eq->l.get(), for_symbol))
        {
            l = node_eq->l.get();
            r = node_eq->r.get();
        }
        else if (is_symbol_(node_eq->r.get(), for_symbol))
        {
            // swap l,r
            auto tmp                         = std::move(const_cast<NodeBin*>(node_eq)->l);
            const_cast<NodeBin*>(node_eq)->l = std::move(const_cast<NodeBin*>(node_eq)->r);
            const_cast<NodeBin*>(node_eq)->r = std::move(tmp);

            l = node_eq->l.get();
            r = node_eq->r.get();
        }

        if (l != nullptr && r != nullptr)
        {
            node_eq->r = simplify_(node_eq->r);
            r          = node_eq->r.get();

            // x = 1 or x = a or x = [expr]
            if (is_num_(r))
            {
                // m_solution << std::format("{} = {}\n", for_symbol, dynamic_cast<const LeafNum*>(r)->value);
                return true;
            }
            else if (is_symbol_(r))
            {
                // m_solution << std::format("{} = {}\n", for_symbol, dynamic_cast<const LeafSymbol*>(r)->value);
                return true;
            }
            else if (is_expr_(r))
            {
                // x = (1+a)
                auto res = solve_expr_(node_eq->r, for_symbol);
                if (!res.has_value())
                {
                    // m_solution << std::format("{} = {}", for_symbol)
                    return true;
                }

                if (res.value())
                    return solve_equation_(node, for_symbol);    // redo it
                else
                    return false;
            }
        }
        else
        {
            // RHS - LHS=0
            auto zero      = std::make_unique<LeafNum>();
            zero->value    = 0.0;
            auto n         = std::make_unique<NodeBin>();
            n->token.type  = eTOKENS::SUM_OP;
            n->token.value = "-";
            n->l           = std::move(node_eq->r);
            n->r           = std::move(node_eq->l);
            node_eq->l     = std::move(n);
            node_eq->r     = std::move(zero);

            node_eq->l = simplify_(node_eq->l);
            auto res   = solve_expr_(node_eq->l, for_symbol);

            if (!res.has_value())
                return true;

            if (res.value())
                return solve_equation_(node, for_symbol);    // redo it
            else
                return false;
        }
    }

    return false;
}

std::optional<bool> Solver::solve_expr_(std::unique_ptr<INode>& node, const std::string_view for_symbol)
{
    if (auto bin = dynamic_cast<NodeBin*>(node.get()))
    {
        if (bin->token.type == eTOKENS::EQUAL)
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
            if (bin->token.value.size() > 1)
                return false;

            switch (bin->token.value[0])
            {
            default:
                return false;

            case '+':
            {
                auto n   = std::make_unique<LeafNum>();
                n->value = dynamic_cast<const LeafNum*>(bin->l.get())->value + dynamic_cast<const LeafNum*>(bin->r.get())->value;
                node     = std::move(n);
            }
            break;
            case '-':
            {
                auto n   = std::make_unique<LeafNum>();
                n->value = dynamic_cast<const LeafNum*>(bin->l.get())->value - dynamic_cast<const LeafNum*>(bin->r.get())->value;
                node     = std::move(n);
            }
            break;
            case '*':
            {
                auto n   = std::make_unique<LeafNum>();
                n->value = dynamic_cast<const LeafNum*>(bin->l.get())->value * dynamic_cast<const LeafNum*>(bin->r.get())->value;
                node     = std::move(n);
            }
            break;
            case '/':
            {
                auto n   = std::make_unique<LeafNum>();
                n->value = dynamic_cast<const LeafNum*>(bin->l.get())->value / dynamic_cast<const LeafNum*>(bin->r.get())->value;
                node     = std::move(n);
            }
            break;
            case '^':
            {
                auto n   = std::make_unique<LeafNum>();
                n->value = std::pow(dynamic_cast<const LeafNum*>(bin->l.get())->value, dynamic_cast<const LeafNum*>(bin->r.get())->value);
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

            return std::nullopt;

            // TODO: simplify symbols and so on...
            // return false;
        }
    }

    return false;
}

bool Solver::solve_unary_(std::unique_ptr<INode>& node, const std::string_view for_symbol)
{
    if (auto uni = dynamic_cast<NodeUnary*>(node.get()))
    {
        if (uni->token.value.size() == 1)
        {
            if (uni->token.value[0] == '+')
            {
                node = std::move(uni->n);
                return true;
            }
            else    // '-'
            {
                if (is_num_(uni->n.get()))
                {
                    auto n   = std::make_unique<LeafNum>();
                    n->value = -dynamic_cast<LeafNum*>(uni->n.get())->value;
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
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Solver::solve(AST& ast, const std::string_view for_symbol)
{
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

    if (!solve_equation_(const_cast<INode*>(ast.getRoot()), for_symbol))
    {
        std::cerr << std::format("ERROR: unable to solve equation [{}, {}]\n", ast.to_string(), for_symbol);
        return false;
    }

    return true;
}
