#pragma once

// Tipos de bloques que pueden existir en un objeto de la escena. Cada bloque tiene un comportamiento distinto y puede contener distintos datos.
enum TipoBloque {
	BLOQUE_MOTION,	  // Movimiento del objeto, administracion de posicion
	BLOQUE_ANIMACION, // Reproduccion de animaciones, control de frames y sprites
	BLOQUE_SONIDO,	  // Reproduccion de sonidos y musica
	BLOQUE_CONTROL,	  // Condicionales y bucles
	BLOQUE_EVENTO,	  // Inicio, labels
	BLOQUE_VARIABLE,  // Control y asignacion de variables
	BLOQUE_OPERACION, // Operaciones matematicas y logicas
	BLOQUE_MAX
};

enum BloquesMotion {
	B_MOTION_GET_POSICION_X,
	B_MOTION_GET_POSICION_Y,
	B_MOTION_SET_POSICION_X,
	B_MOTION_SET_POSICION_Y,
	B_MOTION_ADD_POSICION_X,
	B_MOTION_ADD_POSICION_Y
};

// Los bloques actuan como un arbol AST (Abstract Syntax Tree) que representa la logica de un objeto en la escena.
// Cada bloque puede contener otros bloques como hijos, formando una estructura jerarquica que define el comportamiento del objeto.
// Los bloques son compilados a scripts de behavior que son ejecutados por la CPU.
class NodoBloque {
  public:
	virtual ~NodoBloque() {}
	virtual void Compilar() = 0;
};