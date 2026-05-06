#pragma once

#include "LexScanner.hpp"
#include "ParserLL1.hpp"
#include "Interpreter.hpp"
#include "Solver.hpp"

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
    static constexpr std::string_view CMD_EVAL       = "eval";
    static constexpr std::string_view CMD_SOLVER     = "solver";

    enum class eType
    {
        EVAL,
        SOLVER,
    };

    LexScanner  m_lex    = LexScanner(std::make_unique<std::stringstream>(""));
    ParserLL1   m_parser = ParserLL1(m_lex);
    Interpreter m_intr;
    Solver      m_solver;

    eType m_type = eType::EVAL;
    bool  m_quit = false;

    std::string extract_args_(std::string_view s, std::string_view cmd);

    void banner_() const noexcept;
    void help_() const noexcept;
    void printSymbolTable_() const noexcept;
    void printLastValue_() const noexcept;
    void symbol_unset_(const std::string_view replCmd) noexcept;
    void symbols_clear_() noexcept;

    void printShellLine_() const;
    bool handleReplCmd(const std::string_view replCmd);


public:
    REPL()  = default;
    ~REPL() = default;

    REPL(const REPL&) = delete;
    REPL(REPL&&)      = delete;

    REPL& operator=(const REPL&) = delete;
    REPL& operator=(REPL&&)      = delete;

    int runLoop();
};
