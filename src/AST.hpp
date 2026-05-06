#pragma once

#include "Token.hpp"

#include <memory>
#include <list>
#include <sstream>

struct INode
{
    virtual ~INode() = default;

    // virtual double eval() = 0; // TODO: need to find a way to pass the symbol table or to be aware of te symbol table
};

struct LeafNum : public INode
{
    double value;
};

struct LeafSymbol : public INode
{
    std::string value;
};

struct NodeUnary : public INode
{
    Token                  token;
    std::unique_ptr<INode> n = nullptr;
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

    void to_string_(const INode* node, std::stringstream& ss, const int level) const;
    void print_(const INode* node, const int indent);

public:
    AST()  = default;
    ~AST() = default;

    void                setRoot(std::unique_ptr<INode>& root);
    inline const INode* getRoot() const noexcept;

    std::string to_string() const;

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
