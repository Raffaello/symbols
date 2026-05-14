#include "ParserLL1.hpp"
#include "multi_precision.hpp"

#include <iostream>
#include <format>
#include <stdexcept>
#include <cassert>

bool ParserLL1::advance_()
{
    if (!m_lexer.next())
        return false;

    m_token = m_lexer.lastToken();
    return true;
}

bool ParserLL1::expect_(const eTOKENS type)
{
    return m_token.type == type;
}

std::unique_ptr<AST::INode> ParserLL1::stmt_()
{
    auto s = stmtPrime_();
    if (s == nullptr)
        return nullptr;

    if (m_token.type != eTOKENS::END)
    {
        std::cerr << std::format("ERROR: Expected statement end, instead: {}\n", m_token.value);
        return nullptr;
    }

    return s;
}

std::unique_ptr<AST::INode> ParserLL1::stmtPrime_()
{
    auto l = expr_();
    if (l == nullptr)
        return nullptr;

    if (m_token.type == eTOKENS::EQUAL)
    {
        Token t = m_token;
        if (!advance_())
        {
            std::cerr << std::format("ERROR: after {}\n", t.value);
            return nullptr;
        }

        auto r = expr_();
        if (r == nullptr)
            return nullptr;

        return AST::NodeBin::make(AST::eOperators::EQUAL, std::move(l), std::move(r));
    }

    return l;
}

std::unique_ptr<AST::INode> ParserLL1::expr_()
{
    auto left = term_();
    if (left == nullptr)
        return nullptr;

    return exprPrime_(std::move(left));
}

std::unique_ptr<AST::INode> ParserLL1::exprPrime_(std::unique_ptr<AST::INode> left)
{
    if (m_token.type == eTOKENS::SUM_OP)
    {
        const Token t = m_token;
        if (!advance_())
        {
            std::cerr << std::format("ERROR: after operator {}\n", t.value);
            return nullptr;
        }

        auto right = term_();
        if (right == nullptr)
            return nullptr;

        AST::eOperators op = AST::eOperators::NONE;
        if (t.value == TOKEN_VALUE_PLUS)
            op = AST::eOperators::ADD;
        else if (t.value == TOKEN_VALUE_MINUS)
            op = AST::eOperators::SUB;
        else
        {
            std::cerr << std::format("ERROR: invalid sum operator {}", t.value);
            return nullptr;
        }

        return exprPrime_(AST::NodeBin::make(op, std::move(left), std::move(right)));
    }
    else
        return left;
}

std::unique_ptr<AST::INode> ParserLL1::term_()
{
    auto left = factor_();
    if (left == nullptr)
        return nullptr;

    return termPrime_(std::move(left));
}

std::unique_ptr<AST::INode> ParserLL1::termPrime_(std::unique_ptr<AST::INode> left)
{
    if (m_token.type == eTOKENS::MUL_OP)
    {
        const Token t = m_token;
        if (!advance_())
        {
            std::cerr << std::format("ERROR: after operator {}\n", t.value);
            return nullptr;
        }

        auto right = factor_();
        if (right == nullptr)
            return nullptr;

        AST::eOperators op = AST::eOperators::NONE;
        if (t.value == TOKEN_VALUE_MUL)
            op = AST::eOperators::MUL;
        else if (t.value == TOKEN_VALUE_DIV)
            op = AST::eOperators::DIV;
        else
        {
            std::cerr << std::format("ERROR: invalid mul operator {}", t.value);
            return nullptr;
        }

        return termPrime_(AST::NodeBin::make(op, std::move(left), std::move(right)));
    }
    else
        return left;
}

std::unique_ptr<AST::INode> ParserLL1::factor_()
{
    auto u = unary_();
    auto p = pow_();
    if (p == nullptr)
        return nullptr;

    if (u == nullptr)
        return p;

    dynamic_cast<AST::NodeUnary*>(u.get())->n = std::move(p);
    return u;
}

std::unique_ptr<AST::INode> ParserLL1::unary_()
{
    if (m_token.type == eTOKENS::SUM_OP)
    {
        Token t = m_token;
        if (!advance_())
        {
            std::cerr << std::format("ERROR: after: '{}'\n", t.value);
            return nullptr;
        }

        return AST::NodeUnary::make(t.value == TOKEN_VALUE_MINUS);    // n->n     = nullptr;
    }
    else
        return nullptr;
}

std::unique_ptr<AST::INode> ParserLL1::pow_()
{
    auto left = pred_();
    if (left == nullptr)
        return nullptr;

    return powPrime_(std::move(left));
}

std::unique_ptr<AST::INode> ParserLL1::powPrime_(std::unique_ptr<AST::INode> left)
{
    if (m_token.type == eTOKENS::POW_OP)
    {
        const Token t = m_token;
        if (!advance_())
        {
            std::cerr << std::format("ERROR: after {}\n", t.value);
            return nullptr;
        }

        auto r = factor_();
        if (r == nullptr)
            return nullptr;

        return AST::NodeBin::make(AST::eOperators::POW, std::move(left), std::move(r));
    }

    return left;
}

std::unique_ptr<AST::INode> ParserLL1::pred_()
{
    switch (m_token.type)
    {
        using enum eTOKENS;

    case LEFT_PARENTHESES:
    {
        if (!advance_())
        {
            std::cerr << std::format("ERROR: after: '{}'\n", m_token.value);
            return nullptr;
        }

        auto node = expr_();
        if (node == nullptr)
            return nullptr;

        if (!expect_(eTOKENS::RIGHT_PARENTHESES))
        {
            std::cerr << std::format("ERROR: expected ')' instead {}\n", m_token.value);
            return nullptr;
        }

        advance_();
        return node;
    }
    break;
    case SYMBOL:
    {
        auto node = AST::LeafSymbol::make(m_token.value);
        advance_();
        return node;
    }
    break;
    case NUM:
    {
        size_t    pos = 0;
        ast_num_t num(m_token.value);

        advance_();
        return AST::LeafNum::make(num);
    }
    break;
    default:
        std::cerr << std::format("ERROR: Expected '(', a symbol or a number. Instead: {}\n", m_token.value);
        return nullptr;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ParserLL1::ParserLL1(LexScanner& lex_scanner) : m_lexer(lex_scanner)
{
    mp::mpfr_float::default_precision(MPFR_PRECISION);
}

bool ParserLL1::parse()
{
    if (!advance_())
        return false;

    auto root = stmt_();
    if (root == nullptr)
        return false;

    if (m_token.type == eTOKENS::ERROR)
        throw std::runtime_error("debug");    // it should never store a token error, as the lexer is reporting the error, here just return nullptr/false instead and it has store the last scanned token

    m_ast.setRoot(root);
    return true;
}
