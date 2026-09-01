#!/bin/bash
set -euo pipefail
find ./engine \
	\( -path ./engine/lib \) -prune -o \
	-type f \( -name "*.c" -o -name "*.h" -o -name "*.cpp" -o -name "*.hpp" \) \
	-print0 | xargs -0 -t clang-format-16 -i -style=file