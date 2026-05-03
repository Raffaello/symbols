#include "ParserLL1.hpp"

#include <iostream>
#include <format>
#include <stdexcept>
#include <cassert>

bool ParserLL1::advance_()
{
    if (!m_lexer.next())
    {
        if (m_token.type == eTOKENS::ERROR)
            std::cerr << std::format("Error: {}\n", m_token.value);
        else
        {
            m_end         = true;
            m_token.value = "";
        }

        return false;
    }

    m_token = m_lexer.lastToken();
    return true;
}

std::unique_ptr<INode> ParserLL1::stmt_()
{
    auto expr = expr_();
    if (auto sym = dynamic_cast<const LeafSymbol*>(expr.get()))
    {
        // then could be an assignment
        if (m_token.type == eTOKENS::EQUAL)
        {
            Token t = m_token;
            if (!advance_())
            {
                std::cerr << std::format("Expected an expression after {}\n", t.value);
                return nullptr;
            }

            auto expr2 = expr_();
            if (expr2 == nullptr)
                return nullptr;

            auto n   = std::make_unique<NodeBin>();
            n->token = t;
            n->l     = std::move(expr);
            n->r     = std::move(expr2);

            return n;
        }
    }

    if (!m_end)
    {
        std::cerr << std::format("ERROR: invalid statement\n");
        return nullptr;
    }

    return expr;
}

bool ParserLL1::expect_(const eTOKENS type)
{
    return m_token.type == type;
}

std::unique_ptr<INode> ParserLL1::expr_()
{
    auto left = term_();
    if (left == nullptr)
        return nullptr;

    return exprPrime_(std::move(left));
}

std::unique_ptr<INode> ParserLL1::exprPrime_(std::unique_ptr<INode> left)
{
    if (m_token.type == eTOKENS::SUM_OP /*|| m_token.type == eTOKENS::EQUAL*/)
    {
        auto node   = std::make_unique<NodeBin>();
        node->token = m_token;

        if (!advance_())
        {
            std::cerr << std::format("ERROR: Expected a factor after operator {}\n", node->token.value);
            return nullptr;
        }

        auto right = term_();
        if (right == nullptr)
            return nullptr;

        node->l = std::move(left);
        node->r = std::move(right);

        return exprPrime_(std::move(node));
    }
    else
    {
        return left;
    }
}

std::unique_ptr<INode> ParserLL1::term_()
{
    auto left = factor_();
    if (left == nullptr)
        return nullptr;

    return termPrime_(std::move(left));
}

std::unique_ptr<INode> ParserLL1::termPrime_(std::unique_ptr<INode> left)
{
    if (m_token.type == eTOKENS::MUL_OP)
    {
        auto node   = std::make_unique<NodeBin>();
        node->token = m_token;

        if (!advance_())
        {
            std::cerr << std::format("ERROR: Expected a factor after operator {}\n", node->token.value);
            return nullptr;
        }

        auto right = factor_();
        if (right == nullptr)
            return nullptr;

        node->l = std::move(left);
        node->r = std::move(right);

        return termPrime_(std::move(node));
    }
    else
        return left;
}

std::unique_ptr<INode> ParserLL1::factor_()
{
    auto u = unary_();
    auto p = pred_();
    if (p == nullptr)
        return nullptr;

    if (u == nullptr)
        return p;

    dynamic_cast<NodeUnary*>(u.get())->n = std::move(p);
    return u;
}

std::unique_ptr<INode> ParserLL1::unary_()
{
    if (m_token.type == eTOKENS::SUM_OP)
    {
        Token t = m_token;
        if (!advance_())
        {
            std::cerr << std::format("ERROR: after: '{}'\n", t.value);
            return nullptr;
        }

        auto n   = std::make_unique<NodeUnary>();
        n->token = t;
        n->n     = nullptr;
        return n;
    }
    else
        return nullptr;
}

std::unique_ptr<INode> ParserLL1::pred_()
{
    switch (m_token.type)
    {
        using enum eTOKENS;

    case LEFT_PARENTHESES:
    {
        if (!advance_())
        {
            std::cerr << std::format("ERROR: expected an expression after: '('\n");
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
        auto node   = std::make_unique<LeafSymbol>();
        node->value = m_token.value;
        advance_();
        return node;
    }
    break;
    case NUM:
    {
        size_t       pos = 0;
        const double num = std::stod(m_token.value, &pos);
        if (pos != m_token.value.size())
        {
            std::cerr << std::format("ERROR: unable to parse number: {}, parsed into: {}\n", m_token.value, num);
            return nullptr;
        }

        auto node   = std::make_unique<LeafNum>();
        node->value = num;

        advance_();
        return node;
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
}

bool ParserLL1::parse()
{
    m_end = false;
    if (!advance_())
        return false;

    // auto root = expr_();
    auto root = stmt_();
    if (root == nullptr)
        return false;

    if (m_token.type == eTOKENS::ERROR)
        std::runtime_error("debug");

    if (!m_end)
        std::runtime_error("debug2");

    m_ast.setRoot(root);
    return true;
}
