#!/bin/bash
set -euo pipefail

cd "$(dirname "$0")"

if [ -z "${EMSDK_PATH:-}" ]; then
	echo "EMSDK_PATH no esta definido"
	exit 1
fi

if [ ! -f "$EMSDK_PATH/emsdk_env.sh" ]; then
	echo "No se encontro $EMSDK_PATH/emsdk_env.sh"
	exit 1
fi

source "$EMSDK_PATH/emsdk_env.sh" --quiet

rm -rf build-wasm

emcmake cmake -S . -B build-wasm -DCMAKE_BUILD_TYPE=Release -DSUPERBLOQUES_WASM=ON
cmake --build build-wasm -j"$(nproc)"

if [ ! -f frontend/editor/public/compilador/superbloques.js ]; then
	echo "No se genero frontend/editor/public/compilador/superbloques.js"
	exit 1
fi
if [ ! -f frontend/editor/public/compilador/superbloques.wasm ]; then
	echo "No se genero frontend/editor/public/compilador/superbloques.wasm"
	exit 1
fi

echo "WASM listo en frontend/editor/public/compilador"
