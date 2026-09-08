#pragma once

#include <string>
#include <vector>

// Tipos de bloques que pueden existir en un objeto de la escena. Cada bloque tiene un comportamiento distinto y puede contener distintos datos.
enum TipoBloque {
	// Tipo basico: Numero (ParametroEspecial)
	BLOQUE_NUMERO,

	// Movimiento del objeto, administracion de posicion
	BLOQUE_MOTION,
	BLOQUE_MOTION_GET_POSICION_X,
	BLOQUE_MOTION_GET_POSICION_Y,
	BLOQUE_MOTION_SET_POSICION_X,
	BLOQUE_MOTION_SET_POSICION_Y,
	BLOQUE_MOTION_ADD_POSICION_X,
	BLOQUE_MOTION_ADD_POSICION_Y,

	// Reproduccion de animaciones, control de frames y sprites
	BLOQUE_ANIMACION,

	// Reproduccion de sonidos y musica
	BLOQUE_SONIDO,

	// Condicionales y bucles
	BLOQUE_CONTROL,

	// Inicio, labels
	BLOQUE_EVENTO,

	// Control y asignacion de variables
	BLOQUE_VARIABLE,

	// Operaciones matematicas y logicas
	BLOQUE_OPERACION,
	BLOQUE_OPERACION_SUMA,

	// Final
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
	~NodoBloque() {}
	void Compilar();

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