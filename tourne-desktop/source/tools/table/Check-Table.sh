#!/bin/sh
# Syntax-checks a Tourne'Table .wh.cpp without Windhawk, Windows or a GPU.
#
# Windhawk builds the real thing 32-bit with clang on the PC; this is the
# cheaper check a session anywhere can run before pushing. It uses the stub
# headers in check/: the Windhawk API, the parts of Direct2D 1.2 and 1.3 that
# mingw does not ship, and just enough WinRT for the media session code. None
# of them run; they only have to have the right shapes.
#
# A clean run prints nothing and exits 0.
#
#   tools/table/Check-Table.sh tourne-table-2.1.2-test.wh.cpp
set -e
here=$(cd "$(dirname "$0")" && pwd)
file=${1:-tourne-table-2.1.2-test.wh.cpp}
exec x86_64-w64-mingw32-g++ -std=c++20 -fsyntax-only -fpermissive \
    -I"$here/check" -include wh.h -DUNICODE -D_UNICODE "$file"
