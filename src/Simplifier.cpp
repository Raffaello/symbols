#include "Simplifier.hpp"
#include "mp_t.hpp"

bool Simplifier::reduce_(AST& src, AST::INode* pCurrent)
{
    if (pCurrent == nullptr)
        return false;

    if (pCurrent->is_num())
        return true;
    else if (pCurrent->is_symbol())
        return true;
    else if (pCurrent->is_unary())
        return reduce_uny_(src, pCurrent);
    else if (pCurrent->is_expr())
        return reduce_expr_(src, pCurrent);

    return false;
}

bool Simplifier::reduce_uny_(AST& src, AST::INode* pCurrent)
{
    auto pNodeUny = dynamic_cast<AST::NodeUnary*>(pCurrent);
    if (pNodeUny == nullptr)
        return false;

    if (!reduce_(src, pNodeUny->n.get()))
        return false;

    // if it is a + just drop it.
    if (!pNodeUny->negate)
    {
        auto pCur = pNodeUny->n.get();
        if (!src.updateNode(pCurrent, pNodeUny->n))
            return false;

        return reduce_(src, pCur);
    }

    // case uny->num
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

    // case double unary (negation)
    else if (pNodeUny->n->is_unary())
    {
        auto                        pNodeUnyChild = dynamic_cast<AST::NodeUnary*>(pNodeUny->n.get());
        std::unique_ptr<AST::INode> pNodeUpd      = AST::NodeUnary::make(pNodeUny->negate ^ pNodeUnyChild->negate, AST::clone(pNodeUnyChild->n.get()));
        auto                        pCur          = pNodeUpd.get();
        if (!src.updateNode(pCurrent, pNodeUpd))
            return false;

        return reduce_(src, pCur);
    }
    // case uny->expr
    else if (pNodeUny->n->is_expr())
    {
        auto pNodeBin = dynamic_cast<AST::NodeBin*>(pNodeUny->n.get());
        if (pNodeBin == nullptr)
            return false;

        const AST::INode* l = nullptr;
        const AST::INode* r = nullptr;
        bool              lr_swap;

        if (pNodeBin->l->is_num() && pNodeBin->r->is_symbol())
        {
            // 1 + x
            l       = pNodeBin->l.get();
            r       = pNodeBin->r.get();
            lr_swap = false;
        }
        else if (pNodeBin->l->is_symbol() && pNodeBin->r->is_num())
        {

            // x + 1
            r       = pNodeBin->l.get();
            l       = pNodeBin->r.get();
            lr_swap = true;
        }
        else
            return true;

        assert(pNodeUny->negate);    // at this point only the minus should have survived
        assert(l->is_num());
        assert(r->is_symbol());

        // l is num, r is sym
        std::unique_ptr<AST::INode> pNodeUpd = nullptr;
        ast_num_t                   v;
        if (!AST::LeafNum::getValue(l, v))
            return false;

        switch (pNodeBin->op)
        {
            using enum AST::eOperators;

        default:
            [[fallthrough]];
        case NONE:
            return false;

        case ADD:
            // -(1 + x) = -l -r
            pNodeUpd = AST::NodeBin::make(SUB, AST::LeafNum::make(-v), AST::clone(r));
            break;
        case SUB:
            if (lr_swap)    // -(x-1) = 1-x
                pNodeUpd = AST::NodeBin::make(SUB, AST::clone(l), AST::clone(r));
            else            // -(1-x) = x-1
                pNodeUpd = AST::NodeBin::make(SUB, AST::clone(r), AST::clone(l));
            break;

        case POW:
            [[fallthrough]];
        case DIV:
            [[fallthrough]];
        case MUL:
            return true;
        }

        auto pCur = pNodeUpd.get();
        if (!src.updateNode(pCurrent, pNodeUpd))
            return false;

        return reduce_(src, pCur);
    }

    return true;
}

