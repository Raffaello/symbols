#pragma once

#include <unordered_map>
#include <string>

class SymbolTable
{
private:
    std::unordered_map<std::string, double> m_table;

public:
    inline const std::unordered_map<std::string, double>& table() const noexcept;
    inline void                                           clear() noexcept;
    inline bool                                           contains(const std::string& name) const noexcept;

    inline double& operator[](const std::string& name);
    inline size_t  erase(const std::string& name);
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
