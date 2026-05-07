#pragma once

#include "Token.hpp"

#include <memory>
#include <list>

struct INode
{
    virtual ~INode() = default;
};

struct LeafNum : public INode
{
    double value;

    static std::unique_ptr<LeafNum> make(const double value)
    {
        auto n   = std::make_unique<LeafNum>();
        n->value = value;
        return std::move(n);
    }
};

struct LeafSymbol : public INode
{
    std::string value;

    static std::unique_ptr<LeafSymbol> make(const std::string& value)
    {
        auto n   = std::make_unique<LeafSymbol>();
        n->value = value;
        return std::move(n);
    }
};

struct NodeUnary : public INode
{
    Token                  token;
    std::unique_ptr<INode> n = nullptr;

    static std::unique_ptr<NodeUnary> make(const Token& token)
    {
        auto n   = std::make_unique<NodeUnary>();
        n->token = token;
        n->n     = nullptr;
        return std::move(n);
    }
};

struct NodeBin : public INode
{
    Token token;

    std::unique_ptr<INode> l = nullptr;
    std::unique_ptr<INode> r = nullptr;

    static std::unique_ptr<NodeBin> make(const Token& token, std::unique_ptr<INode>& l, std::unique_ptr<INode>& r)
    {
        auto n   = std::make_unique<NodeBin>();
        n->token = token;
        n->l     = std::move(l);
        n->r     = std::move(r);
        return std::move(n);
    }
};

class AST
{
private:
    std::unique_ptr<INode> m_pRoot    = nullptr;
    INode*                 m_pCurrent = nullptr;

    void print_(const INode* node, const int indent);

public:
    AST()  = default;
    ~AST() = default;

    void setRoot(std::unique_ptr<INode>& root);

    inline const INode* getRoot() const noexcept;

    /**
     * TODO: remove / use for debug only
     */
    void print();

    // TODO: still missing how to navigate the AST...
};

inline const INode* AST::getRoot() const noexcept
{
    return m_pRoot.get();
}
