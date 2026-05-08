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

set(_TOP_ENTRIES "SKSE")

# Phase 2 (AS3 cache fast-path): include only the four inventory SWFs that
# embed InventoryDataSetter (the class modified to use the cache fast path).
# These override the corresponding files in the user's existing SkyUI install.
set(_INVENTORY_SWFS
    inventorymenu.swf
    containermenu.swf
    bartermenu.swf
    giftmenu.swf
)

set(_INTERFACE_DIR "${DIST_DIR}/interface")
foreach(_swf ${_INVENTORY_SWFS})
    if(EXISTS "${_INTERFACE_DIR}/${_swf}")
        list(APPEND _TOP_ENTRIES "interface/${_swf}")
    else()
        message(WARNING "PackageDebug.cmake: ${_swf} not found in dist/interface/ — SWF will be missing from zip.")
    endif()
endforeach()

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
