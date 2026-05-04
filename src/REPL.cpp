#include "REPL.hpp"

#include <iostream>
#include <format>
#include <algorithm>

void REPL::banner_() const noexcept
{
    std::cout << std::format("Symbols REPL v0.1.0\n");
    std::cout << std::format("Type \":{}\" for more information.\n", CMD_HELP);
}

void REPL::help_() const noexcept
{
    std::cout << std::format(":{}\t\t => show this screen.\n", CMD_HELP);
    std::cout << std::format(":{}\t\t => exit the program.\n", CMD_QUIT);
    std::cout << std::format(":{}\t => print the symbol table.\n", CMD_SYM_TABLE);
    std::cout << std::format(":{}\t\t => print the last computed value.\n", CMD_LAST_VALUE);
    std::cout << std::format(":{} [symbol]\t => unset a symbol requires a name after.\n", CMD_SYM_UNSET);
    std::cout << std::format(":{}\t => clear the symbol table.\n", CMD_SYM_CLEAR);
    std::cout << std::format("\n");
}

void REPL::printSymbolTable_() const noexcept
{
    // get longer symbol name
    const auto& symTable       = m_intr.symbolTable();
    std::size_t key_max_length = 0;
    for (const auto& [k, v] : symTable)
        key_max_length = std::max<size_t>(key_max_length, k.size());

    key_max_length = std::max<size_t>(key_max_length, 7);

    std::cout << std::format("{:<{}} | {}\n", "Symbol", key_max_length, "Value");
    std::cout << std::string(std::max<size_t>(key_max_length + 8, 14), '-') << "\n";
    for (const auto& [k, v] : symTable)
        std::cout << std::format("{:<{}} | {}\n", k, key_max_length, v);
}

void REPL::printLastValue_() const noexcept
{
    std::cout << std::format("| $? = {}\n", m_intr.lastValue());
}

void REPL::symbol_unset_(const std::string_view replCmd) noexcept
{
    auto extract_args = [](std::string_view s, std::string_view cmd) {
        if (s.size() < cmd.size())
            return std::string_view("");

        s.remove_prefix(cmd.size());

        while (!s.empty() && std::isspace(s.front()))
            s.remove_prefix(1);

        while (!s.empty() && std::isspace(s.back()))
            s.remove_suffix(1);

        return s;
    };

    auto s = extract_args(replCmd, CMD_SYM_UNSET);
    if (m_intr.unsetSymbol(s))
        printSymbolTable_();
}

void REPL::symbols_clear_() noexcept
{
    m_intr.clearSymbols();
    printSymbolTable_();
}

bool REPL::handleReplCmd(const std::string_view replCmd)
{
    if (replCmd == CMD_HELP)
        help_();
    else if (replCmd == CMD_QUIT)
        m_quit = true;
    else if (replCmd == CMD_SYM_TABLE)
        printSymbolTable_();
    else if (replCmd == CMD_LAST_VALUE)
        printLastValue_();
    else if (replCmd == CMD_SYM_CLEAR)
        symbols_clear_();
    else if (replCmd.starts_with(std::string(CMD_SYM_UNSET) + " "))
        symbol_unset_(replCmd);
    else
        return false;    // not processed

    return true;
}

int REPL::runLoop()
{
    banner_();

    int res = 0;
    m_quit  = false;
    while (!m_quit)
    {
        std::string input;

        std::cout << "\n$> ";
        std::getline(std::cin, input);

        // if (input.empty())
        //     continue;

        // Handling REPL special commands
        if (!input.empty() && input[0] == ':')
        {
            if (handleReplCmd(input.substr(1)))
                continue;
        }

        m_lex.setInput(std::make_unique<std::stringstream>(input));
        if (!m_parser.parse())
            continue;

        if (!m_intr.eval(m_parser.ast()))
            continue;

        printLastValue_();
    }

    return res;
}
