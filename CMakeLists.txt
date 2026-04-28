cmake_minimum_required(VERSION 3.16)
project(Blueberry-Library 
    VERSION 1.0.0
    DESCRIPTION "Moteur de jeu - Game Engine"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    src/*.cpp
)

add_library(Engine SHARED ${ENGINE_SOURCES})

target_include_directories(Engine
    PUBLIC include
)

install(DIRECTORY include/ 
    DESTINATION include
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

# Installer les librairies
install(DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
    DESTINATION lib
)

# Documentation
install(FILES README.md LICENSE
    DESTINATION .
)
