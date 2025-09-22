cmake_minimum_required(VERSION 3.16)
project(Linked_List)

set(CMAKE_CXX_STANDARD 20)

add_executable(Linked_List
        LinkedList.cpp
        CSVparser.cpp
)

target_include_directories(Linked_List PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Simple run target (uses sample CSV in repo)
add_custom_target(run
    COMMAND $<TARGET_FILE:Linked_List> cmake-build-debug/eBid_Monthly_Sales.csv 98109
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS Linked_List
    COMMENT "Running Linked_List (interactive)"
)
