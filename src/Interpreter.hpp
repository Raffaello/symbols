#pragma once

#include "AST.hpp"

#include <unordered_map>
#include <string>
#include <limits>
#include <optional>

class Interpreter
{
private:
    std::unordered_map<std::string, double> m_symbolTable;

    // TODO: the symbol table could contain the lastValue too as a special symbol / keyword for e.g $? or $1
    double m_lastValue = std::numeric_limits<double>::quiet_NaN();

    std::optional<bool> evalNum_(const INode* node);
    std::optional<bool> evalSym_(const INode* node);
    std::optional<bool> evalUni_(const INode* node);
    std::optional<bool> evalBin_(const INode* node);

    bool eval_(const INode* node);

public:
    Interpreter() = default;

    bool eval(const AST& ast);

    inline double lastValue() const noexcept;

    inline const std::unordered_map<std::string, double>& symbolTable() const noexcept;
};

inline double Interpreter::lastValue() const noexcept
{
    return m_lastValue;
}

inline const std::unordered_map<std::string, double>& Interpreter::symbolTable() const noexcept
{
    return m_symbolTable;
}
