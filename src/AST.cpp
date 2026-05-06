#include "AST.hpp"

#include <cassert>
#include <iostream>
#include <format>
#include <sstream>

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

        ss << uni->token.value;
        to_string_(uni->n.get(), ss, level + 1);

        if (level > 0)
            ss << ")";
    }
    else if (auto bin = dynamic_cast<const NodeBin*>(node))
    {
        auto l = level;

        if (level > 0)
            ss << "(";

        switch (bin->token.type)
        {
            using enum eTOKENS;

        default:
            l++;
            to_string_(bin->l.get(), ss, l);
            ss << std::format(" {} ", bin->token.value);
            break;
        case POW_OP:
            l++;
            to_string_(bin->l.get(), ss, l);
            ss << std::format("{}", bin->token.value);
            break;
        case EQUAL:
            to_string_(bin->l.get(), ss, l);
            ss << std::format(" {} ", bin->token.value);
            break;
        case COMMA_OP:
            to_string_(bin->l.get(), ss, l);
            ss << std::format("{} ", bin->token.value);
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
        std::cout << std::format("Unary({})\n", uni->token.value);
        print_(uni->n.get(), indent + 1);
        return;
    }

    if (auto bin = dynamic_cast<const NodeBin*>(node))
    {
        pad(indent);
        std::cout << std::format("Binary({})\n", bin->token.value);

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

void AST::setRoot(std::unique_ptr<INode>& root)
{
    m_pRoot = std::move(root);
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
