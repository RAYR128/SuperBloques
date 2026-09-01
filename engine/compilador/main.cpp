#include <iostream>
#include "rom.h"
#include "asm/ensamblador.h"

int main() {
	// prueba CLI simple por ahora
	InicializarROM();
	EnsamblarROM();
	GuardarROMArchivo("salida.sfc");
	return 1;
}