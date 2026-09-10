#include "codegen.h"
#include <stdexcept>

void Emit_NUMERO(NodoBloque *blk) {
	EsperarEntradas(blk, 0);
	cc.CargarRegConst16(REG_A, (uint16_t)blk->ParametroEspecial);
}

void Emit_VARIABLE(NodoBloque *blk) {
	EsperarEntradas(blk, 0);
	CargarVariableEnA(blk->ParametroEspecial);
}

void Emit_VARIABLE_STORE(NodoBloque *blk) {
	EsperarEntradas(blk, 1);
	CompilarExpresion(&blk->Entradas[0]);
	AlmacenarAEnVariable(blk->ParametroEspecial);
}