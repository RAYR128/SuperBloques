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

echo "Compilador WASM listo en frontend/editor/public/compilador"

mkdir -p frontend/editor/public/emu
emcc -O3 -fno-exceptions -fno-rtti \
	-s WASM=1 \
	-s MODULARIZE=1 \
	-s EXPORT_ES6=1 \
	-s ENVIRONMENT=web \
	-s ALLOW_MEMORY_GROWTH=1 \
	-s INITIAL_MEMORY=67108864 \
	-s "EXPORTED_FUNCTIONS=[_startWithRom,_stopEmulator,_mainLoop,_getScreenBuffer,_getSoundBuffer,_setJoypadInput,_malloc,_free]" \
	-s "EXPORTED_RUNTIME_METHODS=[HEAP8,HEAP16,HEAP32,HEAPU8,HEAPU16,HEAPU32,HEAPF32]" \
	-I engine/emu \
	engine/emu/*.c \
	-o frontend/editor/public/emu/snes9x.js

if [ ! -f frontend/editor/public/emu/snes9x.js ]; then
	echo "No se genero frontend/editor/public/emu/snes9x.js"
	exit 1
fi
if [ ! -f frontend/editor/public/emu/snes9x.wasm ]; then
	echo "No se genero frontend/editor/public/emu/snes9x.wasm"
	exit 1
fi

echo "Emulador WASM listo en frontend/editor/public/emu"
