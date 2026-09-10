#include "datos.h"
#include "asm/op.h"
#include <algorithm>
#include <stdexcept>

void EmpaquetarDatosROM(std::vector<BloqueDato> bloques) {
	for(const BloqueDato &bloque : bloques) {
		if(bloque.tamano == 0 || bloque.tamano > TAMANO_BANCO_DATOS) {
			throw std::runtime_error("EmpaquetarDatosROM: tamano invalido para " + bloque.etiqueta);
		}
		if(bloque.datos == nullptr) {
			throw std::runtime_error("EmpaquetarDatosROM: datos nulos para " + bloque.etiqueta);
		}
	}

	std::stable_sort(bloques.begin(), bloques.end(), [](const BloqueDato &a, const BloqueDato &b) {
		return a.tamano > b.tamano;
	});

	uint32_t usado[CANTIDAD_BANCOS_DATOS] = {};
	for(const BloqueDato &bloque : bloques) {
		int banco = -1;
		for(int i = 0; i < CANTIDAD_BANCOS_DATOS; i++) {
			if(TAMANO_BANCO_DATOS - usado[i] >= bloque.tamano) {
				banco = i;
				break;
			}
		}
		if(banco < 0) {
			throw std::runtime_error("no hay espacio en bancos $10-$3F para " + bloque.etiqueta);
		}

		uint32_t pc = ((uint32_t)(BANCO_DATOS_PRIMERO + banco) << 15) + usado[banco];
		cc.SetearPC(pc);
		cc.Etiqueta(bloque.etiqueta);
		cc.EscribirBytes(bloque.datos, bloque.tamano);
		usado[banco] += bloque.tamano;
	}
}
