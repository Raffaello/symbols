#include "Simplifier.hpp"
#include "mp_t.hpp"

bool Simplifier::reduce_(AST& src, const AST::INode* pCurrent)
{
    if (pCurrent->is_num())
        return true;
    else if (pCurrent->is_symbol())
        return true;
    else if (pCurrent->is_unary())
        return reduce_uny_(src, pCurrent);
    if (pCurrent->is_expr())
        return reduce_expr_(src, pCurrent);

    return false;
}

bool Simplifier::reduce_uny_(AST& src, const AST::INode* pCurrent)
{
    auto pNodeUny = dynamic_cast<const AST::NodeUnary*>(pCurrent);
    if (pNodeUny == nullptr)
        return false;

    if (pNodeUny->n->is_num())
    {
        ast_num_t v;
        if (!AST::LeafNum::getValue(pNodeUny->n.get(), v))
            return false;

        if (pNodeUny->negate)
            v = -v;

        auto pNodeUpd = AST::LeafNum::make(v);
        return src.updateNode(pCurrent, pNodeUpd);
    }
    else
        return reduce_(src, pNodeUny->n.get());
}

bool Simplifier::reduce_expr_(AST& src, const AST::INode* pCurrent)
{
    auto pNodeBin = dynamic_cast<const AST::NodeBin*>(pCurrent);
    if (pNodeBin == nullptr)
        return false;

    if (!reduce_(src, pNodeBin->l.get()))
        return false;

    if (!reduce_(src, pNodeBin->r.get()))
        return false;

    // case NUM NUM
    if (pNodeBin->l->is_num() && pNodeBin->r->is_num())
    {
        std::unique_ptr<AST::INode> pNodeUpd = nullptr;
        ast_num_t                   a, b;
        if (!AST::LeafNum::getValue(pNodeBin->l.get(), a))
            return false;
        if (!AST::LeafNum::getValue(pNodeBin->r.get(), b))
            return false;

        switch (pNodeBin->op)
        {
            using enum AST::eOperators;

        default:
            [[fallthrough]];
        case NONE:
            return false;
        case ADD:
            pNodeUpd = AST::LeafNum::make(a + b);
            break;
        case SUB:
            pNodeUpd = AST::LeafNum::make(a - b);
            break;
        case MUL:
            pNodeUpd = AST::LeafNum::make(a * b);
            break;
        case DIV:
            if (b != 0)
                pNodeUpd = AST::LeafNum::make(a / b);
            else
                return true;
            break;
        case POW:
            if (b == 0)
                pNodeUpd = AST::LeafNum::make(1);
            else if (b == 1)
                pNodeUpd = AST::LeafNum::make(a);
            else
            {
                mp_t z = a;
                z      = z ^ b;
                if (!z.isRational() || z.isWeird())
                    return true;

                ast_num_t q = z;
                pNodeUpd    = AST::LeafNum::make(q);
            }
            break;
        case EQUAL:
            return true;
            break;
        }

        return src.updateNode(pCurrent, pNodeUpd);
    }

    // if l or r is a identity num for the operation
    if (!reduce_expr_identity_and_special_cases_(src, pNodeBin))
        return false;

    // nothing else to simplify at the moment (it won't be used)
    return true;
}

bool Simplifier::reduce_expr_identity_and_special_cases_(AST& src, const AST::NodeBin* pNodeBin)
{
    if (pNodeBin == nullptr)
        return false;

    ast_num_t                   v;
    bool                        lr_swap;
    std::unique_ptr<AST::INode> pNodeUpd = nullptr;
    const AST::INode*           l        = nullptr;
    const AST::INode*           r        = nullptr;

    // considering the num to be on the right first,
    // simpler for dealing to DIV and POW as not commutative
    if (pNodeBin->r->is_num())
    {
        l       = pNodeBin->l.get();
        r       = pNodeBin->r.get();
        lr_swap = false;
    }
    if (pNodeBin->l->is_num())
    {
        l       = pNodeBin->r.get();
        r       = pNodeBin->l.get();
        lr_swap = true;
    }

    if (!AST::LeafNum::getValue(r, v))
        return false;

    switch (pNodeBin->op)
    {
        using enum AST::eOperators;

    case ADD:
        [[fallthrough]];
    case SUB:
        if (v == 0)
            // It could move it instead of cloning, but...
            pNodeUpd = AST::clone(l);
        else
            return true;
        break;
    case MUL:
        if (v == 0)
            pNodeUpd = AST::LeafNum::make(0);
        else if (v == 1)
            pNodeUpd = AST::clone(l);
        else
            return true;
        break;
    case DIV:
        if (v == 1 && !lr_swap)
            pNodeUpd = AST::clone(l);
        else
            return true;
        break;
    case POW:
        if (lr_swap)
        {
            // 1^x
            if (!r->is_num())
                return true;

            ast_num_t v2;
            if (!AST::LeafNum::getValue(r, v2))
                return false;

            if (v2 == 1)
                pNodeUpd = AST::LeafNum::make(1);
            else
                return true;
        }
        else if (v == 0)
            pNodeUpd = AST::LeafNum::make(1);
        else if (v == 1)
            pNodeUpd = AST::clone(l);
        else
            return true;
        break;

    case NONE:
        [[fallthrough]];
    default:
        return false;
    }

    return src.updateNode(pNodeBin, pNodeUpd);
}

bool Simplifier::reduce(AST& src)
{
    AST ast = src;
    if (reduce_(ast, ast.getRoot()))
    {
        src = ast;
        return true;
    }

    return false;
}
