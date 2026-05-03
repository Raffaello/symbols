cmake_minimum_required(VERSION 3.28)


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
target_link_libraries(parser PUBLIC lex_scanner)

add_library(interpreter)
add_dependencies(interpreter parser)
target_sources(interpreter PRIVATE
    Interpreter.cpp
  )
target_include_directories(interpreter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)
target_link_libraries(interpreter PUBLIC parser)

add_executable(symbols main.cpp)
target_compile_definitions(symbols PUBLIC
)
target_compile_options(symbols PRIVATE
    $<$<CXX_COMPILER_ID:GCC>: -Wall -Wextra>
)

target_sources(symbols PRIVATE

)

target_include_directories(symbols PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/
)
target_link_libraries(symbols PRIVATE
    interpreter
)
