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
xx(VARIABLE, "variable")

// Operaciones matematicas y logicas
xx(OPERACION, "operation")
xx(OPERACION_SUMA, "operation_add")

#undef xx