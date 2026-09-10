#include "codegen.h"
#include <stdexcept>

static ContextoCompilacion contextoCompilacionActual = CTX_ESCENA;
static int compiladorUsoScratch = 0;

static void (*const kTablaEmit[BLOQUE_MAX])(NodoBloque *) = {
#define xx(n, s, c) Emit_##n,
#include "listabloques.h"
};

std::string EtiquetaLocal(const char *tag) {
	return std::string("ANM_") + tag + "_" + std::to_string(cc.ObtenerPC());
}

void EsperarEntradas(NodoBloque *blk, size_t entradas) {
	if(blk->Entradas.size() != entradas) {
		if(entradas) {
			throw std::runtime_error(ConvertirTipoDeBloqueAString(blk->TipoDeBloque) + " solo acepta " + std::to_string(entradas) + " entradas");
		} else {
			throw std::runtime_error(ConvertirTipoDeBloqueAString(blk->TipoDeBloque) + " no acepta entradas");
		}
	}
}

void EsperarContexto(NodoBloque *blk, ContextoCompilacion ctx) {
	if(ObtenerContextoCompilacion() == ctx) {
		return;
	}
	const char *donde = (ctx == CTX_OBJETO) ? "objeto" : "escena";
	throw std::runtime_error(ConvertirTipoDeBloqueAString(blk->TipoDeBloque) + " solo se puede usar en " + donde);
}

void SetContextoCompilacion(ContextoCompilacion ctx) {
	contextoCompilacionActual = ctx;
	compiladorUsoScratch = 0;
}

ContextoCompilacion ObtenerContextoCompilacion() {
	return contextoCompilacionActual;
}

uint16_t ScratchPush() {
	if(compiladorUsoScratch >= (WRAM_SCRATCH_SIZE / 2)) {
		throw std::runtime_error("expresion demasiado anidada");
	}
	uint16_t addr = (uint16_t)(WRAM_SCRATCH + compiladorUsoScratch * 2);
	compiladorUsoScratch++;
	return addr;
}

void ScratchPop() {
	if(compiladorUsoScratch <= 0) {
		throw std::runtime_error("scratch underflow");
	}
	compiladorUsoScratch--;
}

static int MaximoVariables() {
	return (contextoCompilacionActual == CTX_OBJETO) ? OBJETO_VARIABLES_MAX : ESCENA_VARIABLES_MAX;
}

static void ValidarIndiceVariable(int indice) {
	if(indice < 0 || indice >= MaximoVariables()) {
		throw std::runtime_error("indice de variable fuera de rango");
	}
}

void CargarVariableEnA(int indice) {
	ValidarIndiceVariable(indice);
	uint16_t off = (uint16_t)(indice * 2);
	if(contextoCompilacionActual == CTX_OBJETO) {
		cc.CargarRegEnMemoria_IndY(REG_A, WRAM_OBJETOS + PARAMETRO_OBJ_VARIABLES + off);
	} else {
		cc.CargarRegEnMemoria(REG_A, WRAM_ESCENA + off);
	}
}

void AlmacenarAEnVariable(int indice) {
	ValidarIndiceVariable(indice);
	uint16_t off = (uint16_t)(indice * 2);
	if(contextoCompilacionActual == CTX_OBJETO) {
		cc.AlmacenarRegEnMemoria_IndY(REG_A, WRAM_OBJETOS + PARAMETRO_OBJ_VARIABLES + off);
	} else {
		cc.AlmacenarRegEnMemoria(REG_A, WRAM_ESCENA + off);
	}
}

void CompilarExpresion(NodoBloque *n) {
	if(!n) {
		throw std::runtime_error("expresion vacia");
	}
	if(ObtenerClaseBloque(n->TipoDeBloque) != BLOQUE_CLASE_VALOR) {
		throw std::runtime_error("se esperaba un bloque de valor");
	}
	n->Compilar();
}

void CompilarCadenaAcciones(NodoBloque *primero) {
	compiladorUsoScratch = 0;
	for(NodoBloque *n = primero; n; n = n->Siguiente) {
		ClaseBloque clase = ObtenerClaseBloque(n->TipoDeBloque);
		if(clase == BLOQUE_CLASE_FINALIZADOR) {
			n->Compilar();
			break;
		}
		if(clase != BLOQUE_CLASE_ACCION) {
			throw std::runtime_error("la cadena de un evento solo puede contener acciones");
		}
		n->Compilar();
	}
}

void EmitirNoImplementado(NodoBloque *blk) {
	std::string nombre = blk ? ConvertirTipoDeBloqueAString(blk->TipoDeBloque) : "?";
	throw std::runtime_error("bloque no implementado: " + nombre);
}

void NodoBloque::Compilar() {
	if((unsigned)TipoDeBloque >= BLOQUE_MAX) {
		throw std::runtime_error("tipo de bloque invalido");
	}
	kTablaEmit[TipoDeBloque](this);
}