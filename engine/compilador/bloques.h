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

// Los bloques actuan como un arbol AST (Abstract Syntax Tree) que representa la logica de un objeto en la escena.
// Cada bloque puede contener otros bloques como hijos, formando una estructura jerarquica que define el comportamiento del objeto.
// Los bloques son compilados a scripts de behavior que son ejecutados por la CPU.
class NodoBloque {
public:
    virtual ~NodoBloque() {}
    virtual void Compilar() = 0;
};