#include "codegen.h"

void Emit_CONTROL(NodoBloque *blk) {
	EmitirNoImplementado(blk);
}

static void EmitirCondicionYSaltarSiFalso(NodoBloque *blk, const std::string &label) {
	EsperarEntradas(blk, 1);
	CompilarExpresion(&blk->Entradas[0]);
	cc.CompararRegConst16(REG_A, 0);
	cc.BranchLong(label, BRANCH_ZERO_SET);
}

void Emit_CONTROL_IF(NodoBloque *blk) {
	std::string fin = EtiquetaLocal("IF");
	EmitirCondicionYSaltarSiFalso(blk, fin);
	CompilarCadenaAcciones(blk->Cuerpo);
	cc.Etiqueta(fin);
}

void Emit_CONTROL_WHILE(NodoBloque *blk) {
	std::string inicio = EtiquetaLocal("WHL");
	cc.Etiqueta(inicio);
	std::string fin = EtiquetaLocal("WHLEND");
	EmitirCondicionYSaltarSiFalso(blk, fin);
	CompilarCadenaAcciones(blk->Cuerpo);
	cc.Saltar(inicio, REF_ABSOLUTE);
	cc.Etiqueta(fin);
}

void Emit_CONTROL_IFELSE(NodoBloque *blk) {
	std::string sino = EtiquetaLocal("IFE");
	std::string fin = EtiquetaLocal("IFEEND");
	EmitirCondicionYSaltarSiFalso(blk, sino);
	CompilarCadenaAcciones(blk->Cuerpo);
	cc.Saltar(fin, REF_ABSOLUTE);
	cc.Etiqueta(sino);
	CompilarCadenaAcciones(blk->CuerpoSino);
	cc.Etiqueta(fin);
}
