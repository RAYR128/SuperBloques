#pragma once
#include <string>

// Ensambla codigo SPC700 y lo escribe en el PC actual de la ROM. Cada base/org emite un bloque little-endian:
//  0x00-0x01: tamaño de los datos (0 para indicar fin)
//  0x02-0x03: direccion de carga en ARAM
//  0x04-...: datos
// Devuelve la direccion ARAM final del ultimo bloque (direccion de carga + tamaño).
// Si no hay datos, devuelve 0.
int EnsamblarProgramaSPC700(std::string &Codigo);