#include "codegen.h"
#include "instancia.h"
#include <stdexcept>

void Emit_EVENTO(NodoBloque *blk) {
	EmitirNoImplementado(blk);
}

static void EmitirEvento(NodoBloque *blk) {
	EsperarEntradas(blk, 0);
	CompilarCadenaAcciones(blk->Siguiente);
}

void Emit_EVENTO_INIT(NodoBloque *blk) {
	EmitirEvento(blk);
}

void Emit_EVENTO_FRAME(NodoBloque *blk) {
	EmitirEvento(blk);
}

static void CargarEstadoEnA() {
	if(ObtenerContextoCompilacion() == CTX_OBJETO) {
		cc.CargarRegEnMemoria_IndY(REG_A, WRAM_OBJETOS + PARAMETRO_OBJ_BHV_SCRIPT_STATUS);
	} else {
		cc.CargarRegEnMemoria(REG_A, WRAM_ESCENA_STATUS);
	}
}

static void AlmacenarAEnEstado8() {
	cc.SetearFlags(FLAG_M_8BIT);
	if(ObtenerContextoCompilacion() == CTX_OBJETO) {
		cc.AlmacenarRegEnMemoria_IndY(REG_A, WRAM_OBJETOS + PARAMETRO_OBJ_BHV_SCRIPT_STATUS);
	} else {
		cc.AlmacenarRegEnMemoria(REG_A, WRAM_ESCENA_STATUS);
	}
	cc.LimpiarFlags(FLAG_M_8BIT);
}

void Emit_EVENTO_ESTADO(NodoBloque *blk) {
	EsperarEntradas(blk, 0);
	CargarEstadoEnA();
	cc.ANDAcumuladorConst16(0x00FF);
}

void Emit_EVENTO_SET_ESTADO(NodoBloque *blk) {
	EsperarEntradas(blk, 1);
	CompilarExpresion(&blk->Entradas[0]);
	AlmacenarAEnEstado8();
}

void Emit_EVENTO_CAMBIAR_ESCENA(NodoBloque *blk) {
	EsperarEntradas(blk, 0);
	int id = blk->ParametroEspecial;
	if(id < 0 || (size_t)id >= EscenasProyecto.size() || id > 255) {
		throw std::runtime_error("evento_cambiar_escena: escena inexistente");
	}
	cc.SetearFlags(FLAG_M_8BIT);
	cc.CargarRegConst8(REG_A, (uint8_t)id);
	cc.AlmacenarRegEnMemoria(REG_A, WRAM_ESCENA_ACTUAL);
	cc.LimpiarFlags(FLAG_M_8BIT);
	cc.LlamadaLong("INICIALIZAR_ESCENA_ID");
	cc.ReturnLong();
}