bool Simplifier::reduce_expr_(AST& src, AST::INode* pCurrent)
{
    auto pNodeBin = dynamic_cast<AST::NodeBin*>(pCurrent);
    if (pNodeBin == nullptr)
        return false;

    if (!reduce_(src, pNodeBin->l.get()))
        return false;

    if (!reduce_(src, pNodeBin->r.get()))
        return false;

    // case expr[expr num]:
    pCurrent = reduce_expr_expr_num_(src, pCurrent);
    if (!reduce_expr_helper_(src, pCurrent, &pNodeBin))
        return false;
    else if (pNodeBin == nullptr)
        return true;

    // this should be for some missing cases
    // this case could be only simplified if it is in the form of:
    // x*x^2 => x^3
    // {s1} [op1] {s1} [op2] {num} same symbols
    // case  num expr
    if (pNodeBin->l->is_num() && pNodeBin->r->is_expr())
    {
        // TODO
    }

    // case expr uny
    pCurrent = reduce_expr_uny_(src, pCurrent);
    if (!reduce_expr_helper_(src, pCurrent, &pNodeBin))
        return false;
    else if (pNodeBin == nullptr)
        return true;


    // case same symbol
    pCurrent = reduce_expr_same_sym_(src, pCurrent);
    if (!reduce_expr_helper_(src, pCurrent, &pNodeBin))
        return false;
    else if (pNodeBin == nullptr)
        return true;


    // case NUM NUM
    pCurrent = reduce_expr_num_num_(src, pCurrent);
    if (!reduce_expr_helper_(src, pCurrent, &pNodeBin))
        return false;
    else if (pNodeBin == nullptr)
        return true;


    // if l or r is an identity num for the operation
    pCurrent = reduce_expr_identity_and_special_cases_(src, pCurrent);
    if (!reduce_expr_helper_(src, pCurrent, &pNodeBin))
        return false;
    else if (pNodeBin == nullptr)
        return true;

    pCurrent = reduce_expr_expr_sym_(src, pCurrent);
    if (!reduce_expr_helper_(src, pCurrent, &pNodeBin))
        return false;
    else if (pNodeBin == nullptr)
        return true;

    // all other situation when is an expr containing l expr and r expr
    pCurrent = reduce_expr_expr_expr_(src, pCurrent);
    if (!reduce_expr_helper_(src, pCurrent, &pNodeBin))
        return false;
    else if (pNodeBin == nullptr)
        return true;


    // nothing else to simplify at the moment (it won't be used)
    return true;
}

AST::INode* Simplifier::reduce_expr_expr_num_(AST& src, AST::INode* pCurrent)
{
    auto pNodeBin = dynamic_cast<const AST::NodeBin*>(pCurrent);
    if (pNodeBin == nullptr)
        return pCurrent;

    // TODO: missing the symmetric part of it l is num and r is expr
    if (!pNodeBin->l->is_expr() || !pNodeBin->r->is_num())
        return pCurrent;    // skip rule

    // 3 factors might be simplified into 2 factor:
    // a*x*b => ab*x (2*x*2 = 4*x)
    // the simplest case if have the same operator, except pow or div as they are commutative.
    auto pNodeBin2 = dynamic_cast<AST::NodeBin*>(pNodeBin->l.get());
    if (pNodeBin2 == nullptr)
        return nullptr;

    if (pNodeBin2->op != pNodeBin->op)
        return pCurrent;    // in such a case stop here for now

    // at this point pNodeBin2 can't be reduce anymore otherwise it was already reduce
    // in the previous rules,
    // so here is most likely the case that is a num - sym or sym - num expression.
    const AST::INode*           l2 = nullptr;
    const AST::INode*           r2 = nullptr;
    bool                        lr2_swapped;
    std::unique_ptr<AST::INode> pNodeUpd = nullptr;
    std::unique_ptr<AST::INode> pNode_r  = nullptr;
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
        return pCurrent;    // skip rule

    // r2 num, l2 sym, r num
    // (l2 _ r2) _ r
    ast_num_t vr;
    ast_num_t vr2;
    if (!AST::LeafNum::getValue(pNodeBin->r.get(), vr))
        return nullptr;
    if (!AST::LeafNum::getValue(r2, vr2))
        return nullptr;

    switch (pNodeBin->op)
    {
        using enum AST::eOperators;

    default:
        [[fallthrough]];
    case NONE:
        return nullptr;

    case ADD:
        // x+n+m => x + (n+m)
        // n+x+m => x + (n+m)
        vr += vr2;
        break;
    case SUB:
        if (lr2_swapped)    // n-x-m => -x - (n-m)
        {
            vr      -= vr2;
            pNode_r  = AST::NodeUnary::make(true, AST::clone(pNodeBin2->r.get()));
        }
        else    // x-n-m => x - (n+m)
            vr += vr2;
        break;
    case MUL:
        // x*n*m => nm*x
        // n*x*m => nm*x
        vr *= vr2;
        break;
    case DIV:
        if (lr2_swapped)
            return pCurrent;    // n/x/m => n / xm : skip

        // x/n/m => x/mn
        vr *= vr2;
        break;
    case POW:
        if (lr2_swapped)
            return pCurrent;    // (n^x)^m : skip

        // (x^n)^m => x^nm
        vr *= vr2;
        break;
    }

    if (pNode_r != nullptr)
        pNodeUpd = AST::NodeBin::make(pNodeBin->op, std::move(pNode_r), AST::LeafNum::make(vr));
    else if (lr2_swapped)
        pNodeUpd = AST::NodeBin::make(pNodeBin->op, std::move(pNodeBin2->r), AST::LeafNum::make(vr));
    else
        pNodeUpd = AST::NodeBin::make(pNodeBin->op, std::move(pNodeBin2->l), AST::LeafNum::make(vr));

    auto pCur = pNodeUpd.get();
    if (!src.updateNode(pNodeBin, pNodeUpd))
        return nullptr;

    return pCur;
}

