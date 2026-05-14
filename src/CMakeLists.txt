cmake_minimum_required(VERSION 3.28)

find_library(GMP_LIBRARY NAMES gmp REQUIRED)
find_library(MPFR_LIBRARY NAMES mpfr REQUIRED)

add_library(lex_scanner)
target_sources(lex_scanner PRIVATE
    LexScanner.cpp
)
target_include_directories(lex_scanner PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)

add_library(parser)
add_dependencies(parser lex_scanner)
target_sources(parser PRIVATE
    ParserLL1.cpp
    AST.cpp
  )
target_include_directories(parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)
target_link_libraries(parser PUBLIC
    lex_scanner
    boost_mp
    ${GMP_LIBRARY}
    ${MPFR_LIBRARY}
 )

add_library(interpreter)
add_dependencies(interpreter parser)
target_sources(interpreter PRIVATE
    SymbolTable.cpp
    Interpreter.cpp
  )
target_include_directories(interpreter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)
target_link_libraries(interpreter PUBLIC parser)

add_library(solver)
add_dependencies(solver parser)
target_sources(solver PRIVATE
    Solver.cpp
    PolynomialForm.cpp
  )
target_include_directories(solver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)
target_link_libraries(solver PUBLIC parser)

add_executable(symbols main.cpp)
target_compile_definitions(symbols PUBLIC
)
target_compile_options(symbols PRIVATE
    $<$<CXX_COMPILER_ID:GCC>: -Wall -Wextra>
)

target_sources(symbols PRIVATE
    REPL.cpp
)

target_include_directories(symbols PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/
)
target_link_libraries(symbols PRIVATE
    interpreter
    solver
)
