#include "AST.hpp"
#include "formatters.hpp"

#include <cassert>
#include <iostream>
#include <format>

std::unique_ptr<AST::INode> AST::clone_(const INode* pNode)
{
    if (pNode->is_num())
    {
        ast_num_t v;
        if (!LeafNum::getValue(pNode, v))
            return nullptr;

        return LeafNum::make(v);
    }
    else if (pNode->is_symbol())
    {
        return LeafSymbol::make(LeafSymbol::getValue(pNode));
    }
    else if (pNode->is_unary())
    {
        auto pUny = dynamic_cast<const NodeUnary*>(pNode);
        if (pUny == nullptr)
            return nullptr;

        auto pSubNodeClone = clone_(pUny->n.get());
        if (pSubNodeClone == nullptr)
            return nullptr;

        return NodeUnary::make(pUny->negate, std::move(pSubNodeClone));
    }
    else if (pNode->is_binary())
    {
        auto pNodeBin = dynamic_cast<const NodeBin*>(pNode);
        if (pNodeBin == nullptr)
            return nullptr;

        auto pNodeLeftClone = clone_(pNodeBin->l.get());
        if (pNodeLeftClone == nullptr)
            return nullptr;

        auto pNodeRightClone = clone_(pNodeBin->r.get());
        if (pNodeRightClone == nullptr)
            return nullptr;

        return NodeBin::make(pNodeBin->op, std::move(pNodeLeftClone), std::move(pNodeRightClone));
    }
    else
    {
        std::cerr << "ERROR: unknown node type to clone\n";
        return nullptr;
    }
}

void AST::to_string_(const INode* node, std::stringstream& ss, const int level)
{
    if (auto num = dynamic_cast<const LeafNum*>(node))
    {
        if (mp::denominator(num->value) == 1)
            ss << num->value;
        else
            ss << "(" << num->value << ")";
    }
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

bool AST::has_symbol_(const AST::INode* node, const std::string_view symbol)
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

bool AST::updateNode_(const std::unique_ptr<AST::INode>* pCurNode, const INode* pNode, std::unique_ptr<INode>& pNodeUpdate)
{
    if (pCurNode->get() == pNode)
        return true;

    // if num is a leaf
    // if sym is a leaf
    if (auto nodeUny = dynamic_cast<const NodeUnary*>(pCurNode->get()))
    {
        const bool res = updateNode_(&nodeUny->n, pNode, pNodeUpdate);
        if (res)
        {
            auto nu = const_cast<NodeUnary*>(nodeUny);
            nu->n   = std::move(pNodeUpdate);
        }

        return res;
    }
    else if (auto nodeBin = dynamic_cast<const NodeBin*>(pCurNode->get()))
    {
        auto b   = const_cast<NodeBin*>(nodeBin);
        bool res = updateNode_(&nodeBin->l, pNode, pNodeUpdate);
        if (res)
        {
            b->l = std::move(pNodeUpdate);
            return true;
        }

        res = updateNode_(&nodeBin->r, pNode, pNodeUpdate);
        if (res)
        {
            b->r = std::move(pNodeUpdate);
            return true;
        }
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

AST::AST(const AST& other)
{
    *this = other;
}

AST& AST::operator=(const AST& other)
{
    if (this == &other)
        return *this;

    auto pNodeRoot = other.cloneRoot();
    setRoot(std::move(pNodeRoot));
    return *this;
}

void AST::setRoot(std::unique_ptr<INode> root)
{
    m_pRoot = std::move(root);
}

bool AST::has_symbol(const std::string_view symbol) const noexcept
{
    return has_symbol_(getRoot(), symbol);
}

bool AST::updateNode(const INode* pNode, std::unique_ptr<INode>& pNodeUpdate)
{
    if (pNode == nullptr || pNodeUpdate == nullptr 
        return false;

    // special case if it is the root.
    if (pNode == getRoot())
    {
        setRoot(std::move(pNodeUpdate));
        return true;
    }

    // find node and its parent to replace it
    return updateNode_(&m_pRoot, pNode, pNodeUpdate);
}

std::unique_ptr<AST::INode> AST::cloneRoot() const
{
    return AST::clone(getRoot());
}

std::unique_ptr<AST::INode> AST::clone(const INode* pNode)
{
    if (pNode == nullptr)
        return nullptr;

    auto pNodeClone = clone_(pNode);
    if (pNodeClone == nullptr)
        return nullptr;

    return pNodeClone;
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
