#!/bin/bash
set -euo pipefail
cmake -S . -B build
cmake --build build --config Release -j"$(nproc)"
./superbloques_cli -i ejemplos/pruebas.json