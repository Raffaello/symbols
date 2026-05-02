#include "AST.hpp"

#include <cassert>

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

void AST::setRoot(std::unique_ptr<INode>& root)
{
    m_pRoot = std::move(root);
}
