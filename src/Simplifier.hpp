#pragma once

#include "AST.hpp"

#include <memory>

class Simplifier
{
private:
    static bool reduce_(AST& src, AST::INode* pCurrent);
    static bool reduce_uny_(AST& src, AST::INode* pCurrent);
    static bool reduce_expr_(AST& src, AST::INode* pCurrent);

    static bool reduce_expr_expr_num_(AST& src, AST::INode** pCurrent);
    // static bool reduce_expr_expr_num_same_op_(AST& src, const,)

    static bool reduce_expr_same_sym_(AST& src, AST::INode** pCurrent);
    static bool reduce_expr_num_num_(AST& src, AST::INode** pCurrent);
    static bool reduce_expr_identity_and_special_cases_(AST& src, AST::INode** pCurrent);

public:
    static bool reduce(AST& src);
};
