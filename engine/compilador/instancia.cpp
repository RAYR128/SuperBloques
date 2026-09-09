#include "instancia.h"
#include "asm/op.h"

std::vector<Escena> EscenasProyecto;
std::vector<ObjetoEscena> ObjetosProyecto;

void Escena::Compilar() {
    cc.Etiqueta("ESCENA_ENTRY_" + Nombre);
    for(NodoBloque bloque : Bloques) {
        if(bloque.EsInicioEvento()) {
            bloque.Compilar();
        }
    }
    cc.ReturnLong();
}

void ObjetoEscena::Compilar() {
    cc.Etiqueta("OBJETO_ENTRY_" + Nombre);
    for(NodoBloque bloque : Bloques) {
        if(bloque.EsInicioEvento()) {
            bloque.Compilar();
        }
    }
    cc.ReturnLong();
}