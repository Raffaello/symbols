#pragma once

#include "LexScanner.hpp"
#include "Token.hpp"
#include "AST.hpp"

// #include <list>
#include <stack>

/**
 * Recursive Descent LL1 Parsers
 *
 * Grammar:
 * E  ::= T E'
 * E' ::= + T E' | - T E' | e
 * T  ::= F T'
 * T' ::= * F T' | / F T' | e
 * F  ::= (E) | SYMBOL | NUM
 */
class ParserLL1
{
private:
    LexScanner& m_lexer;
    AST         m_ast;
    Token       m_token;
    bool        m_end = false;

    bool advance_();
    bool expect_(const eTOKENS type);

    std::unique_ptr<INode> expr_();
    void                   exprPrime_();
    std::unique_ptr<INode> term_();
    void                   termPrime_();
    std::unique_ptr<INode> factor_();

public:
    // bool parse(const std::list<Token>& tokens);
    ParserLL1(LexScanner& lex_scanner);
    bool parse();

    inline AST& ast() noexcept;
};

inline AST& ParserLL1::ast() noexcept
{
    return m_ast;
}
