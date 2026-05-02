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
public:
    // struct Node
    // {
    //     Token token;

    // std::unique_ptr<Node> l = nullptr;
    // std::unique_ptr<Node> r = nullptr;

    // Node() = default;
    // Node(const Token& token);
    // };

private:
    std::unique_ptr<INode> m_pRoot    = nullptr;
    INode*                 m_pCurrent = nullptr;

    void print_(const INode* node, const int indent);

public:
    AST()  = default;
    ~AST() = default;

    void setRoot(std::unique_ptr<INode>& root);
    // void setCurrentToken(const Token& token);
    // void setLeft(std::unique_ptr<BinaryAST::Node> node);
    // void setRight(std::unique_ptr<BinaryAST::Node> node);

    // void add_left(const Token& token);
    // void add_right(const Token& token);
    // void move_left();
    // void move_right();

    /**
     * TODO: remove / use for debug only
     */
    void print();
};
