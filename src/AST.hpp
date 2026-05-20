#pragma once

#include "mp_t.hpp"

#include <memory>
#include <list>
#include <sstream>

#include <boost/multiprecision/mpfr.hpp>

class AST
{
public:
    enum class eOperators
    {
        NONE,    // as a placeholder if not init, this is basically an error if it is encountered
        ADD,
        SUB,
        MUL,
        DIV,
        POW,
        EQUAL,
    };

    struct INode
    {
        virtual ~INode() = default;

        inline bool is_expr() const noexcept;
        inline bool is_unary() const noexcept;
        inline bool is_symbol() const noexcept;
        inline bool is_symbol(const std::string_view symbol) const noexcept;
        inline bool is_num() const noexcept;
    };

    struct LeafNum : public INode
    {
        ast_num_t value;

        static std::unique_ptr<INode> make(const ast_num_t value)
        {
            auto n   = std::make_unique<LeafNum>();
            n->value = value;
            return std::move(n);
        }

        static bool getValue(const INode* pNode, ast_num_t& value)
        {
            if (auto pNum = dynamic_cast<const LeafNum*>(pNode))
            {
                value = pNum->value;
                return true;
            }

            return false;
        }

        static bool setValue(INode* pNode, const ast_num_t& value)
        {
            if (auto pNum = dynamic_cast<LeafNum*>(pNode))
            {
                pNum->value = value;
                return true;
            }

            return false;
        }
    };

    struct LeafSymbol : public INode
    {
        std::string value;

        static std::unique_ptr<INode> make(const std::string& value)
        {
            auto n   = std::make_unique<LeafSymbol>();
            n->value = value;
            return std::move(n);
        }

        static const char* getValue(const INode* pNode)
        {
            if (auto pSym = dynamic_cast<const LeafSymbol*>(pNode))
                return pSym->value.c_str();

            return nullptr;
        }
    };

    struct NodeUnary : public INode
    {
        bool                   negate = false;
        std::unique_ptr<INode> n      = nullptr;

        inline const char value() const noexcept { return negate ? '-' : '+'; }

        static std::unique_ptr<NodeUnary> make(const bool negate, std::unique_ptr<INode> node)
        {
            auto n    = std::make_unique<NodeUnary>();
            n->negate = negate;
            n->n      = std::move(node);
            return std::move(n);
        }
    };

    struct NodeBin : public INode
    {
        eOperators op = eOperators::NONE;

        std::unique_ptr<INode> l = nullptr;
        std::unique_ptr<INode> r = nullptr;

        static std::unique_ptr<NodeBin> make(const eOperators op, std::unique_ptr<INode> l, std::unique_ptr<INode> r)
        {
            auto n = std::make_unique<NodeBin>();
            n->op  = op;
            n->l   = std::move(l);
            n->r   = std::move(r);
            return std::move(n);
        }
    };

private:
    std::unique_ptr<INode> m_pRoot = nullptr;

    static std::unique_ptr<AST::INode> clone_(const INode* pNode);
    static void                        to_string_(const INode* node, std::stringstream& ss, const int level);
    static void                        print_(const INode* node, const int indent);
    static bool                        has_symbol_(const AST::INode* node, const std::string_view symbol);
    static bool                        updateNode_(const std::unique_ptr<AST::INode>* pCurNode, const INode* pNode, std::unique_ptr<INode>& pNodeUpdate);

public:
    AST()  = default;
    ~AST() = default;

    AST(AST&&) noexcept   = default;
    AST& operator=(AST&&) = default;

    AST(const AST& other);
    AST& operator=(const AST& other);

    inline bool         isEquation() const noexcept;
    inline const INode* getRoot() const noexcept;
    void                setRoot(std::unique_ptr<INode> root);
    bool                has_symbol(const std::string_view symbol) const noexcept;

    bool updateNode(const INode* node, std::unique_ptr<INode>& updated_node);

    std::unique_ptr<AST::INode>   cloneRoot() const;
    static std::unique_ptr<INode> clone(const INode* pNode);

    std::string to_string() const;
    void        print();

    static char operator_to_string(const eOperators op);
};

inline bool AST::isEquation() const noexcept
{
    if (auto bin = dynamic_cast<const AST::NodeBin*>(getRoot()))
        return bin->op == AST::eOperators::EQUAL;

    return false;
}

inline bool AST::INode::is_expr() const noexcept
{
    if (auto bin = dynamic_cast<const AST::NodeBin*>(this))
        return bin->op == AST::eOperators::ADD ||
               bin->op == AST::eOperators::SUB ||
               bin->op == AST::eOperators::MUL ||
               bin->op == AST::eOperators::DIV ||
               bin->op == AST::eOperators::POW;

    return false;
}

inline bool AST::INode::is_unary() const noexcept
{
    return dynamic_cast<const AST::NodeUnary*>(this) != nullptr;
}

inline bool AST::INode::is_symbol() const noexcept
{
    return dynamic_cast<const AST::LeafSymbol*>(this) != nullptr;
}

inline bool AST::INode::is_symbol(const std::string_view symbol) const noexcept
{
    if (auto sym = dynamic_cast<const AST::LeafSymbol*>(this))
    {
        if (sym->value == symbol)
            return true;
    }

    return false;
}

inline bool AST::INode::is_num() const noexcept
{
    return dynamic_cast<const AST::LeafNum*>(this);
}

inline const AST::INode* AST::getRoot() const noexcept
{
    return m_pRoot.get();
}
