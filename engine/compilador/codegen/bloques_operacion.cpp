#include "codegen.h"
#include <stdexcept>

void Emit_OPERACION(NodoBloque *blk) {
	(void)blk;
}

static void EmitirBinaria(NodoBloque *blk, bool resta) {
	if(blk->Entradas.size() != 2) {
		throw std::runtime_error(resta ? "OPERACION_RESTA requiere 2 entradas" : "OPERACION_SUMA requiere 2 entradas");
	}

	NodoBloque *izq = &blk->Entradas[0];
	NodoBloque *der = &blk->Entradas[1];

	auto aplicarInmediato = [&](uint16_t imm) {
		if(resta) {
			cc.SetearFlags(FLAG_CARRYF);
			cc.RestaAcumuladorConst16(imm);
		} else {
			cc.LimpiarFlags(FLAG_CARRYF);
			cc.SumaAcumuladorConst16(imm);
		}
	};
	auto aplicarMemoria = [&](uint16_t addr) {
		if(resta) {
			cc.SetearFlags(FLAG_CARRYF);
			cc.RestaAcumuladorMemoria(addr);
		} else {
			cc.LimpiarFlags(FLAG_CARRYF);
			cc.SumaAcumuladorMemoria(addr);
		}
	};

	if(der->TipoDeBloque == BLOQUE_NUMERO) {
		if(izq->TipoDeBloque == BLOQUE_NUMERO) {
			cc.CargarRegConst16(REG_A, (uint16_t)izq->ParametroEspecial);
		} else {
			CompilarExpresion(izq);
		}
		aplicarInmediato((uint16_t)der->ParametroEspecial);
		return;
	}

	if(!resta && izq->TipoDeBloque == BLOQUE_NUMERO) {
		CompilarExpresion(der);
		aplicarInmediato((uint16_t)izq->ParametroEspecial);
		return;
	}

	if(resta) {
		CompilarExpresion(der);
		uint16_t slot = ScratchPush();
		cc.AlmacenarRegEnMemoria(REG_A, slot);
		if(izq->TipoDeBloque == BLOQUE_NUMERO) {
			cc.CargarRegConst16(REG_A, (uint16_t)izq->ParametroEspecial);
		} else {
			CompilarExpresion(izq);
		}
		aplicarMemoria(slot);
		ScratchPop();
		return;
	}

	CompilarExpresion(izq);
	uint16_t slot = ScratchPush();
	cc.AlmacenarRegEnMemoria(REG_A, slot);
	CompilarExpresion(der);
	aplicarMemoria(slot);
	ScratchPop();
}

void Emit_OPERACION_SUMA(NodoBloque *blk) {
	EmitirBinaria(blk, false);
}

void Emit_OPERACION_RESTA(NodoBloque *blk) {
	EmitirBinaria(blk, true);
}

void Emit_OPERACION_MULTIPLICACION(NodoBloque *blk) {
	if(blk->Entradas.size() != 2) {
		throw std::runtime_error("OPERACION_MULTIPLICACION requiere 2 entradas");
	}
	EmitirNoImplementado(blk);
}

void Emit_OPERACION_DIVISION(NodoBloque *blk) {
	if(blk->Entradas.size() != 2) {
		throw std::runtime_error("OPERACION_DIVISION requiere 2 entradas");
	}
	EmitirNoImplementado(blk);
}