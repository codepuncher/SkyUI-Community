vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO Microsoft/DirectXTK
    REF mar2026
    SHA512 cb302ae961cda328bf2b7f070e044b6bfba60ca8b1498763a1bc71ee38265654122e5f3a6a41ec4a5af8972fb1d5012aeaf42e3e3ef44151e2ed218a270892fb
    HEAD_REF main
)

# Install headers only — shader compilation requires fxc.exe (Windows-only).
# CommonLibSSE-NG only uses SimpleMath.h / DirectXMath types from these headers.
file(INSTALL "${SOURCE_PATH}/Inc/" DESTINATION "${CURRENT_PACKAGES_DIR}/include")

# Create a header-only CMake target so find_package(directxtk) works.
set(DIRECTXTK_TARGETS_FILE "${CURRENT_PACKAGES_DIR}/share/directxtk/directxtkTargets.cmake")
file(MAKE_DIRECTORY "${CURRENT_PACKAGES_DIR}/share/directxtk")

file(WRITE "${DIRECTXTK_TARGETS_FILE}" [[
add_library(Microsoft::DirectXTK INTERFACE IMPORTED GLOBAL)
set_target_properties(Microsoft::DirectXTK PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include"
)
]])

configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/directxtkConfig.cmake.in"
    "${CURRENT_PACKAGES_DIR}/share/directxtk/directxtkConfig.cmake"
    @ONLY
)

file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)

set(VCPKG_BUILD_TYPE release)
