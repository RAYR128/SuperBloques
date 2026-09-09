#include "bloques.h"

struct MapaTipoBloque {
	TipoBloque Tipo;
	const char *Nombre;
};

static const MapaTipoBloque kMapaTipos[] = {
	#define xx(n,s) {BLOQUE_##n,s},
	#include "listabloques.h"
};

TipoBloque ConvertirStringATipoDeBloque(std::string Entrada) {
	for(const auto &m : kMapaTipos) {
		if(Entrada == m.Nombre) {
			return m.Tipo;
		}
	}
	return BLOQUE_MAX;
}

std::string ConvertirTipoDeBloqueAString(TipoBloque Entrada) {
	for(const auto &m : kMapaTipos) {
		if(Entrada == m.Tipo) {
			return m.Nombre;
		}
	}
	return "";
}

void NodoBloque::Compilar() {
	
}

bool NodoBloque::EsInicioEvento() {
	return TipoDeBloque == BLOQUE_EVENTO_FRAME || TipoDeBloque == BLOQUE_EVENTO_INIT;
}