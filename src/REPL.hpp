#pragma once

#include "LexScanner.hpp"
#include "ParserLL1.hpp"
#include "Interpreter.hpp"

#include <memory>
#include <sstream>
#include <string_view>

class REPL
{
private:
    static constexpr std::string_view CMD_HELP       = "help";
    static constexpr std::string_view CMD_QUIT       = "quit";
    static constexpr std::string_view CMD_SYM_TABLE  = "symbols";
    static constexpr std::string_view CMD_LAST_VALUE = "?";
    static constexpr std::string_view CMD_SYM_UNSET  = "unset";
    static constexpr std::string_view CMD_SYM_CLEAR  = "sym_clear";


    LexScanner  m_lex    = LexScanner(std::make_unique<std::stringstream>(""));
    ParserLL1   m_parser = ParserLL1(m_lex);
    Interpreter m_intr;
    bool        m_quit = false;

    void banner_() const noexcept;
    void help_() const noexcept;
    void printSymbolTable_() const noexcept;
    void printLastValue_() const noexcept;
    void symbol_unset_(const std::string_view replCmd) noexcept;
    void symbols_clear_() noexcept;

    bool handleReplCmd(const std::string_view replCmd);

public:
    int runLoop();
};
