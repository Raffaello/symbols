#pragma once

#include "AST.hpp"

#include <memory>

class Simplifier
{
private:
    static bool reduce_(AST& src, const AST::INode* pCurrent);
    static bool reduce_uny_(AST& src, const AST::INode* pCurrent);
    static bool reduce_expr_(AST& src, const AST::INode* pCurrent);
    static bool reduce_expr_identity_and_special_cases_(AST& src, const AST::NodeBin* pNodeBin);

public:
    static bool reduce(AST& src);
};
