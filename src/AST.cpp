#include "AST.hpp"

#include <cassert>
#include <iostream>
#include <format>

void AST::to_string_(const INode* node, std::stringstream& ss, const int level) const
{
    if (auto num = dynamic_cast<const LeafNum*>(node))
        ss << num->value;
    else if (auto sym = dynamic_cast<const LeafSymbol*>(node))
        ss << sym->value;
    else if (auto uni = dynamic_cast<const NodeUnary*>(node))
    {
        if (level > 0)
            ss << "(";

        ss << uni->value();
        to_string_(uni->n.get(), ss, level + 1);

        if (level > 0)
            ss << ")";
    }
    else if (auto bin = dynamic_cast<const NodeBin*>(node))
    {
        auto l = level;

        if (level > 0)
            ss << "(";

        switch (bin->op)
        {
            using enum eOperators;

        default:
            l++;
            to_string_(bin->l.get(), ss, l);
            ss << std::format(" {} ", AST::operator_to_string(bin->op));
            break;
        case POW:
            l++;
            to_string_(bin->l.get(), ss, l);
            ss << std::format("{}", AST::operator_to_string(bin->op));
            break;
        case EQUAL:
            to_string_(bin->l.get(), ss, l);
            ss << std::format(" {} ", AST::operator_to_string(bin->op));
            break;
        }

        to_string_(bin->r.get(), ss, l);
        if (level > 0)
            ss << ")";
    }
    else
        throw std::runtime_error("invalid AST");
}

void AST::print_(const INode* node, const int indent)
{
    auto pad = [&](int n) {
        for (int i = 0; i < n; i++)
            std::cout << "  ";
    };

    if (auto num = dynamic_cast<const LeafNum*>(node))
    {
        pad(indent);
        std::cout << std::format("Number({})\n", num->value);
        return;
    }

    if (auto sym = dynamic_cast<const LeafSymbol*>(node))
    {
        pad(indent);
        std::cout << std::format("Symbol({})\n", sym->value);
        return;
    }

    if (auto uni = dynamic_cast<const NodeUnary*>(node))
    {
        pad(indent);
        std::cout << std::format("Unary({})\n", uni->value());
        print_(uni->n.get(), indent + 1);
        return;
    }

    if (auto bin = dynamic_cast<const NodeBin*>(node))
    {
        pad(indent);
        std::cout << std::format("Binary({})\n", AST::operator_to_string(bin->op));

        pad(indent);
        std::cout << "Left:\n";
        print_(bin->l.get(), indent + 1);

        pad(indent);
        std::cout << "Right:\n";
        print_(bin->r.get(), indent + 1);
        return;
    }

    pad(indent);
    std::cout << "<Unknown node>\n";
}

bool AST::has_symbol_(const AST::INode* node, const std::string_view symbol) const noexcept
{
    if (node->is_symbol(symbol))
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

/////////////////////////////////////////////////////////////////////////////////////////////////

void AST::setRoot(std::unique_ptr<INode>& root)
{
    m_pRoot = std::move(root);
}

bool AST::has_symbol(const std::string_view symbol) const noexcept
{
    return has_symbol_(getRoot(), symbol);
}

std::string AST::to_string() const
{
    std::stringstream ss;

    to_string_(m_pRoot.get(), ss, 0);
    return ss.str();
}

void AST::print()
{
    print_(m_pRoot.get(), 0);
}

char AST::operator_to_string(const eOperators op)
{
    switch (op)
    {
        using enum eOperators;

    default:
        [[fallthrough]];
    case NONE:
        std::cerr << "ERROR: unknown operator\n";
        return 0;

    case ADD:
        return '+';
    case SUB:
        return '-';
    case MUL:
        return '*';
    case DIV:
        return '/';
    case POW:
        return '^';
    case EQUAL:
        return '=';
    }
}
