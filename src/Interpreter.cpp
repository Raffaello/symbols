#include "Interpreter.hpp"

#include <cassert>
#include <iostream>
#include <format>
#include <cmath>

std::optional<bool> Interpreter::evalNum_(const AST::INode* node)
{
    if (AST::LeafNum::getValue(node, m_lastValue))
    {
        m_lastExpr = std::format("{}", m_lastValue);
        return true;
    }

    return std::nullopt;
}

std::optional<bool> Interpreter::evalSym_(const AST::INode* node)
{
    const char* v = AST::LeafSymbol::getValue(node);
    if (v != nullptr)
    {
        if (m_symbolTable.getSymbol(v, m_lastValue))
        {
            m_lastExpr = std::format("{} = {}", v, m_lastValue);
            return true;
        }
        else
        {
            std::cerr << std::format("ERROR: Symbol {} not found!\n", v);
            return false_();
        }
    }

    return std::nullopt;
}

std::optional<bool> Interpreter::evalUny_(const AST::INode* node)
{
    if (auto uni = dynamic_cast<const AST::NodeUnary*>(node))
    {
        if (!eval_(uni->n.get()))
            return false_();

        if (uni->negate)
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

std::optional<bool> Interpreter::evalBin_(const AST::INode* node)
{
    if (auto bin = dynamic_cast<const AST::NodeBin*>(node))
    {
        if (!eval_(bin->r.get()))
            return false_();

        const double r = m_lastValue;

        // TODO: specific for the assignment:
        if (bin->op == AST::eOperators::EQUAL)
        {
            const char* sym = m_symbolTable.setSymbol(bin->l.get(), r);
            if (sym != nullptr)
            {
                m_lastValue = r;
                m_lastExpr  = std::format("{} = {}", sym, r);
                return true;
            }

            std::cerr << std::format("ERROR: Wrong assignment, LHS not a symbol, equation not supported yet\n");
            return false_();
        }

        if (!eval_(bin->l.get()))
            return false_();

        const double l = m_lastValue;
        switch (bin->op)
        {
            using enum AST::eOperators;

        case SUM:
            m_lastValue = l + r;
            break;
        case DIF:
            m_lastValue = l - r;
            break;
        case MUL:
            m_lastValue = l * r;
            break;
        case DIV:
            if (r == 0.0)
                std::cout << std::format("WARN: division by zero detected\n");

            m_lastValue = l / r;
            break;
        case POW:
            m_lastValue = std::pow(l, r);
            break;
        default:
            std::cerr << std::format("ERROR: not supported operator '{}'\n", static_cast<int>(bin->op));
            return false_();
        }

        m_lastExpr = std::format("{} {} {} = {}", l, AST::operator_to_string(bin->op), r, m_lastValue);
        // std::cout << std::format("|> {}\n", m_lastExpr);
        return true;
    }

    return std::nullopt;
}

bool Interpreter::eval_(const AST::INode* node)
{
    auto res = evalNum_(node);
    if (res.has_value())
        return *res;

    res = evalSym_(node);
    if (res.has_value())
        return *res;

    res = evalUny_(node);
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

bool Interpreter::unsetSymbol(const std::string& symbol) noexcept
{
    if (m_symbolTable.contains(symbol))
    {
        m_symbolTable.erase(symbol);
        return true;
    }

    std::cerr << std::format("ERROR: Symbol {} not found!\n", symbol);
    return false_();
}
