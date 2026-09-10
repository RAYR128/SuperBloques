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
	EscenaEjemplo.Bloques.resize(6);

	NodoBloque &EventoFrame = EscenaEjemplo.Bloques[0];
	NodoBloque &StoreVar1 = EscenaEjemplo.Bloques[1];
	NodoBloque &StoreVar2 = EscenaEjemplo.Bloques[2];
	NodoBloque &SetLayer1X = EscenaEjemplo.Bloques[3];
	NodoBloque &SetMosaico = EscenaEjemplo.Bloques[4];
	NodoBloque &SetBrillo = EscenaEjemplo.Bloques[5];

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
		NodoBloque GetLayer;
		GetLayer.TipoDeBloque = BLOQUE_ANIMACION_SCENE_LAYER1_POSITION_X;
		StoreVar2.Entradas.push_back(GetLayer);
	}

	SetLayer1X.TipoDeBloque = BLOQUE_ANIMACION_SCENE_SET_LAYER1_POSITION_X;
	{
		NodoBloque Lit16;
		Lit16.TipoDeBloque = BLOQUE_NUMERO;
		Lit16.ParametroEspecial = 16;
		SetLayer1X.Entradas.push_back(Lit16);
	}

	SetMosaico.TipoDeBloque = BLOQUE_ANIMACION_SCENE_SET_MOSAIC_FILTER;
	{
		NodoBloque Lit;
		Lit.TipoDeBloque = BLOQUE_NUMERO;
		Lit.ParametroEspecial = 0xF0;
		SetMosaico.Entradas.push_back(Lit);
	}

	SetBrillo.TipoDeBloque = BLOQUE_ANIMACION_SCENE_SET_BRIGHTNESS;
	{
		NodoBloque Lit;
		Lit.TipoDeBloque = BLOQUE_NUMERO;
		Lit.ParametroEspecial = 0xF0;
		SetBrillo.Entradas.push_back(Lit);
	}

	EventoFrame.Siguiente = &StoreVar1;
	StoreVar1.Previo = &EventoFrame;
	StoreVar1.Siguiente = &StoreVar2;
	StoreVar2.Previo = &StoreVar1;
	StoreVar2.Siguiente = &SetLayer1X;
	SetLayer1X.Previo = &StoreVar2;
	SetLayer1X.Siguiente = &SetMosaico;
	SetMosaico.Previo = &SetLayer1X;
	SetMosaico.Siguiente = &SetBrillo;
	SetBrillo.Previo = &SetMosaico;

	ObjetosProyecto.push_back(ObjetoEscena{});
	ObjetoEscena &ObjetoEjemplo = ObjetosProyecto.back();
	ObjetoEjemplo.Nombre = "Objeto1";
	ObjetoEjemplo.Bloques.resize(2);
	NodoBloque &ObjFrame = ObjetoEjemplo.Bloques[0];
	NodoBloque &SetSprite = ObjetoEjemplo.Bloques[1];
	ObjFrame.TipoDeBloque = BLOQUE_EVENTO_FRAME;
	SetSprite.TipoDeBloque = BLOQUE_ANIMACION_OBJ_SET_SPRITE;
	{
		NodoBloque Lit;
		Lit.TipoDeBloque = BLOQUE_NUMERO;
		Lit.ParametroEspecial = 3;
		SetSprite.Entradas.push_back(Lit);
	}
	ObjFrame.Siguiente = &SetSprite;
	SetSprite.Previo = &ObjFrame;

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
