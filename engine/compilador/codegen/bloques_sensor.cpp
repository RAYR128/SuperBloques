#include "codegen.h"
#include <stdexcept>
#include <string>

void Emit_SENSOR(NodoBloque *blk) {
	EmitirNoImplementado(blk);
}

void Emit_SENSOR_TIEMPO(NodoBloque *blk) {
	EsperarEntradas(blk, 0);
	cc.CargarRegEnMemoria(REG_A, WRAM_TIMER);
}

static void EmitirBotonControl(NodoBloque *blk, uint16_t addrControl, const char *nombre) {
	EsperarEntradas(blk, 0);
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

void Emit_SENSOR_BOTON_CONTROL_1_PRESIONADO(NodoBloque *blk) {
	EmitirBotonControl(blk, WRAM_CONTROL1_PRESIONADO, "SENSOR_BOTON_CONTROL_1");
}

void Emit_SENSOR_BOTON_CONTROL_2_PRESIONADO(NodoBloque *blk) {
	EmitirBotonControl(blk, WRAM_CONTROL2_PRESIONADO, "SENSOR_BOTON_CONTROL_2");
}