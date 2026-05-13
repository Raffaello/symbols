include(FetchContent)
include(ExternalProject)

# Boost Multiprecision
fetchContent_declare(
    boost_mp
    GIT_REPOSITORY https://github.com/boostorg/multiprecision.git
    GIT_TAG        Boost_1_89_0
)

fetchContent_makeAvailable(boost_mp)
# Add include directory manually (Boost modules do not provide CMake targets)
add_library(boost_mp INTERFACE)
target_include_directories(boost_mp INTERFACE
    ${boost_mp_SOURCE_DIR}/include
)
