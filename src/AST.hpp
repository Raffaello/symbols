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
};

struct LeafSymbol : public INode
{
    std::string value;
};

struct NodeBin : public INode
{
    Token token;

    std::unique_ptr<INode> l = nullptr;
    std::unique_ptr<INode> r = nullptr;
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

    /**
     * TODO: remove / use for debug only
     */
    void print();

    // TODO: still missing how to navigate the AST...
};
