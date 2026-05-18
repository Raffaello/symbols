#pragma once

#include "AST.hpp"
#include "SymbolTable.hpp"
#include "multi_precision.hpp"


#include <string>
#include <limits>
#include <optional>
#include <string_view>
#include <memory>
#include <variant>

class Interpreter
{
private:
    std::shared_ptr<SymbolTable> m_pSymbolTable = nullptr;

    // TODO: the symbol table could contain the lastValue too as a special symbol / keyword for e.g $? or $1
    mp_num_t    m_lastValue = NAN_VALUE;
    std::string m_lastExpr  = "";    // the result of the last resolved expression

    std::optional<bool> evalNum_(const AST::INode* node);
    std::optional<bool> evalSym_(const AST::INode* node);
    std::optional<bool> evalUny_(const AST::INode* node);
    std::optional<bool> evalBin_(const AST::INode* node);

    bool eval_(const AST::INode* node);

    bool false_() noexcept;

public:
    Interpreter(const std::shared_ptr<SymbolTable>& pSymbolTable);

    bool eval(const AST& ast);
    bool unsetSymbol(const std::string& symbol) noexcept;

    inline void               clearSymbols() noexcept;
    inline const mp_num_t&    lastValue() const noexcept;
    inline std::string_view   lastExpr() const noexcept;
    inline const SymbolTable& symbolTable() const noexcept;
};

inline const mp_num_t& Interpreter::lastValue() const noexcept
{
    return m_lastValue;
}

inline std::string_view Interpreter::lastExpr() const noexcept
{
    return m_lastExpr;
}

inline const SymbolTable& Interpreter::symbolTable() const noexcept
{
    return *m_pSymbolTable;
}

inline void Interpreter::clearSymbols() noexcept
{
    m_pSymbolTable->clear();
}
