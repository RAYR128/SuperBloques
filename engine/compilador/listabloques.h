// clang-format off
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
xx(CONTROL_IF, "control_if") // Entradas[0] -> Camino si es verdad
xx(CONTROL_WHILE, "control_while") // Entradas[0] -> Camino si es verdad, repetir
xx(CONTROL_IFELSE, "control_ifelse") // Entradas[0] -> Camino si es verdad, Entradas[1] -> Camino si es falso

// Inicio, labels
xx(EVENTO, "evento")
xx(EVENTO_INIT, "evento_init") // Siguiente
xx(EVENTO_FRAME, "evento_frame") // ParametroEspecial -> PARAMETRO_OBJ_BHV_SCRIPT_STATUS en el cual se ejecuta. Si es 0 es en todos los estados (no compila IF)

// Control y asignacion de variables
// VARIABLE es similar a NUMERO, y usa ParametroEspecial como un offset hacia "Variables" en ObjetoEscena/Escena
xx(VARIABLE, "variable")
xx(VARIABLE_STORE, "variable_store")

// Operaciones matematicas y logicas
xx(OPERACION, "operation")
xx(OPERACION_SUMA, "operation_add") // (Entradas[0] + Entradas[1])
xx(OPERACION_RESTA, "operation_sub") // (Entradas[0] - Entradas[1])
xx(OPERACION_MULTIPLICACION, "operation_mul") // (Entradas[0] * Entradas[1])
xx(OPERACION_DIVISION, "operation_div") // (Entradas[0] / Entradas[1])

#undef xx
// clang-format on