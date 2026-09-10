#include <iostream>
#include <stdexcept>
#include "rom.h"
#include "ensamblador.h"
#include "instancia.h"

int main() {
	EscenasProyecto.clear();
	ObjetosProyecto.clear();

	EscenasProyecto.push_back(Escena{});
	Escena &EscenaEjemplo = EscenasProyecto.back();
	EscenaEjemplo.Nombre = "Escena1";
	EscenaEjemplo.Variables = {"Variable 1", "Variable 2"};
	EscenaEjemplo.Bloques.resize(3);

	NodoBloque &EventoFrame = EscenaEjemplo.Bloques[0];
	NodoBloque &StoreVar1 = EscenaEjemplo.Bloques[1];
	NodoBloque &StoreVar2 = EscenaEjemplo.Bloques[2];

	EventoFrame.TipoDeBloque = BLOQUE_EVENTO_FRAME;

	StoreVar1.TipoDeBloque = BLOQUE_VARIABLE_STORE;
	StoreVar1.ParametroEspecial = 0;
	{
		NodoBloque Suma;
		Suma.TipoDeBloque = BLOQUE_OPERACION_SUMA;
		NodoBloque Lit4;
		Lit4.TipoDeBloque = BLOQUE_NUMERO;
		Lit4.ParametroEspecial = 4;
		NodoBloque Lit12;
		Lit12.TipoDeBloque = BLOQUE_NUMERO;
		Lit12.ParametroEspecial = 12;
		Suma.Entradas.push_back(Lit4);
		Suma.Entradas.push_back(Lit12);
		StoreVar1.Entradas.push_back(Suma);
	}

	StoreVar2.TipoDeBloque = BLOQUE_VARIABLE_STORE;
	StoreVar2.ParametroEspecial = 1;
	{
		NodoBloque Lit9;
		Lit9.TipoDeBloque = BLOQUE_NUMERO;
		Lit9.ParametroEspecial = 9;
		StoreVar2.Entradas.push_back(Lit9);
	}

	EventoFrame.Siguiente = &StoreVar1;
	StoreVar1.Previo = &EventoFrame;
	StoreVar1.Siguiente = &StoreVar2;
	StoreVar2.Previo = &StoreVar1;

	try {
		InicializarROM();
		EnsamblarROM();
		GuardarROMArchivo("salida.sfc");
	} catch(const std::exception &ex) {
		std::cerr << ex.what() << std::endl;
		return 1;
	}
	return 0;
}
