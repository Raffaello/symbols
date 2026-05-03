#include "Interpreter.hpp"

#include <cassert>
#include <iostream>
#include <format>

bool Interpreter::eval_(const INode* node)
{
    if (auto num = dynamic_cast<const LeafNum*>(node))
    {
        m_lastValue = num->value;
        return true;
    }

    if (auto sym = dynamic_cast<const LeafSymbol*>(node))
    {
        if (m_symbolTable.contains(sym->value))
        {
            m_lastValue = m_symbolTable[sym->value];
            return true;
        }
        else
        {
            std::cerr << std::format("ERROR: Symbol {} not found!\n", sym->value);
            return false;
        }
    }

    if (auto uni = dynamic_cast<const NodeUnary*>(node))
    {
        assert(uni->token.type == eTOKENS::SUM_OP);

        if (!eval_(uni->n.get()))
            return false;

        if (uni->token.value == "-")
            m_lastValue = -m_lastValue;

        return true;
    }

    if (auto bin = dynamic_cast<const NodeBin*>(node))
    {
        if (!eval_(bin->r.get()))
            return false;

        const double r = m_lastValue;

        // specific for the assignemnt:
        if (bin->token.type == eTOKENS::EQUAL)
        {
            if (auto sym = dynamic_cast<const LeafSymbol*>(bin->l.get()))
            {
                m_symbolTable[sym->value] = r;
                return true;
            }

            std::cerr << std::format("ERROR: Wrong assignment, LHR not a symbol");
            return false;
        }

        if (!eval_(bin->l.get()))
            return false;

        const double l = m_lastValue;
        m_lastValue    = r;

        switch (bin->token.type)
        {
            using enum eTOKENS;

        case SUM_OP:
            if (bin->token.value == "+")
            {
                m_lastValue = l + r;
                return true;
            }
            else if (bin->token.value == "-")
            {
                m_lastValue = l - r;
                return true;
            }
            break;
        case MUL_OP:
            if (bin->token.value == "*")
            {
                m_lastValue = l * r;
                return true;
            }
            else if (bin->token.value == "/")
            {
                m_lastValue = l / r;
                return true;
            }
            break;
        default:
            std::cerr << std::format("ERROR: not supported operator '{}'\n", bin->token.value);
            return false;
        }
    }

    std::cerr << std::format("ERROR: don't know how to interpret the input.\n");
    return false;
}

bool Interpreter::eval(const AST& ast)
{
    auto n = ast.getRoot();
    if (n == nullptr)
    {
        std::cerr << "ERROR: AST is empty, nothing to evaluate.\n";
        return false;
    }

    return eval_(n);
}
