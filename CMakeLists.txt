cmake_minimum_required(VERSION 3.28)
project(symbols)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)

include (CTest)
enable_testing()



add_subdirectory(src)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()