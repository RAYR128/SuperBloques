#include "ensamblador.h"
#include "rom.h"
#include "serializer/serializer.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

static void MostrarAyuda(const char *Programa) {
	std::cerr << "Uso: " << Programa << " -i <proyecto.json> [-o <salida.sfc>]" << std::endl;
	std::cerr << std::endl;
	std::cerr << "  -i <archivo>    Proyecto JSON a compilar" << std::endl;
	std::cerr << "  -o <archivo>    Ruta de salida de la ROM (por defecto: salida.sfc)" << std::endl;
	std::cerr << "  -h, --help      Muestra esta ayuda" << std::endl;
}

static std::string LeerArchivo(const std::string &Ruta) {
	std::ifstream Archivo(Ruta);
	if(!Archivo) {
		throw std::runtime_error("no se pudo abrir: " + Ruta);
	}
	std::ostringstream Contenido;
	Contenido << Archivo.rdbuf();
	return Contenido.str();
}

int main(int argc, char **argv) {
	const char *Programa = (argc > 0 && argv[0] != nullptr) ? argv[0] : "superbloques_cli";
	std::string RutaEntrada;
	std::string RutaSalida = "salida.sfc";

	for(int i = 1; i < argc; i++) {
		const std::string Arg = argv[i];
		if(Arg == "-h" || Arg == "--help") {
			MostrarAyuda(Programa);
			return 0;
		}
		if(Arg == "-i") {
			if(i + 1 >= argc) {
				std::cerr << "falta el archivo de entrada para -i" << std::endl;
				MostrarAyuda(Programa);
				return 1;
			}
			RutaEntrada = argv[++i];
			continue;
		}
		if(Arg == "-o") {
			if(i + 1 >= argc) {
				std::cerr << "falta el archivo de salida para -o" << std::endl;
				MostrarAyuda(Programa);
				return 1;
			}
			RutaSalida = argv[++i];
			continue;
		}
		std::cerr << "argumento desconocido: " << Arg << std::endl;
		MostrarAyuda(Programa);
		return 1;
	}

	if(RutaEntrada.empty()) {
		MostrarAyuda(Programa);
		return 1;
	}

	try {
		CargarProyectoDesdeString(LeerArchivo(RutaEntrada));
		InicializarROM();
		EnsamblarROM();
		GuardarROMArchivo(RutaSalida.c_str());
	} catch(const std::exception &ex) {
		std::cerr << ex.what() << std::endl;
		return 1;
	}
	return 0;
}
