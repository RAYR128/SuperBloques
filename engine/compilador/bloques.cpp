#include "bloques.h"

struct MapaTipoBloque {
	TipoBloque Tipo;
	const char *Nombre;
};

static const MapaTipoBloque kMapaTipos[] = {
	#define xx(n,s,c) {BLOQUE_##n,s},
	#include "listabloques.h"
};

static const ClaseBloque kClasesBloque[] = {
	#define xx(n,s,c) c,
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

ClaseBloque ObtenerClaseBloque(TipoBloque Entrada) {
	if((unsigned)Entrada >= BLOQUE_MAX) {
		return BLOQUE_CLASE_CATEGORIA;
	}
	return kClasesBloque[Entrada];
}