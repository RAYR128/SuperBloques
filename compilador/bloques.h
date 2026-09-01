#pragma once

// Tipos de bloques que pueden existir en un objeto de la escena. Cada bloque tiene un comportamiento distinto y puede contener distintos datos.
enum {
    BLOQUE_MOTION,
    BLOQUE_ANIMACION,
    BLOQUE_SONIDO,
    BLOQUE_EVENTO,
    BLOQUE_VARIABLE,
    BLOQUE_OPERACION,
    BLOQUE_MAX
};

class NodoBloque {
public:
    virtual ~NodoBloque() {}
    virtual void Compilar() = 0;
};