#include "compilar.h"
#include "asm/op.h"
#include "ensamblador.h"
#include "rom.h"
#include "serializer/serializer.h"

void CompilarProyectoEnMemoria(const std::string &json) {
	cc.Reiniciar();
	CargarProyectoDesdeString(json);
	InicializarROM();
	EnsamblarROM();
}
