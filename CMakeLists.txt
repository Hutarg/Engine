cmake_minimum_required(VERSION 3.16)

project(Blueberry
    VERSION 1.0.0
    DESCRIPTION "Moteur de jeu - Game Engine"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GenerateExportHeader)

# Recherche des dépendences graphiques
find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

# Crée la librairie
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_library(Blueberry SHARED ${ENGINE_SOURCES})


# Crée le fichier d'export (Export.hpp) de la librairie
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/blueberry")
generate_export_header(Blueberry EXPORT_MACRO_NAME BLUEBERRY_API EXPORT_FILE_NAME 
    "${CMAKE_CURRENT_BINARY_DIR}/blueberry/Export.hpp")

target_link_libraries(Blueberry PRIVATE Vulkan::Vulkan glfw)

target_include_directories(Blueberry 
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/blueberry>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/blueberry>"
        "$<INSTALL_INTERFACE:include>"
        "$<INSTALL_INTERFACE:include/blueberry>"
)

# Installe les fichiers d'en-tête
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/blueberry"
    DESTINATION include
    FILES_MATCHING
        PATTERN "*.hpp"
        PATTERN "*.h"
)

# Installe le fichier d'export
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/blueberry/Export.hpp"
    DESTINATION include/blueberry
)

# Installe les librairies
install(TARGETS Blueberry
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
)

# Installe la documentation et la licence
install(FILES README.md LICENSE DESTINATION .)
