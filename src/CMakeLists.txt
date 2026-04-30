cmake_minimum_required(VERSION 3.28)


add_library(scanner Scanner.cpp)
target_include_directories(scanner PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)

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
    scanner
)
