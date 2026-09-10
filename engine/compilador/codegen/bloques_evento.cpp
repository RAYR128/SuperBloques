#include "codegen.h"
#include <stdexcept>

void Emit_EVENTO(NodoBloque *blk) {
	EmitirNoImplementado(blk);
}

static void EmitirEvento(NodoBloque *blk) {
	if(!blk->Entradas.empty()) {
		throw std::runtime_error("un evento no acepta entradas");
	}
	CompilarCadenaAcciones(blk->Siguiente);
}

void Emit_EVENTO_INIT(NodoBloque *blk) {
	EmitirEvento(blk);
}

void Emit_EVENTO_FRAME(NodoBloque *blk) {
	EmitirEvento(blk);
}