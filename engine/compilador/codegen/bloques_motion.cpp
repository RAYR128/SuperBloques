#include "codegen.h"

void Emit_MOTION(NodoBloque *blk) {
	EmitirNoImplementado(blk);
}

void Emit_MOTION_GET_POSICION_X(NodoBloque *blk) {
	EsperarEntradas(blk, 0);
	cc.CargarRegEnMemoria_IndY(REG_A, WRAM_OBJETOS + PARAMETRO_OBJ_POSICION_X);
}

void Emit_MOTION_GET_POSICION_Y(NodoBloque *blk) {
	EsperarEntradas(blk, 0);
	cc.CargarRegEnMemoria_IndY(REG_A, WRAM_OBJETOS + PARAMETRO_OBJ_POSICION_Y);
}

void Emit_MOTION_SET_POSICION_X(NodoBloque *blk) {
	EmitirNoImplementado(blk);
}

void Emit_MOTION_SET_POSICION_Y(NodoBloque *blk) {
	EmitirNoImplementado(blk);
}

void Emit_MOTION_ADD_POSICION_X(NodoBloque *blk) {
	EmitirNoImplementado(blk);
}

void Emit_MOTION_ADD_POSICION_Y(NodoBloque *blk) {
	EmitirNoImplementado(blk);
}