AST::INode* Simplifier::reduce_expr_expr_sym_(AST& src, AST::INode* pCurrent)
{
    auto pNodeBin = dynamic_cast<AST::NodeBin*>(pCurrent);
    if (pNodeBin == nullptr)
        return pCurrent;

    if (!((pNodeBin->l->is_expr() && pNodeBin->r->is_symbol()) ||
          (pNodeBin->r->is_expr() && pNodeBin->l->is_symbol())))
        return pCurrent;

    // possible simplification rules:
    // x + (x+1) = 2*x + 1 // can be skipped
    // x + (x/2) = skip
    // x * (x+|-1) = skip
    // x / (x +|- 1) = skip
    // x * (x*2) = skip
    // x +|- (x +|-|*) =  should be possible to do something
    // x ^ (...) = skip
    // .....
    //......
    // x - (x+1) = (0 + 1) => 1 from another rule there after
    // x + (x*2) = 3*x
    // x * (x/2) = 3/2 x (as number, not as expression)

    // x * (x^2) = x^3
    switch (pNodeBin->op)
    {
        using enum AST::eOperators;

    default:
        [[fallthrough]];
    case NONE:
        return nullptr;

    case ADD:
        // TODO
        return pCurrent;
    case SUB:
        // TODO
        return pCurrent;
    case MUL:
        return reduce_expr_expr_sym_mul_(src, pNodeBin);
    case DIV:
        // TODO
        return reduce_expr_expr_sym_div_(src, pNodeBin);
    case POW:
        return pCurrent;    // skip
    }

    return nullptr;    // unteachable: like something is missing in this function
}

