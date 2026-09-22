cmake_minimum_required(VERSION 4.0)

project(voa LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_program(CLANG_FORMAT_EXECUTABLE NAMES clang-format REQUIRED)
find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy REQUIRED)

if(MINGW)
    set(CMAKE_CXX_STANDARD_LIBRARIES "-lstdc++exp")
endif()

set(VOA_SOURCES
    libvoa/src/voa.cpp
    example/example-01.cpp
)

set(VOA_HEADERS
    libvoa/include/voa.hpp
)

add_library(voa STATIC
    libvoa/src/voa.cpp
)

target_include_directories(voa
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/libvoa/include
)

target_compile_features(voa PUBLIC cxx_std_23)

add_executable(example-01
    example/example-01.cpp
)

target_link_libraries(example-01 PRIVATE voa)

add_custom_target(clang-format
    COMMAND ${CLANG_FORMAT_EXECUTABLE}
        --style=file:${CMAKE_CURRENT_SOURCE_DIR}/.clang-format
        --i
        ${VOA_SOURCES}
        ${VOA_HEADERS}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    VERBATIM
)

add_custom_target(clang-tidy
    COMMAND ${CLANG_TIDY_EXECUTABLE}
        --config-file=${CMAKE_CURRENT_SOURCE_DIR}/.clang-tidy
        --header-filter=.*
        -p ${CMAKE_BINARY_DIR}
        ${VOA_SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS example-01
    VERBATIM
)

add_custom_target(clang-tidy-fix
    COMMAND ${CLANG_TIDY_EXECUTABLE}
        --config-file=${CMAKE_CURRENT_SOURCE_DIR}/.clang-tidy
        --header-filter=.*
        -p ${CMAKE_BINARY_DIR}
        -fix
        ${VOA_SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS example-01
    VERBATIM
)