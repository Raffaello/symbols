#include "AST.hpp"

#include <cassert>
#include <iostream>
#include <format>

// BinaryAST::Node::Node(const Token& token) : token(token)
// {
// }

// AST::BinaryAST()
// {
//     // m_pRoot    = std::make_unique<Node>();
//     // m_pCurrent = m_pRoot.get();
// }

// AST::~BinaryAST()
// {
// }

// void BinaryAST::setCurrentToken(const Token& token)
// {
//     assert(m_pCurrent != nullptr);
//     m_pCurrent->token = token;
// }

// void BinaryAST::setLeft(std::unique_ptr<BinaryAST::Node> node)
// {
//     m_pCurrent->l = std::move(node);
// }

// void BinaryAST::setRight(std::unique_ptr<BinaryAST::Node> node)
// {
//     m_pCurrent->r = std::move(node);
// }

// void BinaryAST::add_left(const Token& token)
// {
//     // assert(pNode != nullptr);
//     assert(m_pCurrent != nullptr);
//     assert(m_pCurrent->l != nullptr);

// m_pCurrent->l = std::make_unique<Node>(token);
// }

// void BinaryAST::add_right(const Token& token)
// {
//     // assert(pNode != nullptr);
//     assert(m_pCurrent != nullptr);
//     assert(m_pCurrent->r != nullptr);

// m_pCurrent->r = std::make_unique<Node>(token);
// }

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

void AST::print()
{
    print_(m_pRoot.get(), 0);
}
