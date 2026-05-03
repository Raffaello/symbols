#pragma once

#include "AST.hpp"

#include <unordered_map>
#include <string>
#include <limits>

class Interpreter
{
private:
    std::unordered_map<std::string, double> m_symbolTable;

    double m_lastValue = std::numeric_limits<double>::quiet_NaN();

    bool eval_(const INode* node);

public:
    Interpreter() = default;

    bool eval(const AST& ast);

    inline double lastValue() const noexcept;
};

inline double Interpreter::lastValue() const noexcept
{
    return m_lastValue;
}