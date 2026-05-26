#pragma once

#include "AST.hpp"

#include <memory>

class Simplifier
{
private:
    static bool reduce_(AST& src, AST::INode* pCurrent);
    static bool reduce_uny_(AST& src, AST::INode* pCurrent);
    static bool reduce_expr_(AST& src, AST::INode* pCurrent);

    static AST::INode* reduce_expr_expr_num_(AST& src, AST::INode* pCurrent);
    static AST::INode* reduce_expr_expr_sym_(AST& src, AST::INode* pCurrent);
    static AST::INode* reduce_expr_expr_sym_mul_(AST& src, AST::NodeBin* pNodeBin);
    static AST::INode* reduce_expr_expr_sym_div_(AST& src, AST::NodeBin* pNodeBin);

    static AST::INode* reduce_expr_uny_(AST& src, AST::INode* pCurrent);
    static AST::INode* reduce_expr_same_sym_(AST& src, AST::INode* pCurrent);
    static AST::INode* reduce_expr_num_num_(AST& src, AST::INode* pCurrent);
    static AST::INode* reduce_expr_identity_and_special_cases_(AST& src, AST::INode* pCurrent);

    static AST::INode* reduce_expr_expr_expr_(AST& src, AST::INode* pCurrent);

    static bool reduce_expr_helper_(AST& src, AST::INode* pCurrent, AST::NodeBin** pNodeBin);

public:
    static bool reduce(AST& src);
};
