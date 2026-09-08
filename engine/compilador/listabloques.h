// Generacion utilizando macros para bloques.
#ifndef xx
#define xx(n,s)
#endif

// Tipo basico: Numero (ParametroEspecial)
xx(NUMERO, "numero")

// Movimiento del objeto) administracion de posicion
xx(MOTION, "motion")
xx(MOTION_GET_POSICION_X, "motion_get_posicion_x")
xx(MOTION_GET_POSICION_Y, "motion_get_posicion_y")
xx(MOTION_SET_POSICION_X, "motion_set_posicion_x")
xx(MOTION_SET_POSICION_Y, "motion_set_posicion_y")
xx(MOTION_ADD_POSICION_X, "motion_add_posicion_x")
xx(MOTION_ADD_POSICION_Y, "motion_add_posicion_y")

// Reproduccion de animaciones) control de frames y sprites
xx(ANIMACION, "animation")

// Reproduccion de sonidos y musica
xx(SONIDO, "sound")

// Condicionales y bucles
xx(CONTROL, "control")

// Inicio) labels
xx(EVENTO, "event")

// Control y asignacion de variables
// VARIABLE usa ParametroEspecial como un offset hacia "Variables" en ObjetoEscena/Escena
xx(VARIABLE, "variable")
xx(VARIABLE_STORE, "variable_store")

// Operaciones matematicas y logicas
xx(OPERACION, "operation")
xx(OPERACION_SUMA, "operation_add") // (Bloques[0] + Bloques[1])
xx(OPERACION_RESTA, "operation_sub") // (Bloques[0] - Bloques[1])
xx(OPERACION_MULTIPLICACION, "operation_mul") // (Bloques[0] * Bloques[1])
xx(OPERACION_DIVISION, "operation_div") // (Bloques[0] / Bloques[1])

#undef xx