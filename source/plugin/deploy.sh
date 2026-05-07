#!/usr/bin/env bash
# Copies the built SkyUI_SE.dll to the Skyrim SE SKSE plugins folder for testing.

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DLL="$SCRIPT_DIR/build/debug-clang-cl-vcpkg-ae/SkyUI_SE.dll"
DEST="/home/lee/.local/share/Steam/steamapps/common/Skyrim Special Edition/Data/SKSE/Plugins"

if [[ ! -f "$DLL" ]]; then
    echo "ERROR: DLL not found at $DLL"
    echo "Run: cd source/plugin && VCPKG_ROOT=~/.local/share/vcpkg cmake --build --preset clang-linux-se"
    exit 1
fi

cp "$DLL" "$DEST/"
echo "Deployed: $DLL → $DEST/SkyUI_SE.dll"
