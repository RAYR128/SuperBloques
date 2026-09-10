#include "codegen.h"
#include <stdexcept>
#include <string>

void Emit_SENSOR(NodoBloque *blk) {
	EmitirNoImplementado(blk);
}

void Emit_SENSOR_TIEMPO(NodoBloque *blk) {
	if(!blk->Entradas.empty()) {
		throw std::runtime_error("SENSOR_TIEMPO no acepta entradas");
	}
	cc.CargarRegEnMemoria(REG_A, WRAM_TIMER);
}

static void EmitirBotonControl(NodoBloque *blk, uint16_t addrControl, const char *nombre) {
	if(!blk->Entradas.empty()) {
		throw std::runtime_error(std::string(nombre) + " no acepta entradas");
	}
	int bit = blk->ParametroEspecial;
	if(bit < BOTON_FIRMA_0 || bit > BOTON_B) {
		throw std::runtime_error(std::string(nombre) + " requiere ParametroEspecial en 0-15");
	}
	cc.CargarRegEnMemoria(REG_A, addrControl);
	cc.ShiftARight(bit);
	cc.ANDAcumuladorConst16(1);
}

void Emit_SENSOR_BOTON_CONTROL_1(NodoBloque *blk) {
	EmitirBotonControl(blk, WRAM_CONTROL1, "SENSOR_BOTON_CONTROL_1");
}

void Emit_SENSOR_BOTON_CONTROL_2(NodoBloque *blk) {
	EmitirBotonControl(blk, WRAM_CONTROL2, "SENSOR_BOTON_CONTROL_2");
}
