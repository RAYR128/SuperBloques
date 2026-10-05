#pragma once
#include <string>

// Esto crea el programa en el PC actual con un header de 4 bytes separado en 2:
// 0x00-0x01: tamaño del bloque (base $XXXX)
// 0x02-0x03: dirección de carga del bloque
// Devuelve la direccion final (tamaño del bloque + direccion de carga) para poder encadenar bloques.
int EnsamblarPrograma(std::string &Codigo);