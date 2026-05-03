
cmake_minimum_required(VERSION 3.28)

find_package(GTest 1.13 CONFIG REQUIRED)
include(GoogleTest)
include(CMakeParseArguments)

macro(macro_test) # ${ARGN} link libraries
    
    set(prefix TEST)
    #set(flags DEFAULT)
    set(singleValues EXE)
    set(multiValues FILES FIXTURES LINKS_PRIVATE)

    cmake_parse_arguments(${prefix} "${flags}" "${singleValues}" "${multiValues}" ${ARGN})
    add_executable (${TEST_EXE} ${TEST_FILES})
    target_include_directories(${TEST_EXE} PUBLIC
        ${PROJECT_SOURCE_DIR}/test
        # ${CMAKE_SOURCE_DIR}/
        ${TEST_INCLUDE_DIR}
    )

    target_link_libraries(${TEST_EXE} PRIVATE GTest::gtest GTest::gmock ${PROJECT_TEST} ${PROJECT_SYS_NULL})

    foreach(component_i ${PROJECT_TEST})
        get_target_property(private_include_dirs ${component_i} INCLUDE_DIRECTORIES)
        get_target_property(private_link_libraries ${component_i} LINK_LIBRARIES)
        target_include_directories(${TEST_EXE} PRIVATE ${private_include_dirs})
        target_link_libraries(${TEST_EXE} PRIVATE ${private_link_libraries} ${PROJECT_TEST})
    endforeach()
    unset(private_include_dirs)
    unset(private_link_libraries)

    add_custom_command(TARGET ${TEST_EXE} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:GTest::gtest_main> $<TARGET_FILE_DIR:${TEST_EXE}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:GTest::gtest> $<TARGET_FILE_DIR:${TEST_EXE}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:GTest::gmock> $<TARGET_FILE_DIR:${TEST_EXE}>
    )

    set(fixtures ${TEST_FIXTURES})
    list(LENGTH fixtures num_fixtures)
    unset(fixtures)
    if(num_fixtures EQUAL 0)
    else()
        foreach(fixture_i ${TEST_FIXTURES})
            add_custom_command(TARGET ${TEST_EXE} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        ${CMAKE_CURRENT_SOURCE_DIR}/${fixture_i}
                        ${CMAKE_CURRENT_BINARY_DIR}/${fixture_i}
            )
        endforeach()
    endif()

    target_link_libraries(${TEST_EXE} PRIVATE ${TEST_LINKS_PRIVATE})
    set_property(TARGET ${TEST_EXE} PROPERTY COMPILE_WARNING_AS_ERROR ON)

    gtest_discover_tests(${TEST_EXE} PROPERTIES TEST_DISCOVERY_TIMEOUT 600 DISCOVERY_TIMEOUT 600)
endmacro()

macro_test(
    EXE TestLexScanner
    FILES "TestLexScanner.cpp"
    FIXTURES 
    LINKS_PRIVATE "lex_scanner"
)

macro_test(
    EXE TestParserLL1
    FILES "TestParserLL1.cpp"
    FIXTURES 
    LINKS_PRIVATE "parser"
)

macro_test(
    EXE TestInterpreter
    FILES "TestInterpreter.cpp"
    FIXTURES 
    LINKS_PRIVATE "interpreter"
)
