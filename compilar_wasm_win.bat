@echo off
setlocal EnableExtensions EnableDelayedExpansion
cd /d "%~dp0"

if not exist "..\emsdk\emsdk_env.bat" (
	echo No se encontro ..\emsdk\emsdk_env.bat
	exit /b 1
)

call ..\emsdk\emsdk_env.bat
if errorlevel 1 exit /b 1

if exist build-wasm rmdir /s /q build-wasm

emcmake cmake -G "MinGW Makefiles" -S . -B build-wasm -DCMAKE_BUILD_TYPE=Release -DSUPERBLOQUES_WASM=ON
if errorlevel 1 exit /b 1

cmake --build build-wasm -j 8
if errorlevel 1 exit /b 1

if not exist "frontend\editor\public\compilador\superbloques.js" (
	echo No se genero frontend\editor\public\compilador\superbloques.js
	exit /b 1
)
if not exist "frontend\editor\public\compilador\superbloques.wasm" (
	echo No se genero frontend\editor\public\compilador\superbloques.wasm
	exit /b 1
)

echo Compilador WASM listo en frontend\editor\public\compilador

if not exist "frontend\editor\public\emu" mkdir "frontend\editor\public\emu"

set "EMU_SRCS="
for %%F in ("engine\emu\*.c") do set "EMU_SRCS=!EMU_SRCS! engine/emu/%%~nxF"
if "!EMU_SRCS!"=="" (
	echo No se encontraron fuentes en engine\emu
	exit /b 1
)

emcc -O3 -fno-exceptions -fno-rtti ^
	-s WASM=1 ^
	-s MODULARIZE=1 ^
	-s EXPORT_ES6=1 ^
	-s ENVIRONMENT=web ^
	-s ALLOW_MEMORY_GROWTH=1 ^
	-s INITIAL_MEMORY=67108864 ^
	-s "EXPORTED_FUNCTIONS=[_startWithRom,_stopEmulator,_mainLoop,_getScreenBuffer,_getSoundBuffer,_setJoypadInput,_malloc,_free]" ^
	-s "EXPORTED_RUNTIME_METHODS=[HEAP8,HEAP16,HEAP32,HEAPU8,HEAPU16,HEAPU32,HEAPF32]" ^
	-I engine/emu ^
	!EMU_SRCS! ^
	-o frontend\editor\public\emu\snes9x.js
if errorlevel 1 exit /b 1

if not exist "frontend\editor\public\emu\snes9x.js" (
	echo No se genero frontend\editor\public\emu\snes9x.js
	exit /b 1
)
if not exist "frontend\editor\public\emu\snes9x.wasm" (
	echo No se genero frontend\editor\public\emu\snes9x.wasm
	exit /b 1
)

echo Emulador WASM listo en frontend\editor\public\emu
exit /b 0
