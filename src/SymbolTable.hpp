#pragma once

#include "AST.hpp"
#include "multi_precision.hpp"

#include <unordered_map>
#include <string>
#include <cmath>
#include <algorithm>

class SymbolTable
{
private:
    // TODO: for the Solver the Symbol table should store a simplified AST, INode, its own mini AST for each symbol,
    //       this will allow substitutions for e.g
    //
    //     a = b + c,
    //     b = d,
    //     c = 1,
    //     d = 2
    //     => a = 2 + 1
    //        b = 2
    //        c = 1
    //        d = 2
    //
    // or just keep a symbol defined as a = b + c, where b and c are other 2 symbols (undefined)
    // so this can allow substituion in an equation as well.
    std::unordered_map<std::string, mp_num_t> m_table;

public:
    inline const std::unordered_map<std::string, mp_num_t>& table() const noexcept;

    inline void      clear() noexcept;
    inline bool      contains(const std::string& name) const noexcept;
    inline mp_num_t& operator[](const std::string& name);
    inline size_t    erase(const std::string& name);

    size_t key_max_length() const noexcept;

    /**
     * @brief Set the Symbol object if @p pNode is a LeafSymbol node
     *
     * @param pNode
     * @param val
     * @return const char* @c nullptr if it didn't set, symbol value (name) otherwise
     */
    const char* setSymbol(const AST::INode* pNode, const mp_num_t& val);
    bool        getSymbol(const std::string& name, mp_num_t& val) const noexcept;
};

inline void SymbolTable::clear() noexcept
{
    m_table.clear();
}

inline const std::unordered_map<std::string, mp_num_t>& SymbolTable::table() const noexcept
{
    return m_table;
}

inline bool SymbolTable::contains(const std::string& name) const noexcept
{
    return m_table.contains(name);
}

inline mp_num_t& SymbolTable::operator[](const std::string& name)
{
    return m_table[name];
}

inline size_t SymbolTable::erase(const std::string& name)
{
    return m_table.erase(name);
}

inline bool SymbolTable::getSymbol(const std::string& name, mp_num_t& val) const noexcept
{
    if (auto it = m_table.find(name); it != m_table.end())
    {
        val = it->second;
        return true;
    }

    return false;
}
