#!/bin/sh
# Builds TourneIconsKit.exe with MinGW-w64 (on Linux, or MSYS2 on Windows).
# Usage: tools/icons/kit/build.sh [output folder]
set -e
here=$(cd "$(dirname "$0")" && pwd)
out=${1:-$here/out}
mkdir -p "$out"
cxx=${CXX:-x86_64-w64-mingw32-g++-posix}
windres=${WINDRES:-x86_64-w64-mingw32-windres}
"$windres" -I "$here" "$here/kit.rc" -O coff -o "$out/kit.res"
"$cxx" -std=c++20 -O2 -s -municode -mwindows \
    -DUNICODE -D_UNICODE -DWINVER=0x0A00 -D_WIN32_WINNT=0x0A00 \
    -I "$here" "$here/kit.cpp" "$out/kit.res" -o "$out/TourneIconsKit.exe" -static \
    -lole32 -loleaut32 -luuid -lshlwapi -lshell32 -lwindowscodecs -lversion -lcomdlg32 -lcomctl32 \
    -luxtheme -ldwmapi -lpsapi -ld2d1 -lntdll -ladvapi32 -lgdi32 -luser32 -lurlmon
echo "$out/TourneIconsKit.exe"
