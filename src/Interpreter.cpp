#include "Interpreter.hpp"

#include <cassert>
#include <iostream>
#include <format>
#include <cmath>

std::optional<bool> Interpreter::evalNum_(const INode* node)
{
    if (auto num = dynamic_cast<const LeafNum*>(node))
    {
        m_lastValue = num->value;
        m_lastExpr  = std::format("{}", num->value);
        return true;
    }

    return std::nullopt;
}

std::optional<bool> Interpreter::evalSym_(const INode* node)
{
    if (auto sym = dynamic_cast<const LeafSymbol*>(node))
    {
        if (m_symbolTable.contains(sym->value))
        {
            m_lastValue = m_symbolTable[sym->value];
            m_lastExpr  = std::format("{} = {}", sym->value, m_lastValue);
            return true;
        }
        else
        {
            std::cerr << std::format("ERROR: Symbol {} not found!\n", sym->value);
            return false_();
        }
    }

    return std::nullopt;
}

std::optional<bool> Interpreter::evalUni_(const INode* node)
{
    if (auto uni = dynamic_cast<const NodeUnary*>(node))
    {
        assert(uni->token.type == eTOKENS::SUM_OP);

        if (!eval_(uni->n.get()))
            return false_();

        if (uni->token.value == "-")
        {
            m_lastValue = -m_lastValue;
            if (m_lastValue < 0.0)
                m_lastExpr = std::format("-({})", m_lastExpr);
            else
                m_lastExpr = std::format("{}", m_lastValue);
        }

        return true;
    }

    return std::nullopt;
}

std::optional<bool> Interpreter::evalBin_(const INode* node)
{
    if (auto bin = dynamic_cast<const NodeBin*>(node))
    {
        if (!eval_(bin->r.get()))
            return false_();

        const double r = m_lastValue;

        // TODO: specific for the assignment:
        if (bin->token.type == eTOKENS::EQUAL)
        {
            if (auto sym = dynamic_cast<const LeafSymbol*>(bin->l.get()))
            {
                m_symbolTable[sym->value] = r;
                m_lastValue               = r;
                m_lastExpr                = std::format("{} = {}", sym->value, r);
                return true;
            }

            std::cerr << std::format("ERROR: Wrong assignment, LHS not a symbol, equation not supported yet\n");
            return false_();
        }

        if (!eval_(bin->l.get()))
            return false_();

        const double l = m_lastValue;
        switch (bin->token.type)
        {
            using enum eTOKENS;

        case SUM_OP:
            if (bin->token.value == "+")
                m_lastValue = l + r;
            else if (bin->token.value == "-")
                m_lastValue = l - r;
            break;
        case MUL_OP:
            if (bin->token.value == "*")
                m_lastValue = l * r;
            else if (bin->token.value == "/")
            {
                if (r == 0.0)
                    std::cout << std::format("WARN: division by zero detected\n");

                m_lastValue = l / r;
            }
            break;
        case POW_OP:
            m_lastValue = std::pow(l, r);
            break;
        default:
            std::cerr << std::format("ERROR: not supported operator '{}'\n", bin->token.value);
            return false_();
        }

        m_lastExpr = std::format("{} {} {} = {}", l, bin->token.value, r, m_lastValue);
        // std::cout << std::format("|> {}\n", m_lastExpr);
        return true;
    }

    return std::nullopt;
}

bool Interpreter::eval_(const INode* node)
{
    auto res = evalNum_(node);
    if (res.has_value())
        return *res;

    res = evalSym_(node);
    if (res.has_value())
        return *res;

    res = evalUni_(node);
    if (res.has_value())
        return *res;

    res = evalBin_(node);
    if (res.has_value())
        return *res;

    std::cerr << std::format("ERROR: don't know how to interpret the input.\n");
    return false_();
}

bool Interpreter::false_() noexcept
{
    m_lastValue = std::numeric_limits<double>::quiet_NaN();
    m_lastExpr  = "";
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////

bool Interpreter::eval(const AST& ast)
{
    auto n = ast.getRoot();
    if (n == nullptr)
    {
        std::cerr << "ERROR: AST is empty, nothing to evaluate.\n";

        return false_();
    }

    return eval_(n);
}

bool Interpreter::unsetSymbol(const std::string_view symbol) noexcept
{
    if (m_symbolTable.contains(symbol.data()))
    {
        m_symbolTable.erase(symbol.data());
        return true;
    }

    std::cerr << std::format("ERROR: Symbol {} not found!\n", symbol);
    return false_();
}
