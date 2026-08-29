cmake_minimum_required(VERSION 3.20)
project(Bernardo C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

add_compile_options(-Wall -Werror)

add_executable(Bernardo
        src/main.c
        src/line_reader.c
        src/token_scanner.c
        src/dish_table.c
        src/formula.c
        src/formula_parser.c
        src/sat_propagate.c
        src/sat_solver.c
        src/threshold_search.c
        src/result_printer.c
)

target_include_directories(Bernardo PRIVATE src)
