#include "compilar.h"
#include "rom.h"
#include <emscripten.h>
#include <exception>
#include <string>

static std::string gError;

extern "C" {

EMSCRIPTEN_KEEPALIVE
int sb_compilar(const char *json, int longitud) {
	try {
		gError.clear();
		if(json == nullptr || longitud < 0) {
			gError = "json invalido";
			return 1;
		}
		CompilarProyectoEnMemoria(std::string(json, json + longitud));
		return 0;
	} catch(const std::exception &ex) {
		gError = ex.what();
		return 1;
	} catch(...) {
		gError = "error desconocido";
		return 1;
	}
}

EMSCRIPTEN_KEEPALIVE
const uint8_t *sb_rom() {
	return DROM;
}

EMSCRIPTEN_KEEPALIVE
int sb_rom_tamano() {
	return (int)TAMANO_ROM;
}

EMSCRIPTEN_KEEPALIVE
const char *sb_ultimo_error() {
	return gError.c_str();
}
}
