include(FetchContent)
include(ExternalProject)

# Boost Multiprecision
set(CMAKE_CXX_STANDARD 23)
FetchContent_Declare(
    boost_mp
    GIT_REPOSITORY https://github.com/boostorg/multiprecision.git
    GIT_TAG        Boost_1_89_0
)

FetchContent_MakeAvailable(boost_mp)
# Add include directory manually (Boost modules do not provide CMake targets)
add_library(boost_mp INTERFACE)
target_include_directories(boost_mp INTERFACE
    ${boost_mp_SOURCE_DIR}/include
)

unset(CMAKE_CXX_STANDARD)
