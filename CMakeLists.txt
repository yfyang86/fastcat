cmake_minimum_required(VERSION 3.16)
project(fastcat VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(fastcat
    src/main.cpp
    src/args.cpp
    src/file_reader.cpp
    src/syntax_highlight.cpp
    src/csv_formatter.cpp
    src/theme.cpp
    src/pager.cpp
)

target_include_directories(fastcat PRIVATE include)

target_compile_options(fastcat PRIVATE
    $<$<CXX_COMPILER_ID:Clang>:-fcolor-diagnostics>
)
