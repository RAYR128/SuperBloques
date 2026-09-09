#pragma once

#include <string>
#include <vector>

// Tipos de bloques que pueden existir en un objeto de la escena. Cada bloque tiene un comportamiento distinto y puede contener distintos datos.
enum TipoBloque {
	#define xx(n,s) BLOQUE_##n,
	#include "listabloques.h"
	BLOQUE_MAX
};

// Serializer
TipoBloque ConvertirStringATipoDeBloque(std::string Entrada);
std::string ConvertirTipoDeBloqueAString(TipoBloque Entrada);

// Los bloques actuan como un arbol AST (Abstract Syntax Tree) que representa la logica de un objeto en la escena.
// Cada bloque puede contener otros bloques como hijos, formando una estructura jerarquica que define el comportamiento del objeto.
// Los bloques son compilados a scripts de behavior que son ejecutados por la CPU.
class NodoBloque {
  public:
	NodoBloque()
		: TipoDeBloque(BLOQUE_NUMERO), ParametroEspecial(0), PosicionVisualX(0), PosicionVisualY(0), Siguiente(nullptr),
		  Previo(nullptr) {}
	~NodoBloque() {}

	// Compilar un bloque y todos sus sub-bloques.
	void Compilar();

	// Tipo de bloque
	TipoBloque TipoDeBloque;

	// Entradas
	std::vector<NodoBloque> Entradas;
	int ParametroEspecial;

	// Usado en el frontend, para organizacion.
	// Solo el bloque mas superior (control, inicio de evento) respeta o usa estos valores. Los sub-bloques de un bloque no lo utilizan.
	int PosicionVisualX, PosicionVisualY;
	
	// Estructura
	NodoBloque *Siguiente;
	NodoBloque *Previo;
};