#include "AST.hpp"

#include <cassert>

AST::Node::Node(const Token& token) : token(token)
{
}

AST::AST()
{
    m_pRoot    = std::make_unique<Node>();
    m_pCurrent = m_pRoot.get();
}

AST::~AST()
{
}

void AST::add_left(const Token& token)
{
    // assert(pNode != nullptr);
    assert(m_pCurrent != nullptr);
    assert(m_pCurrent->l != nullptr);

    m_pCurrent->l = std::make_unique<Node>(token);
}

void AST::add_right(const Token& token)
{
    // assert(pNode != nullptr);
    assert(m_pCurrent != nullptr);
    assert(m_pCurrent->r != nullptr);

    m_pCurrent->r = std::make_unique<Node>(token);
}
