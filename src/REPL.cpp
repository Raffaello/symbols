#include "REPL.hpp"

#include <iostream>
#include <format>
#include <algorithm>
#include <stdexcept>
#include <ranges>

std::string REPL::extract_args_(std::string_view s, std::string_view cmd)
{
    if (s.size() < cmd.size())
        return "";

    s.remove_prefix(cmd.size());

    while (!s.empty() && std::isspace(s.front()))
        s.remove_prefix(1);

    while (!s.empty() && std::isspace(s.back()))
        s.remove_suffix(1);

    return std::string(s);
}

std::vector<std::string> REPL::splitString_(const std::string& str, char delimiter)
{
    std::vector<std::string> tokens;
    std::stringstream        ss(str);
    std::string              token;

    auto trim = [](std::string_view sv) {
        auto is_not_space = [](unsigned char ch) {
            return !std::isspace(ch);
        };

        auto first = std::ranges::find_if(sv, is_not_space);
        auto last  = std::ranges::find_if(sv | std::views::reverse, is_not_space).base();

        return std::string_view(first, last - first);
    };

    while (std::getline(ss, token, delimiter))
        tokens.emplace_back(trim(token));

    return tokens;
}

void REPL::banner_() const noexcept
{
    std::cout << std::format("Symbols REPL v0.1.0\n");
    std::cout << std::format("Type \":{}\" for more information.\n", CMD_HELP);
}

void REPL::help_() const noexcept
{
    std::cout << std::format(":{}\t\t => show this screen.\n", CMD_HELP);
    std::cout << std::format(":{}\t\t => exit the program.\n", CMD_QUIT);
    std::cout << std::format(":{}\t\t => switch to eval mode (Interpreter).\n", CMD_EVAL);
    std::cout << std::format(":{}\t\t => switch to solver mode (Solver).\n", CMD_SOLVER);
    std::cout << std::format(":{}\t => print the symbol table.\n", CMD_SYM_TABLE);
    std::cout << std::format(":{}\t\t => print the last computed value.\n", CMD_LAST_VALUE);
    std::cout << std::format(":{} [symbol]\t => unset a symbol requires a name after.\n", CMD_SYM_UNSET);
    std::cout << std::format(":{}\t => clear the symbol table.\n", CMD_SYM_CLEAR);
    std::cout << std::format("\n");
}

void REPL::printSymbolTable_() const noexcept
{
    // get longer symbol name
    const auto&       symTable       = m_intr.symbolTable();
    const std::size_t key_max_length = std::max<size_t>(symTable.key_max_length(), 7);

    std::cout << std::format("{:<{}} | {}\n", "Symbol", key_max_length, "Value");
    std::cout << std::string(std::max<size_t>(key_max_length + 8, 14), '-') << "\n";
    for (const auto& [k, v] : symTable.table())
        std::cout << std::format("{:<{}} | {}\n", k, key_max_length, v);
}

void REPL::printLastValue_() const noexcept
{
    std::cout << std::format("| $? = {}\n", m_intr.lastValue());
}

void REPL::symbol_unset_(const std::string_view replCmd) noexcept
{
    auto s = extract_args_(replCmd, CMD_SYM_UNSET);
    if (m_intr.unsetSymbol(s))
        printSymbolTable_();
}

void REPL::symbols_clear_() noexcept
{
    m_intr.clearSymbols();
    printSymbolTable_();
}

void REPL::printShellLine_() const
{
    switch (m_type)
    {
    case eType::EVAL:
        std::cout << "\n$eval> ";
        break;
    case eType::SOLVER:
        std::cout << "\n$solver> ";
        break;
    default:
        throw std::invalid_argument("m_type invalid");
    }
}

bool REPL::handleReplCmd(const std::string_view replCmd)
{
    if (replCmd == CMD_HELP)
        help_();
    else if (replCmd == CMD_QUIT)
        m_quit = true;
    else if (replCmd == CMD_EVAL)
        m_type = eType::EVAL;
    else if (replCmd == CMD_SOLVER)
        m_type = eType::SOLVER;
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

    m_quit = false;
    while (!m_quit)
    {
        std::string input;

        printShellLine_();
        if (!std::getline(std::cin, input))
        {
            std::cin.clear();
            continue;
        }

        // Handling REPL special commands
        if (!input.empty() && input[0] == ':')
        {
            if (handleReplCmd(input.substr(1)))
                continue;
        }

        // NOTE: split the input by ',',instead of doing a grammar for that,
        // prefer to doing at REPL level, (the grammar was done and then reverted, as that would have needed 2 grammars)
        std::vector<std::string> inputs = splitString_(input, ',');

        for (size_t i = 0; i < inputs.size(); ++i)
        {
            const std::string& in = inputs[i];

            m_lex.setInput(std::make_unique<std::stringstream>(in));
            if (!m_parser.parse())
                continue;

            switch (m_type)
            {
            case eType::EVAL:
                if (!m_intr.eval(m_parser.ast()))
                    continue;

                printShellLine_();
                std::cout << std::format("{}\n", m_intr.lastExpr());
                break;
            case eType::SOLVER:
            {
                ++i;
                if (i >= inputs.size())
                {
                    std::cerr << std::format("ERROR: missing symbol to solve for in '{}'\n Usage e.g: x+1=0, x\n, ", in);
                    break;
                }

                const std::string in2 = inputs[i];
                if (m_solver.solve(m_parser.ast(), in2))
                    std::cout << std::format("|> {}\n", m_solver.solution());
            }
            break;
            default:
                throw std::runtime_error("unknown m_type");
            }
        }
    }

    return 0;
}
