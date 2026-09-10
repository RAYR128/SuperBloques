// clang-format off
// Generacion utilizando macros para bloques.
#ifndef xx
#define xx(n,s,c)
#endif

// Tipo basico: Numero (ParametroEspecial)
xx(NUMERO, "numero", BLOQUE_CLASE_VALOR)

// Movimiento del objeto) administracion de posicion
xx(MOTION, "motion", BLOQUE_CLASE_CATEGORIA)
xx(MOTION_GET_POSICION_X, "motion_get_posicion_x", BLOQUE_CLASE_VALOR)
xx(MOTION_GET_POSICION_Y, "motion_get_posicion_y", BLOQUE_CLASE_VALOR)
xx(MOTION_SET_POSICION_X, "motion_set_posicion_x", BLOQUE_CLASE_ACCION) // PARAMETRO_OBJ_POSICION_X = Entradas[0]
xx(MOTION_SET_POSICION_Y, "motion_set_posicion_y", BLOQUE_CLASE_ACCION) // PARAMETRO_OBJ_POSICION_Y = Entradas[0]
xx(MOTION_ADD_POSICION_X, "motion_add_posicion_x", BLOQUE_CLASE_ACCION) // PARAMETRO_OBJ_POSICION_X += Entradas[0]
xx(MOTION_ADD_POSICION_Y, "motion_add_posicion_y", BLOQUE_CLASE_ACCION) // PARAMETRO_OBJ_POSICION_Y += Entradas[0]

// Reproduccion de animaciones) control de frames y sprites
xx(ANIMACION, "animation", BLOQUE_CLASE_CATEGORIA)
xx(ANIMACION_OBJ_SET_SPRITE, "animation_obj_set_sprite", BLOQUE_CLASE_ACCION) // PARAMETRO_OBJ_SPRITE = Entradas[0]
xx(ANIMACION_SCENE_LAYER1_POSITION_X, "animacion_scene_layer1_position_x", BLOQUE_CLASE_VALOR)
xx(ANIMACION_SCENE_LAYER1_POSITION_Y, "animacion_scene_layer1_position_y", BLOQUE_CLASE_VALOR)
xx(ANIMACION_SCENE_LAYER2_POSITION_X, "animacion_scene_layer2_position_x", BLOQUE_CLASE_VALOR)
xx(ANIMACION_SCENE_LAYER2_POSITION_Y, "animacion_scene_layer2_position_y", BLOQUE_CLASE_VALOR)
xx(ANIMACION_SCENE_LAYER3_POSITION_X, "animacion_scene_layer3_position_x", BLOQUE_CLASE_VALOR)
xx(ANIMACION_SCENE_LAYER3_POSITION_Y, "animacion_scene_layer3_position_y", BLOQUE_CLASE_VALOR)
xx(ANIMACION_SCENE_SET_LAYER1_POSITION_X, "animacion_scene_set_layer1_position_x", BLOQUE_CLASE_ACCION)
xx(ANIMACION_SCENE_SET_LAYER1_POSITION_Y, "animacion_scene_set_layer1_position_y", BLOQUE_CLASE_ACCION)
xx(ANIMACION_SCENE_SET_LAYER2_POSITION_X, "animacion_scene_set_layer2_position_x", BLOQUE_CLASE_ACCION)
xx(ANIMACION_SCENE_SET_LAYER2_POSITION_Y, "animacion_scene_set_layer2_position_y", BLOQUE_CLASE_ACCION)
xx(ANIMACION_SCENE_SET_LAYER3_POSITION_X, "animacion_scene_set_layer3_position_x", BLOQUE_CLASE_ACCION)
xx(ANIMACION_SCENE_SET_LAYER3_POSITION_Y, "animacion_scene_set_layer3_position_y", BLOQUE_CLASE_ACCION)
xx(ANIMACION_SCENE_SET_MOSAIC_FILTER, "animacion_scene_set_mosaic_filter", BLOQUE_CLASE_ACCION)
xx(ANIMACION_SCENE_SET_BRIGHTNESS, "animacion_scene_set_brightness", BLOQUE_CLASE_ACCION)

// Reproduccion de sonidos y musica
xx(SONIDO, "sound", BLOQUE_CLASE_CATEGORIA)

// Condicionales y bucles
xx(CONTROL, "control", BLOQUE_CLASE_CATEGORIA)
xx(CONTROL_IF, "control_if", BLOQUE_CLASE_ACCION) // Entradas[0] -> condicion; Cuerpo -> pila si verdad
xx(CONTROL_WHILE, "control_while", BLOQUE_CLASE_ACCION) // Entradas[0] -> condicion; Cuerpo -> pila del cuerpo
xx(CONTROL_IFELSE, "control_ifelse", BLOQUE_CLASE_ACCION) // Entradas[0] -> condicion; Cuerpo / CuerpoSino -> pilas

// Inicio, labels
xx(EVENTO, "evento", BLOQUE_CLASE_CATEGORIA)
xx(EVENTO_INIT, "evento_init", BLOQUE_CLASE_EVENTO) // Siguiente
xx(EVENTO_FRAME, "evento_frame", BLOQUE_CLASE_EVENTO) // ParametroEspecial -> PARAMETRO_OBJ_BHV_SCRIPT_STATUS en el cual se ejecuta. Si es 0 es en todos los estados (no compila IF)

// Control y asignacion de variables
// VARIABLE es similar a NUMERO, y usa ParametroEspecial como un offset hacia "Variables" en ObjetoEscena/Escena
xx(VARIABLE, "variable", BLOQUE_CLASE_VALOR)
xx(VARIABLE_STORE, "variable_store", BLOQUE_CLASE_ACCION)

// Operaciones matematicas y logicas
xx(OPERACION, "operation", BLOQUE_CLASE_CATEGORIA)
xx(OPERACION_SUMA, "operation_add", BLOQUE_CLASE_VALOR) // (Entradas[0] + Entradas[1])
xx(OPERACION_RESTA, "operation_sub", BLOQUE_CLASE_VALOR) // (Entradas[0] - Entradas[1])
xx(OPERACION_MULTIPLICACION, "operation_mul", BLOQUE_CLASE_VALOR) // (Entradas[0] * Entradas[1])
xx(OPERACION_DIVISION, "operation_div", BLOQUE_CLASE_VALOR) // (Entradas[0] / Entradas[1])

// Lectura de sensores de hardware (timer global y botones)
xx(SENSOR, "sensor", BLOQUE_CLASE_CATEGORIA)
xx(SENSOR_TIEMPO, "sensor_tiempo", BLOQUE_CLASE_VALOR) // Devuelve WRAM_TIMER
xx(SENSOR_BOTON_CONTROL_1, "sensor_boton_control_1", BLOQUE_CLASE_VALOR) // ParametroEspecial -> BotonControl (bit 0-15 de WRAM_CONTROL1)
xx(SENSOR_BOTON_CONTROL_2, "sensor_boton_control_2", BLOQUE_CLASE_VALOR) // ParametroEspecial -> BotonControl (bit 0-15 de WRAM_CONTROL2)
xx(SENSOR_BOTON_CONTROL_1_PRESIONADO, "sensor_boton_control_1_presionado", BLOQUE_CLASE_VALOR) // ParametroEspecial -> BotonControl (bit 0-15 de WRAM_CONTROL1_PRESIONADO)
xx(SENSOR_BOTON_CONTROL_2_PRESIONADO, "sensor_boton_control_2_presionado", BLOQUE_CLASE_VALOR) // ParametroEspecial -> BotonControl (bit 0-15 de WRAM_CONTROL2_PRESIONADO)

#undef xx
																								   // clang-format on
