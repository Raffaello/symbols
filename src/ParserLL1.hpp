#pragma once

#include "LexScanner.hpp"
#include "Token.hpp"
#include "AST.hpp"

#include <stack>

/**
 * Recursive Descent LL1 Parsers
 *
 * Grammar:
 * S    ::= S' END
 * S'   ::= E | E = E
 * E    ::= T E'
 * E'   ::= + T E' | - T E' | e
 * T    ::= F T'
 * T'   ::= * F T' | / F T' | e
 * F    ::= U POW
 * U    ::= + | - | e
 * POW  ::= P POW'
 * POW' ::= ^ F | e
 * P    ::= (E) | SYMBOL | NUM
 */
class ParserLL1
{
private:
    LexScanner& m_lexer;
    AST         m_ast;
    Token       m_token;

    bool advance_();
    bool expect_(const eTOKENS type);

    std::unique_ptr<AST::INode> stmt_();
    std::unique_ptr<AST::INode> stmtPrime_();
    std::unique_ptr<AST::INode> expr_();
    std::unique_ptr<AST::INode> exprPrime_(std::unique_ptr<AST::INode> left);
    std::unique_ptr<AST::INode> term_();
    std::unique_ptr<AST::INode> termPrime_(std::unique_ptr<AST::INode> left);
    std::unique_ptr<AST::INode> factor_();
    std::unique_ptr<AST::INode> unary_();
    std::unique_ptr<AST::INode> pow_();
    std::unique_ptr<AST::INode> powPrime_(std::unique_ptr<AST::INode> left);
    std::unique_ptr<AST::INode> pred_();

public:
    ParserLL1(LexScanner& lex_scanner);

    bool parse();

    inline AST& ast() noexcept;
};

inline AST& ParserLL1::ast() noexcept
{
    return m_ast;
}
