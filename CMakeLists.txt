cmake_minimum_required(VERSION 3.1)

project(DLP LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/includes/*.h")
file(GLOB_RECURSE SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/includes/*.cpp")

add_executable(DLP
    main.cpp
    ${HEADER_FILES}
    ${SOURCE_FILES}
    )

target_include_directories(DLP PRIVATE includes)