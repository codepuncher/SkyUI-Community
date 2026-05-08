#[=======================================================================[.rst:
PackageDebug
------------

Build-time helper to package the debug dist/ directory as a
Vortex-installable ZIP for in-game testing.

Using a fixed archive name (SkyUI_SE_dev.zip) means Vortex recognises
each new build as an update to the same mod entry — no manual
disable/remove step required.

Variables expected:
  DIST_DIR    - path to the debug dist/ directory (loose files)
  ZIP_OUTPUT  - absolute path for the output .zip

#]=======================================================================]

foreach(_v DIST_DIR ZIP_OUTPUT)
    if(NOT DEFINED ${_v} OR "${${_v}}" STREQUAL "")
        message(FATAL_ERROR "PackageDebug.cmake: ${_v} is not set.")
    endif()
endforeach()

if(NOT EXISTS "${DIST_DIR}")
    message(FATAL_ERROR "PackageDebug.cmake: DIST_DIR does not exist: ${DIST_DIR}")
endif()

# Phase 1 (FormCache / Scaleform API): only the DLL is needed.
# SWFs, BSA, ESP, and Papyrus scripts all come from the official SkyUI mod
# in the Vortex Dev profile.  The ActionScript fast path (Phase 2) will add
# SWF compilation back when the time comes.
set(_TOP_ENTRIES "SKSE")

if(NOT EXISTS "${DIST_DIR}/SKSE/Plugins/SkyUI_SE.dll")
    message(FATAL_ERROR "PackageDebug.cmake: SkyUI_SE.dll not found. Build source/plugin/ first.")
endif()

get_filename_component(_ZIP_DIR "${ZIP_OUTPUT}" DIRECTORY)
file(MAKE_DIRECTORY "${_ZIP_DIR}")

message(STATUS "Packaging dev build -> ${ZIP_OUTPUT}")
foreach(_e ${_TOP_ENTRIES})
    message(STATUS "  + ${_e}")
endforeach()

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E tar cf "${ZIP_OUTPUT}" --format=zip ${_TOP_ENTRIES}
    WORKING_DIRECTORY "${DIST_DIR}"
    RESULT_VARIABLE _result
)

if(_result)
    message(FATAL_ERROR "PackageDebug.cmake: zip failed (exit code ${_result})")
endif()

message(STATUS "Done: ${ZIP_OUTPUT}")
