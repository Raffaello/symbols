#pragma once

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

    inline size_t key_max_length() const noexcept;
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

inline size_t SymbolTable::key_max_length() const noexcept
{
    std::size_t key_max_length = 0;
    for (const auto& [k, v] : m_table)
        key_max_length = std::max<size_t>(key_max_length, k.size());

    return key_max_length;
}
