cmake_minimum_required(VERSION 3.18)
project(facet-theme-halloween VERSION 0.1.0 LANGUAGES CXX)
set(THEME_VERSION_SUFFIX "alpha")  # pre-release tag; empty for final releases; keep manifest.json in sync

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# The theme SDK lives in facet-core (0.8 and later): a local checkout next to
# this project (or FACET_CORE_DIR) is used when present, otherwise it is
# downloaded.
set(FACET_CORE_DIR "" CACHE PATH "facet-core checkout providing the SDK (default: ../facet-core, else download)")
set(FACET_CORE_REF "v0.8.0-alpha" CACHE STRING "facet-core git ref to download the SDK from")
if(NOT FACET_CORE_DIR AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../facet-core/sdk/CMakeLists.txt")
    set(FACET_CORE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../facet-core")
endif()
if(FACET_CORE_DIR)
    add_subdirectory(${FACET_CORE_DIR}/sdk ${CMAKE_BINARY_DIR}/facet_sdk)
else()
    include(FetchContent)
    FetchContent_Declare(facet_core
        GIT_REPOSITORY https://github.com/siakinnik/facet-core.git
        GIT_TAG ${FACET_CORE_REF}
        GIT_SHALLOW TRUE
        SOURCE_SUBDIR sdk)
    FetchContent_MakeAvailable(facet_core)
endif()

# The generator runs here, at build time; only its output goes to the device.
add_executable(halloween-theme src/main.cpp)
target_link_libraries(halloween-theme PRIVATE facet_sdk)
target_compile_options(halloween-theme PRIVATE -Wall -Wextra)

# The build dir doubles as a module dir: FACET_PLUGIN_PATH=<build>/theme-halloween.plugin
set(PLUGIN_OUT ${CMAKE_BINARY_DIR}/theme-halloween.plugin)
file(MAKE_DIRECTORY ${PLUGIN_OUT})
facet_plugin_manifest(${CMAKE_CURRENT_SOURCE_DIR}/manifest.json ${PLUGIN_OUT}/manifest.json)
add_custom_command(
    OUTPUT ${PLUGIN_OUT}/halloween.json
    COMMAND halloween-theme ${PLUGIN_OUT}/halloween.json
    DEPENDS halloween-theme
    COMMENT "Writing halloween.json")
add_custom_target(theme ALL DEPENDS ${PLUGIN_OUT}/halloween.json)
