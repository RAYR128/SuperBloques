# Proyecto SuperBloques
“Diseño e implementación de un entorno de programación visual orientado al desarrollo de software para arquitecturas de 16 bits”

Este es un proyecto el cual intenta implementar un entorno de desarrollo visual para arquitecturas de 16-bit que utilizan la CPU WDC 65C816, en este caso, fue elegida la consola Super Nintendo

# Instrucciones de compilacion
## Compilador
Se requiere un toolchain capaz de utilizar CMake y GCC/G++ (Version 11.4 o superior).

En windows:

```bat
cmake -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=cmake/windows.cm -DCMAKE_BUILD_TYPE=Release . -B build-windows
cd build-windows
make -j8
pause
```

En linux (Ubuntu 22.04 o distribuciones basadas en debian):

```bat
sudo apt update && sudo apt install -y build-essential cmake gdb

cmake -S . -B build
cmake --build build --config Release -j"$(nproc)"
```

## Frontend
Trabajo en progreso.

## Backend
Trabajo en progreso.

# Estado actual
Compilador: El compilador produce una ROM la cual es ejecutable en emuladores, pero ya que no hay configuracion de video, el color de la pantalla es asignado a un timer estatico. Emuladores de depuracion de codigo como BSNES muestran que la CPU si esta corriendo, pero falta la configuracion de video. El siguiente paso es inicializar la VRAM de la consola y los registers de video para mostrar una pantalla valida. Despues de eso, seria la implementacion de todos los bloques programables, visualizacion de objetos, modificacion de tilemaps y el codigo para generar estos.

Backend: Trabajo a empezar, el plan es hacer una plataforma de distribucion y almacenacion de proyectos JSON y un sistema de cuentas.

Frontend: Trabajo a empezar, el plan es hacer un editor visual que pueda llamar al compilador a traves de WebAssembly, añadir bloques, y tener un viewport/canvas que muestre el juego corriendo, informacion de depuracion, entre otras herramientas.