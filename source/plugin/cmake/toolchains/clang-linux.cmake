# Cross-compile toolchain: Linux host → Windows x64 (MSVC ABI)
# Uses clang-cl + lld-link with Windows SDK/CRT headers from xwin

set(CMAKE_SYSTEM_NAME    Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# xwin splatted output (Windows SDK + MSVC CRT)
set(XWIN_DIR "$ENV{HOME}/.xwin" CACHE PATH "xwin splat output dir")

# clang-cl: Clang in MSVC compatibility mode — uses MSVC ABI and accepts /flags
set(CMAKE_C_COMPILER   /usr/bin/clang-cl CACHE FILEPATH "")
set(CMAKE_CXX_COMPILER /usr/bin/clang-cl CACHE FILEPATH "")
set(CMAKE_AR           /usr/bin/llvm-lib CACHE FILEPATH "")
set(CMAKE_LINKER       /usr/bin/lld-link CACHE FILEPATH "")
set(CMAKE_MT           /usr/bin/llvm-mt CACHE FILEPATH "")
set(CMAKE_RC_COMPILER  /usr/bin/llvm-rc  CACHE FILEPATH "")

# Target x64 Windows (MSVC ABI)
set(CMAKE_C_COMPILER_TARGET   x86_64-pc-windows-msvc)
set(CMAKE_CXX_COMPILER_TARGET x86_64-pc-windows-msvc)

# Force release dynamic CRT for all build types:
# - xwin only ships msvcrt.lib (no msvcrtd.lib)
# - SKSE plugins must match the game's CRT (release)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL" CACHE STRING "")

# Windows SDK and CRT include paths (/imsvc = system include, no warnings from these headers)
set(CMAKE_C_FLAGS_INIT   "/imsvc\"${XWIN_DIR}/crt/include\" /imsvc\"${XWIN_DIR}/sdk/include/ucrt\" /imsvc\"${XWIN_DIR}/sdk/include/um\" /imsvc\"${XWIN_DIR}/sdk/include/shared\"")
set(CMAKE_CXX_FLAGS_INIT "/imsvc\"${XWIN_DIR}/crt/include\" /imsvc\"${XWIN_DIR}/sdk/include/ucrt\" /imsvc\"${XWIN_DIR}/sdk/include/um\" /imsvc\"${XWIN_DIR}/sdk/include/shared\"")

# Windows SDK and CRT library paths (lld-link style: /libpath:)
set(_XWIN_LIBFLAGS "/libpath:\"${XWIN_DIR}/crt/lib/x86_64\" /libpath:\"${XWIN_DIR}/sdk/lib/um/x86_64\" /libpath:\"${XWIN_DIR}/sdk/lib/ucrt/x86_64\"")
set(CMAKE_EXE_LINKER_FLAGS_INIT    "${_XWIN_LIBFLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "${_XWIN_LIBFLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "${_XWIN_LIBFLAGS}")

set(CMAKE_FIND_ROOT_PATH "${XWIN_DIR}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
