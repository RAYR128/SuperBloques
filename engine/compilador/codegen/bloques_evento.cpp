#include "codegen.h"
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