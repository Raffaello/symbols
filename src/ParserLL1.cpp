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
            m_end = true;
        // throw std::runtime_error("LexicalScanner Error!");    // it shouldn't never reach here, unless there is a bug.

        return false;
    }

    m_token = m_lexer.lastToken();
    return true;
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

    return left;

    // TODO RIGHT
}

void ParserLL1::exprPrime_()
{
}

std::unique_ptr<INode> ParserLL1::term_()
{
    auto left = factor_();
    if (left == nullptr)
        return nullptr;

    return left;

    // TODO RIGHT (termPrime_)

    // auto nodeBin = std::make_unique<NodeBin>();
    // nodeBin->l   = std::move(left);

    // return nodeBin;
}

void ParserLL1::termPrime_()
{
}

std::unique_ptr<INode> ParserLL1::factor_()
{

    switch (m_token.type)
    {
        using enum eTOKENS;

    case LEFT_PARENTHESES:
    {
        // auto node   = std::make_unique<LeafNum>();
        // node->token = m_token;
        if (!advance_())
        {
            std::cerr << std::format("ERROR: expected an expression after: '('\n");
            return nullptr;
        }

        auto node = expr_();
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

    auto root = expr_();
    if (root == nullptr)
        return false;

    if (m_token.type == eTOKENS::ERROR)
        std::runtime_error("debug");

    if (!m_end)
        std::runtime_error("debug2");

    m_ast.setRoot(root);
    return true;
}