AST::INode* Simplifier::reduce_expr_expr_sym_mul_(AST& src, AST::NodeBin* pNodeBin)
{
    if (pNodeBin == nullptr)
        return nullptr;

    if (pNodeBin->op != AST::eOperators::MUL)
        return pNodeBin;

    bool              swap_lr;
    const AST::INode* l = nullptr;
    AST::INode*       r = nullptr;
    if (pNodeBin->l->is_symbol() && pNodeBin->r->is_expr())
    {
        l       = pNodeBin->l.get();
        r       = pNodeBin->r.get();
        swap_lr = false;
    }
    else if (pNodeBin->l->is_expr() && pNodeBin->r->is_symbol())
    {
        l       = pNodeBin->r.get();
        r       = pNodeBin->l.get();
        swap_lr = true;
    }
    else
        return pNodeBin;    // skip it for now

    auto sym = AST::LeafSymbol::getValue(l);
    if (sym == nullptr)
        return nullptr;

    auto pNodeBinR = dynamic_cast<AST::NodeBin*>(r);
    if (pNodeBinR == nullptr)
        return nullptr;

    bool              swap_r_lr;
    const AST::INode* rl = nullptr;
    const AST::INode* rr = nullptr;

    // here can reduce only if there are 2 symbols and a num
    if (pNodeBinR->l->is_symbol(sym) && pNodeBinR->r->is_num())
    {
        rl        = pNodeBinR->l.get();
        rr        = pNodeBinR->r.get();
        swap_r_lr = false;
    }
    else if (pNodeBinR->r->is_symbol(sym) && pNodeBinR->l->is_num())
    {
        rr        = pNodeBinR->l.get();
        rl        = pNodeBinR->r.get();
        swap_r_lr = true;
    }
    else
        return pNodeBin;    // skip it for now


    // x * (x+1) = skip
    // x * (x-1) = skip
    // x * (x*2) = (x^2 * 2)
    // x * (x/2) = (x^2 / 2)
    // x * (x^2) = x^3

    std::unique_ptr<AST::INode> pNodeUpd = nullptr;
    // sym=l, sym=rl, num=rr
    switch (pNodeBinR->op)
    {
        using enum AST::eOperators;

    default:
        [[fallthrough]];
    case NONE:
        return nullptr;

    case ADD:
        [[fallthrough]];
    case SUB:
        return pNodeBin;    // skip
    case MUL:
        pNodeUpd = AST::NodeBin::make(
            pNodeBinR->op,
            AST::NodeBin::make(POW, AST::clone(l), AST::LeafNum::make(2)),
            AST::clone(rr));
        break;
    case DIV:
        if (swap_r_lr)    // x*(2/x) = 2
        {
            pNodeUpd = AST::clone(rr);
        }
        else    // x*(x/2) = x^2/2
        {
            pNodeUpd = AST::NodeBin::make(
                pNodeBinR->op,
                AST::NodeBin::make(POW, AST::clone(l), AST::LeafNum::make(2)),
                AST::clone(rr));
        }
        break;
    case POW:
    {
        if (swap_r_lr)
            return pNodeBin;    // x * 2^x => skip

        ast_num_t v;
        if (!AST::LeafNum::getValue(rr, v))
            return nullptr;

        pNodeUpd = AST::NodeBin::make(POW, AST::clone(l), AST::LeafNum::make(v + 1));
    }
    break;
    }

    auto pCur = pNodeUpd.get();
    if (!src.updateNode(pNodeBin, pNodeUpd))
        return nullptr;

    return pCur;
}

AST::INode* Simplifier::reduce_expr_expr_sym_div_(AST& src, AST::NodeBin* pNodeBin)
{
    if (pNodeBin == nullptr)
        return nullptr;

    if (pNodeBin->op != AST::eOperators::DIV)
        return pNodeBin;

    // TODO
    return pNodeBin;
}

