#include "codegen.h"
#include <stdexcept>

void Emit_NUMERO(NodoBloque *blk) {
	if(!blk->Entradas.empty()) {
		throw std::runtime_error("NUMERO no acepta entradas");
	}
	cc.CargarRegConst16(REG_A, (uint16_t)blk->ParametroEspecial);
}

void Emit_VARIABLE(NodoBloque *blk) {
	if(!blk->Entradas.empty()) {
		throw std::runtime_error("VARIABLE no acepta entradas");
	}
	CargarVariableEnA(blk->ParametroEspecial);
}

void Emit_VARIABLE_STORE(NodoBloque *blk) {
	if(blk->Entradas.size() != 1) {
		throw std::runtime_error("VARIABLE_STORE requiere 1 entrada");
	}
	CompilarExpresion(&blk->Entradas[0]);
	AlmacenarAEnVariable(blk->ParametroEspecial);
}