#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Bancos LoROM reservados para blobs (32KB cada uno, $8000-$FFFF).
#define BANCO_DATOS_PRIMERO 0x10
#define BANCO_DATOS_ULTIMO 0x3F
#define TAMANO_BANCO_DATOS 0x8000
#define CANTIDAD_BANCOS_DATOS (BANCO_DATOS_ULTIMO - BANCO_DATOS_PRIMERO + 1)

struct BloqueDato {
	std::string etiqueta;
	const uint8_t *datos;
	uint32_t tamano;
};

// First Fit Decreasing: un blob nunca cruza de banco.
void EmpaquetarDatosROM(std::vector<BloqueDato> bloques);