AST::INode* Simplifier::reduce_expr_uny_(AST& src, AST::INode* pCurrent)
{
    auto pNodeBin = dynamic_cast<AST::NodeBin*>(pCurrent);
    if (pNodeBin == nullptr)
        return pCurrent;

    bool              lr_swap;
    const AST::INode* l = nullptr;
    AST::INode*       r = nullptr;

    if (pNodeBin->r->is_unary())
    {
        lr_swap = false;
        l       = pNodeBin->l.get();
        r       = pNodeBin->r.get();
    }
    else if (pNodeBin->l->is_unary())
    {
        lr_swap = true;
        r       = pNodeBin->l.get();
        l       = pNodeBin->r.get();
    }
    else
        return pCurrent;    // skip rule


    auto pNodeUny = dynamic_cast<AST::NodeUnary*>(r);
    if (pNodeUny == nullptr || l == nullptr)
        return nullptr;

    // (a _ b) _ (-+c)
    // (a + b) + -c => a+b-c
    // (a + b) + +c => a+b+c
    // (a + b) - -c => a+b-+c
    // (a + b) - +c => a+b-c
    // (a _ b) - -( {expr} )

    // (a _ b) _ u(c) =>
    // (a _ b) + u(c) =>  (a _ b) u c
    // (a _ b) - u(c) =>  (a _ b) -u c
    // (a _ b) * u(c) => u[(a _ b) * c]
    // (a _ b) / u(c) => u[(a _ b) / c]
    // (a _ b) ^ u(c) => 1/((a _ b)^c) skip

    // Doing only the operator ADD and SUB for now
    // MUL and DIV could change the left expression for it but need a first, doesn't look a simplification at the moment
    std::unique_ptr<AST::INode> pNodeBinUpd = nullptr;
    switch (pNodeBin->op)
    {
        using enum AST::eOperators;

    default:
        [[fallthrough]];
    case NONE:
        return nullptr;

    case POW:
        return pCurrent;

    case DIV:
        [[fallthrough]];
    case MUL:
        pNodeBinUpd = AST::NodeUnary::make(
            pNodeUny->negate,
            AST::NodeBin::make(
                pNodeBin->op,
                AST::clone(l),
                AST::clone(pNodeUny->n.get())));
        break;

    case ADD:
        // a + +|n b
        pNodeBinUpd = AST::NodeBin::make(pNodeUny->negate ? SUB : ADD, AST::clone(l), std::move(pNodeUny->n));
        break;
    case SUB:
        // +|-b - a => +b - a, -b - a
        if (lr_swap)
            return pCurrent;    // skip;
        else                    // a - +|-b
            pNodeBinUpd = AST::NodeBin::make(pNodeUny->negate ? ADD : SUB, AST::clone(l), std::move(pNodeUny->n));
        break;
    }

    auto pCur = pNodeBinUpd.get();
    if (!src.updateNode(pNodeBin, pNodeBinUpd))
        return nullptr;

    return pCur;
}

AST::INode* Simplifier::reduce_expr_same_sym_(AST& src, AST::INode* pCurrent)
{
    auto pNodeBin = dynamic_cast<AST::NodeBin*>(pCurrent);
    if (pNodeBin == nullptr)
        return pCurrent;

    if (!pNodeBin->l->is_symbol())
        return pCurrent;    // skip the rule, no error

    auto sym = AST::LeafSymbol::getValue(pNodeBin->l.get());
    if (sym == nullptr)
        return nullptr;

    if (!pNodeBin->r->is_symbol(sym))
        return pCurrent;    // skip the rule, no same symbols

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
        pNodeUpd = AST::NodeBin::make(MUL, std::move(pNodeBin->l), AST::LeafNum::make(2));
        break;
    case SUB:
        // x-x = 0
        pNodeUpd = AST::LeafNum::make(0);
        break;
    case MUL:
        // x*x = x^2
        pNodeUpd = AST::NodeBin::make(POW, std::move(pNodeBin->l), AST::LeafNum::make(2));
        break;
    case DIV:
        // x/x = 1, assuming x!=0
        pNodeUpd = AST::LeafNum::make(1);
        break;
    case POW:
        // x^x : skip
        return pCurrent;
    }

    auto pCur = pNodeUpd.get();
    if (!src.updateNode(pNodeBin, pNodeUpd))
        return nullptr;

    return pCur;
}

AST::INode* Simplifier::reduce_expr_num_num_(AST& src, AST::INode* pCurrent)
{
    auto pNodeBin = dynamic_cast<const AST::NodeBin*>(pCurrent);
    if (pNodeBin == nullptr)
        return pCurrent;

    if (!pNodeBin->l->is_num() || !pNodeBin->r->is_num())
        return pCurrent;    // skip the rule, no error

    std::unique_ptr<AST::INode> pNodeUpd = nullptr;
    ast_num_t                   a, b;
    if (!AST::LeafNum::getValue(pNodeBin->l.get(), a))
        return nullptr;

    if (!AST::LeafNum::getValue(pNodeBin->r.get(), b))
        return nullptr;

    switch (pNodeBin->op)
    {
        using enum AST::eOperators;

    default:
        [[fallthrough]];
    case NONE:
        return nullptr;

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
            return pCurrent;
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
                return pCurrent;

            ast_num_t q = z;
            pNodeUpd    = AST::LeafNum::make(q);
        }
        break;
    case EQUAL:
        return pCurrent;
        break;
    }

    auto pCur = pNodeUpd.get();
    if (!src.updateNode(pNodeBin, pNodeUpd))
        return nullptr;

    return pCur;
}

