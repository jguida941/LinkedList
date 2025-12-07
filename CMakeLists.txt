cmake_minimum_required(VERSION 3.16)
project(Linked_List)

set(CMAKE_CXX_STANDARD 20)

# Main executable
add_executable(Linked_List
        src/LinkedList.cpp
        src/CSVparser.cpp
)

target_include_directories(Linked_List PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Simple run target (uses sample CSV in repo)
add_custom_target(run
    COMMAND $<TARGET_FILE:Linked_List> data/eBid_Monthly_Sales.csv
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS Linked_List
    COMMENT "Running Linked_List (interactive)"
)

#============================================================================
# Unit Testing with Catch2
#============================================================================
option(BUILD_TESTS "Build unit tests" ON)

if(BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.0
    )
    FetchContent_MakeAvailable(Catch2)

    add_executable(tests
        tests/test_linkedlist.cpp
    )
    target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

    # Enable CTest integration
    include(CTest)
    include(Catch)
    catch_discover_tests(tests)

    # Quick test target
    add_custom_target(test_run
        COMMAND tests
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS tests
        COMMENT "Running unit tests"
    )
endif()
