# Proyecto SuperBloques
**"Diseño e implementacion de un entorno de programacion visual orientado al desarrollo de software para arquitecturas de 16 bits"**

SuperBloques es un entorno de desarrollo visual pensado para arquitecturas de 16 bits basadas en la CPU **WDC 65C816** y sus variantes. El objetivo concreto de esta primera etapa es la **Super Nintendo Entertainment System (SNES)**.

La idea es que puedas armar logica, graficos y comportamiento de un programa/juego arrastrando bloques, y que el sistema compile eso a una ROM ejecutable en emuladores y compatible con el hardware real.

## ¿Que intenta resolver?

Programar para SNES hoy implica lidiar con:

- Ensamblador 65C816 y variantes
- Mapa de memoria, banks y modo nativo vs emulacion
- Inicializacion de PPU, VRAM, CGRAM, OAM
- Tilemaps, sprites, paletas y modos de video
- Toolchains poco amigables si vienes de un editor visual

SuperBloques apunta a tapar ese hueco: un editor visual + un compilador para generar un ROM, sin perder de vista que el target real es hardware de 16 bits.

## Componentes del proyecto

El repo se divide en tres frentes.

| Componente | Que es |
|---|---|
| **Compilador** | Pipeline que toma la representacion del proyecto y genera una ROM SNES |
| **Frontend** | Sitio y editor visual de bloques, tilemaps, objetos y proyectos |
| **Backend** | Plataforma de cuentas + almacenamiento/distribucion de proyectos |

# Instrucciones de compilacion
## Compilador
Se requiere un toolchain capaz de utilizar CMake (Version 3.16 o superior) y gcc/g++ (Version 11.4 o superior).

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
## Compilador
El compilador es compilable localmente, no existe target a WASM aun, pero produce una ROM la cual ya es ejecutable en emuladores, pero ya que no hay configuracion de video, el color de la pantalla es asignado a un timer estatico. Emuladores de depuracion de codigo como BSNES muestran que la CPU si esta corriendo. El siguiente paso es inicializar la VRAM de la consola y los registers de video para mostrar una pantalla valida. Despues de eso, seria la implementacion de todos los bloques programables, visualizacion de objetos, serializacion y carga de JSON para proyectos, modificacion de tilemaps y el codigo para generar estos.

## Backend
Trabajo a empezar, el plan es hacer una plataforma de distribucion y almacenacion de proyectos JSON y un sistema de cuentas.

## Frontend
Existe un mockup para un editor visual, aunque no hay codigo de implementacion o WebAssembly aun. Aun no existe implementacion o frontend para el sitio actual.