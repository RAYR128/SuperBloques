#include "rom.h"
#include <cstring>
#include <cstdio>

uint8_t DROM[TAMANO_ROM];

uint32_t ConvertirAddrHwAPc(uint32_t addrHw) {
	uint32_t bank = (addrHw >> 16) & 0x7F;
	if(bank < 0x40 && (addrHw & 0x8000)) {
		return (addrHw & 0x7FFF) | (bank << 15);
	}
	return INVALIDO;
}

uint32_t ConvertirAddrPcAHw(uint32_t addrPc) {
	if(addrPc < TAMANO_ROM) {
		uint32_t bank = (addrPc >> 15) & 0x7F;
		return (addrPc & 0x7FFF) | (bank << 16) | 0x8000;
	}
	return INVALIDO;
}

void GuardarROMArchivo(const char *nombreArchivo) {
	FILE *archivo = fopen(nombreArchivo, "wb");
	if(archivo) {
		fwrite(DROM, sizeof(uint8_t), TAMANO_ROM, archivo);
		fclose(archivo);
	}
}

void InicializarROM() {
	memset(DROM, 0, TAMANO_ROM);
}

// CRC32 IEEE 802.3 (polinomio 0xEDB88320), igual que WLA-DX / zlib.
// Los emuladores lo usan en [rom checksum] del .sym para verificar que el archivo
// de simbolos corresponde a esta ROM.
uint32_t CalcularCRC32ROM() {
	static uint32_t tabla[256];
	static bool inicializada = false;
	if(!inicializada) {
		for(uint32_t i = 0; i < 256; i++) {
			uint32_t c = i;
			for(int b = 0; b < 8; b++) {
				c = (c >> 1) ^ (0xEDB88320u & (uint32_t) - (int32_t)(c & 1));
			}
			tabla[i] = c;
		}
		inicializada = true;
	}

	uint32_t crc = 0xFFFFFFFFu;
	for(uint32_t i = 0; i < TAMANO_ROM; i++) {
		crc = tabla[(crc ^ DROM[i]) & 0xFF] ^ (crc >> 8);
	}
	return ~crc;
}