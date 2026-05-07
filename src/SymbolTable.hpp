#pragma once

#include "AST.hpp"

#include <unordered_map>
#include <string>
#include <cmath>
#include <algorithm>

class SymbolTable
{
private:
    std::unordered_map<std::string, double> m_table;

public:
    inline const std::unordered_map<std::string, double>& table() const noexcept;

    inline void    clear() noexcept;
    inline bool    contains(const std::string& name) const noexcept;
    inline double& operator[](const std::string& name);
    inline size_t  erase(const std::string& name);

    size_t key_max_length() const noexcept;
    /**
     * @brief Set the Symbol object if @p pNode is a LeafSymbol node
     *
     * @param pNode
     * @param val
     * @return const char* @c nullptr if it didn't set, symbol value (name) otherwise
     */
    const char* setSymbol(const AST::INode* pNode, const double val);
    bool        getSymbol(const std::string& name, double& val) const noexcept;
};

inline void SymbolTable::clear() noexcept
{
    m_table.clear();
}

inline const std::unordered_map<std::string, double>& SymbolTable::table() const noexcept
{
    return m_table;
}

inline bool SymbolTable::contains(const std::string& name) const noexcept
{
    return m_table.contains(name);
}

inline double& SymbolTable::operator[](const std::string& name)
{
    return m_table[name];
}

inline size_t SymbolTable::erase(const std::string& name)
{
    return m_table.erase(name);
}

inline bool SymbolTable::getSymbol(const std::string& name, double& val) const noexcept
{
    if (auto it = m_table.find(name); it != m_table.end())
    {
        val = it->second;
        return true;
    }

    return false;
}
