#include "SymbolTable.hpp"

size_t SymbolTable::key_max_length() const noexcept
{
    std::size_t key_max_length = 0;
    for (const auto& [k, v] : m_table)
        key_max_length = std::max<size_t>(key_max_length, k.size());

    return key_max_length;
}

const char* SymbolTable::setSymbol(const AST::INode* pNode, const int_num_t& val)
{
    const char* sym = AST::LeafSymbol::getValue(pNode);
    if (sym != nullptr)
    {
        m_table[sym] = val;
        return sym;
    }

    return nullptr;
}
