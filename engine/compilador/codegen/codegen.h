#pragma once

#include "asm/op.h"
#include "bloques.h"

enum ContextoCompilacion {
	CTX_ESCENA,
	CTX_OBJETO
};

void SetContextoCompilacion(ContextoCompilacion ctx);
ContextoCompilacion ObtenerContextoCompilacion();

uint16_t ScratchPush();
void ScratchPop();

void CompilarExpresion(NodoBloque *n);
void CompilarCadenaAcciones(NodoBloque *primero);
void CargarVariableEnA(int indice);
void AlmacenarAEnVariable(int indice);
void EmitirNoImplementado(NodoBloque *blk);
void EsperarEntradas(NodoBloque *blk, size_t entradas);

#define xx(n, s, c) extern void Emit_##n(NodoBloque *blk);
#include "listabloques.h"