AST::INode* Simplifier::reduce_expr_identity_and_special_cases_(AST& src, AST::INode* pCurrent)
{
    auto pNodeBin = dynamic_cast<const AST::NodeBin*>(pCurrent);
    if (pNodeBin == nullptr)
        return pCurrent;

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
        return pCurrent;    // skip rule.

    if (!AST::LeafNum::getValue(r, v))
        return nullptr;

    switch (pNodeBin->op)
    {
        using enum AST::eOperators;

    case ADD:
        if (v == 0)
            // It could move it instead of cloning, but...
            pNodeUpd = AST::clone(l);
        else
            return pCurrent;
        break;
    case SUB:
        if (v == 0)
        {
            // e.g. 0-x => -x
            if (lr_swap)
                pNodeUpd = AST::NodeUnary::make(true, AST::clone(l));
            else
                pNodeUpd = AST::clone(l);
        }
        else
            return pCurrent;
        break;
    case MUL:
        if (v == 0)
            pNodeUpd = AST::LeafNum::make(0);
        else if (v == 1)
            pNodeUpd = AST::clone(l);
        else if (v == -1)
            pNodeUpd = AST::NodeUnary::make(true, AST::clone(l));
        else
            return pCurrent;
        break;
    case DIV:
        if (v == 1 && !lr_swap)
            pNodeUpd = AST::clone(l);
        else
            return pCurrent;
        break;
    case POW:
        if (lr_swap)
        {
            // 1^x
            if (v == 1)
                pNodeUpd = AST::LeafNum::make(1);
            else
                return pCurrent;
        }
        else if (v == 0)
            pNodeUpd = AST::LeafNum::make(1);
        else if (v == 1)
            pNodeUpd = AST::clone(l);
        else
            return pCurrent;
        break;

    case NONE:
        [[fallthrough]];
    default:
        return nullptr;
    }

    auto pCur = pNodeUpd.get();
    if (!src.updateNode(pNodeBin, pNodeUpd))
        return nullptr;

    return pCur;
}

AST::INode* Simplifier::reduce_expr_expr_expr_(AST& src, AST::INode* pCurrent)
{
    auto pNodeBin = dynamic_cast<const AST::NodeBin*>(pCurrent);
    if (pNodeBin == nullptr)
        return pCurrent;

    // at this point i am skiping all the rules that could be simplified from all the other rules
    // so in this case the 2 expression alone can't be reduced, e.g.:
    // (2+x) * (2-x)
    // (x^2)*(x^2)
    // (x*2)*(x+1)
    // etc...

    // TODO
    return pCurrent;
}

bool Simplifier::reduce_expr_helper_(AST& src, AST::INode* pCurrent, AST::NodeBin** pNodeBin)
{
    if (pCurrent == nullptr)
        return false;

    if (pNodeBin == nullptr)
        return true;    // nothing more to reduce in this case

    *pNodeBin = dynamic_cast<AST::NodeBin*>(pCurrent);
    if (*pNodeBin == nullptr)
        return reduce_(src, pCurrent);

    return true;
}

bool Simplifier::reduce(AST& src, bool reduce_equation)
{
    AST        ast        = src;
    const bool isEquation = reduce_equation && ast.isEquation();
    if (isEquation)
    {
        if (!ast.convertToExpression())
            return false;
    }

    if (reduce_(ast, ast.getRoot()))
    {
        if (isEquation)
        {
            if (!ast.convertToEquation())
                return false;
        }
        src = std::move(ast);
        return true;
    }

    return false;
}
