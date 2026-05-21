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

    // case expr num:
    if (!reduce_expr_expr_num_(src, &pCurrent))
        return false;

    // this is the case like 2+x*4, or 2*x^2
    // this case could be only simplified if it is in the form of:
    // x+x*2 => x*3
    // {s1} [op1] {s1} [op2] {num} same symbols
    // case  num expr
    if (pNodeBin->l->is_num() && pNodeBin->r->is_expr())
    {
        // TODO
        int i = 0;
    }

    // case same symbol
    if (!reduce_expr_same_sym_(src, &pCurrent))
        return false;

    // case NUM NUM
    if (!reduce_expr_num_num_(src, &pCurrent))
        return false;

    // if l or r is an identity num for the operation
    if (!reduce_expr_identity_and_special_cases_(src, &pCurrent))
        return false;

    // nothing else to simplify at the moment (it won't be used)
    return true;
}

bool Simplifier::reduce_expr_expr_num_(AST& src, const AST::INode** pCurrent)
{
    if (pCurrent == nullptr)
        return false;

    auto pNodeBin = dynamic_cast<const AST::NodeBin*>(*pCurrent);
    if (pNodeBin == nullptr)
        return true;

    if (!pNodeBin->l->is_expr() || !pNodeBin->r->is_num())
        return true;    // skip rule

    // 3 factors might be simplified into 2 factor:
    // a*x*b => ab*x (2*x*2 = 4*x)
    // the simplest case if have the same operator, except pow or div as they are commutative.
    auto pNodeBin2 = dynamic_cast<const AST::NodeBin*>(pNodeBin->l.get());
    if (pNodeBin2 == nullptr)
        return false;

    if (pNodeBin2->op != pNodeBin->op)
        return true;    // in such a case stop here for now

    // at this point pNodeBin2 can't be reduce anymore otherwise it was already reduce
    // in the previous rules,
    // so here is most likely the case that is a num - sym or sym - num expression.
    const AST::INode*           l2 = nullptr;
    const AST::INode*           r2 = nullptr;
    bool                        lr2_swapped;
    std::unique_ptr<AST::INode> pNodeUpd = nullptr;
    if (pNodeBin2->r->is_num())
    {
        r2          = pNodeBin2->r.get();
        l2          = pNodeBin2->l.get();
        lr2_swapped = false;
    }
    else if (pNodeBin2->l->is_num())
    {
        r2          = pNodeBin2->l.get();
        l2          = pNodeBin2->r.get();
        lr2_swapped = true;
    }
    else
        return false;    // error it shouldn't never reach here

    // r2 num, l2 sym, r num
    // (l2 _ r2) _ r
    ast_num_t vr;
    ast_num_t vr2;
    if (!AST::LeafNum::getValue(pNodeBin->r.get(), vr))
        return false;
    if (!AST::LeafNum::getValue(r2, vr2))
        return false;

    switch (pNodeBin->op)
    {
        using enum AST::eOperators;

    default:
        [[fallthrough]];
    case NONE:
        return false;

    case ADD:
        // x+n+m => x + (n+m)
        // n+x+m => x + (n+m)
        [[fallthrough]];
    case SUB:
        // x-n-m => x - (n+m)
        // n-x-m => x - (n+m)
        vr += vr2;
        break;
    case MUL:
        // x*n*m => nm*x
        // n*x*m => nm*x
        vr *= vr2;
        break;
    case DIV:
        if (lr2_swapped)
            return true;    // n/x/m => n / xm : skip

        // x/n/m => x/mn
        vr *= vr2;
        break;
    case POW:
        if (lr2_swapped)
            return true;    // (n^x)^m : skip

        // (x^n)^m => x^nm
        vr *= vr2;
        break;
    }

    if (lr2_swapped)
        pNodeUpd = AST::NodeBin::make(pNodeBin->op, std::move(const_cast<AST::NodeBin*>(pNodeBin2)->r), std::move(AST::LeafNum::make(vr)));
    else
        pNodeUpd = AST::NodeBin::make(pNodeBin->op, std::move(const_cast<AST::NodeBin*>(pNodeBin2)->l), std::move(AST::LeafNum::make(vr)));

    *pCurrent = pNodeUpd.get();
    return src.updateNode(pNodeBin, pNodeUpd);
}

bool Simplifier::reduce_expr_same_sym_(AST& src, const AST::INode** pCurrent)
{
    if (pCurrent == nullptr)
        return false;

    auto pNodeBin = dynamic_cast<const AST::NodeBin*>(*pCurrent);
    if (pNodeBin == nullptr)
        return true;

    if (!pNodeBin->l->is_symbol())
        return true;    // skip the rule, no error

    auto sym = AST::LeafSymbol::getValue(pNodeBin->l.get());
    if (sym == nullptr)
        return false;

    if (!pNodeBin->r->is_symbol(sym))
        return true;    // skip the rule, no same symbols

    std::unique_ptr<AST::INode> pNodeUpd = nullptr;
    switch (pNodeBin->op)
    {
        using enum AST::eOperators;

    default:
        [[fallthrough]];
    case NONE:
        break;

    case ADD:
        // x+x = 2*x
        pNodeUpd = AST::NodeBin::make(MUL, std::move(const_cast<AST::NodeBin*>(pNodeBin)->l), AST::LeafNum::make(2));
        break;
    case SUB:
        // x-x = 0
        pNodeUpd = AST::LeafNum::make(0);
        break;
    case MUL:
        // x*x = x^2
        pNodeUpd = AST::NodeBin::make(POW, std::move(const_cast<AST::NodeBin*>(pNodeBin)->l), AST::LeafNum::make(2));
        break;
    case DIV:
        // x/x = 1, assuming x!=0
        pNodeUpd = AST::LeafNum::make(1);
        break;
    case POW:
        // x^x : skip
        return true;
    }

    *pCurrent = pNodeUpd.get();
    return src.updateNode(pNodeBin, pNodeUpd);
}

bool Simplifier::reduce_expr_num_num_(AST& src, const AST::INode** pCurrent)
{
    if (pCurrent == nullptr)
        return false;

    auto pNodeBin = dynamic_cast<const AST::NodeBin*>(*pCurrent);
    if (pNodeBin == nullptr)
        return true;

    if (!pNodeBin->l->is_num() || !pNodeBin->r->is_num())
        return true;    // skip the rule, no error

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

    *pCurrent = pNodeUpd.get();
    return src.updateNode(pNodeBin, pNodeUpd);
}

bool Simplifier::reduce_expr_identity_and_special_cases_(AST& src, const AST::INode** pCurrent)
{
    if (pCurrent == nullptr)
        return false;

    auto pNodeBin = dynamic_cast<const AST::NodeBin*>(*pCurrent);
    if (pNodeBin == nullptr)
        return true;

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
    else if (pNodeBin->l->is_num())
    {
        l       = pNodeBin->r.get();
        r       = pNodeBin->l.get();
        lr_swap = true;
    }
    else
        return true;    // skip rule.

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

    *pCurrent = pNodeUpd.get();
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
