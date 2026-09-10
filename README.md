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
## Compilador (Local)
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
Se requiere Node.js y [pnpm](https://pnpm.io/). El editor es una app Vite en `frontend/editor`.

```bash
cd frontend/editor
pnpm install
pnpm build
```

`pnpm build` escribe `frontend/editor/dist`, que es lo que sirve el backend en `/editor`. Para desarrollar el editor sin el servidor Go:

```bash
cd frontend/editor
pnpm dev
```

## Backend
Trabajo en progreso.

# Estado actual
## Compilador
El compilador es compilable localmente, no existe target a WASM aun, pero produce una ROM la cual ya es ejecutable en emuladores. Los objetos no tienen sistema de renderizacion aun.

## Backend
Trabajo a empezar, el plan es hacer una plataforma de distribucion y almacenacion de proyectos JSON y un sistema de cuentas.

## Frontend
El editor visual (Blockly / Zelos) permite armar escenas, objetos, variables y bloques, y exportar el proyecto como JSON `spec/BLOQUES.md`. No hay WebAssembly ni compilacion a ROM desde el navegador aun. Aun no existe frontend para el sitio de cuentas.