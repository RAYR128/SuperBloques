@echo off
setlocal EnableExtensions
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

echo WASM listo en frontend\editor\public\compilador
exit /b 0
