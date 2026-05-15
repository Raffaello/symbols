#include "Interpreter.hpp"
#include "formatters.hpp"
#include "multi_precision.hpp"

#include <cassert>
#include <iostream>
#include <format>
#include <cmath>
#include <stdexcept>

Interpreter::Interpreter(const std::shared_ptr<SymbolTable>& pSymbolTable) : m_pSymbolTable(pSymbolTable)
{
    if (m_pSymbolTable == nullptr)
        throw std::invalid_argument("symbol table is null");
}

std::optional<bool> Interpreter::evalNum_(const AST::INode* node)
{
    ast_num_t v;
    if (AST::LeafNum::getValue(node, v))
    {
        m_lastValue = v;
        m_lastExpr  = std::format("{}", m_lastValue);
        return true;
    }

    return std::nullopt;
}

std::optional<bool> Interpreter::evalSym_(const AST::INode* node)
{
    const char* v = AST::LeafSymbol::getValue(node);
    if (v != nullptr)
    {
        int_num_t n;
        if (m_pSymbolTable->getSymbol(v, n))
        {
            m_lastValue = n;
            m_lastExpr  = std::format("{} = {}", v, m_lastValue);
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
            m_lastExpr  = std::format("{}", m_lastValue);
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

        const auto r = m_lastValue;

        // TODO: specific for the assignment:
        if (bin->op == AST::eOperators::EQUAL)
        {
            const char* sym = m_pSymbolTable->setSymbol(bin->l.get(), r);
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

        const auto l = m_lastValue;
        switch (bin->op)
        {
            using enum AST::eOperators;

        case ADD:
            m_lastValue = l + r;
            break;
        case SUB:
            m_lastValue = l - r;
            break;
        case MUL:
            m_lastValue = l * r;
            break;
        case DIV:
            if (mp_isZero(r))
            {
                std::cerr << std::format("ERROR: division by zero detected\n");
                return false;
            }

            m_lastValue = l / r;
            break;
        case POW:
            m_lastValue = mp_pow(l, r);
            break;
        default:
            std::cerr << std::format("ERROR: not supported operator '{}'\n", static_cast<int>(bin->op));
            return false_();
        }

        m_lastExpr = std::format("{} {} {} = {}", l, AST::operator_to_string(bin->op), r, m_lastValue);
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
    m_lastValue = NAN_VALUE;
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
    if (m_pSymbolTable->contains(symbol))
    {
        m_pSymbolTable->erase(symbol);
        return true;
    }

    std::cerr << std::format("ERROR: Symbol {} undefined!\n", symbol);
    return false_();
}
