#include <iostream>
#include "rom.h"
#include "ensamblador.h"
#include "instancia.h"

int main() {
	EscenasProyecto.clear();
	ObjetosProyecto.clear();

	Escena EscenaEjemplo;
	EscenaEjemplo.Nombre = "Escena1";
	EscenasProyecto.push_back(EscenaEjemplo);

	// prueba CLI simple por ahora
	InicializarROM();
	EnsamblarROM();
	GuardarROMArchivo("salida.sfc");
	return 0;
}