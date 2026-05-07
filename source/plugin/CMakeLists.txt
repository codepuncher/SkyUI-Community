cmake_minimum_required(VERSION 3.21)

include("${CMAKE_CURRENT_SOURCE_DIR}/../../Version.cmake")

project(
    SkyUI_Plugin
    VERSION ${VER_MAJOR}.${VER_MINOR}.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# Dependencies (provided by vcpkg via VCPKG_CHAINLOAD_TOOLCHAIN_FILE)
# ---------------------------------------------------------------------------

find_package(spdlog CONFIG REQUIRED)
find_path(RAPIDCSV_INCLUDE_DIRS "rapidcsv.h" REQUIRED)

# ---------------------------------------------------------------------------
# CommonLibSSE-NG (git submodule — alandtse/CommonLibVR ng branch)
# ---------------------------------------------------------------------------

set(ENABLE_SKYRIM_SE   OFF CACHE BOOL "" FORCE)
set(ENABLE_SKYRIM_AE   ON  CACHE BOOL "" FORCE)
set(ENABLE_SKYRIM_VR   OFF CACHE BOOL "" FORCE)
set(BUILD_TESTS        OFF CACHE BOOL "" FORCE)
set(SKSE_SUPPORT_XBYAK OFF CACHE BOOL "" FORCE)

add_subdirectory(
    "${CMAKE_CURRENT_SOURCE_DIR}/../../external/CommonLibSSE-NG"
    "${CMAKE_CURRENT_BINARY_DIR}/CommonLibSSE-NG"
    EXCLUDE_FROM_ALL
)

# add_commonlibsse_plugin is defined in CommonLibSSE.cmake
include("${CMAKE_CURRENT_SOURCE_DIR}/../../external/CommonLibSSE-NG/cmake/CommonLibSSE.cmake")

# ---------------------------------------------------------------------------
# Plugin DLL
# ---------------------------------------------------------------------------

configure_file(
    cmake/Version.h.in
    src/Version.h
    @ONLY
)

# add_commonlibsse_plugin replaces add_library + manual CommonLibSSE linkage.
# It auto-generates SKSEPluginInfo (name, author, version, address library compat)
# into __SkyUI_PluginPlugin.cpp — do NOT duplicate SKSEPluginInfo in main.cpp.
add_commonlibsse_plugin(${PROJECT_NAME}
    NAME    "SkyUI_SE"
    AUTHOR  "SkyUI Community"
    SOURCES src/main.cpp
    USE_ADDRESS_LIBRARY
)

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_BINARY_DIR}/src"
)

target_precompile_headers(${PROJECT_NAME} PRIVATE
    src/PCH.h
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME SkyUI_SE
    PREFIX      ""
    SUFFIX      ".dll"
)
