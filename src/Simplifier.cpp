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

        AST::LeafNum::setValue(pNodeUny->n.get(), v);
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
            // TODO: div by zero check
            pNodeUpd = AST::LeafNum::make(a / b);
            break;
        case POW:
            if (b == 0)
                pNodeUpd = AST::LeafNum::make(1);
            else if (b == 1)
                pNodeUpd = AST::LeafNum::make(a);
            else
                return true;
            break;
        case EQUAL:
            return true;
            break;
        }

        return src.updateNode(pCurrent, pNodeUpd);
    }

    // TODO: nothing else to simplify at the moment
    return true;
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
