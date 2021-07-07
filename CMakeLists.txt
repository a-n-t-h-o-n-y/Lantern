cmake_minimum_required(VERSION 3.14)
Project(MAZE LANGUAGES CXX)

add_subdirectory(external/TermOx)
add_subdirectory(external/MAZE)

add_executable(lantern
    src/lantern.main.cpp
)

target_compile_features(lantern
    PUBLIC
        cxx_std_20
)

target_compile_options(lantern
    PUBLIC
        -Wall
        -Wextra
        -Wpedantic
)

target_link_libraries(lantern PUBLIC TermOx maze-lib)
