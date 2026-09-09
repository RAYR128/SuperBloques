@echo off
setlocal EnableExtensions

cd backend

echo Instalando dependencias
go mod tidy
if errorlevel 1 goto :fail

echo Build de aplicacion..
go build -v -o superbloques_b.exe ./cmd/main
if errorlevel 1 goto :fail

superbloques_b.exe
set "EXITCODE=%ERRORLEVEL%"
popd
exit /b %EXITCODE%

:fail
echo.
echo Fallo script de prueba de backend.
popd
exit /b 1
