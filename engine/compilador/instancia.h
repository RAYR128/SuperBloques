#pragma once

#include "asm/memmap.h"
#include "bloques.h"
#include <string>
#include <vector>

// Una escena es un estado de control del programa.
class Escena {
  public:
	std::string Nombre;
	std::vector<NodoBloque> Bloques;
	std::vector<std::string> Variables;
	uint8_t GraficosPrincipales[0x400 * TILE_SIZE_4BPP];
	uint8_t GraficosHud[0x100 * TILE_SIZE_2BPP];
	uint8_t Tilemap1[64 * 64 * 2];
	uint8_t Tilemap2[32 * 32 * 2];
	uint8_t Tilemap3[32 * 32 * 2];
};

// Un objeto es una declaracion de un objeto el cual puede ser creado en una escena.
// Los objetos contienen "bloques" los cuales son compilados a scripts de behavior que indican como
// se comporta el objeto en la escena. Un objeto puede ser un enemigo, un item, un bloque, etc. La programacion es libre para el usuario.
class ObjetoEscena {
  public:
	std::string Nombre;
	std::vector<NodoBloque> Bloques;
	std::vector<std::string> Variables;
};

extern std::vector<Escena> EscenasProyecto;
extern std::vector<ObjetoEscena> ObjetosProyecto;