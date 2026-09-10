#include "instancia.h"
#include "asm/op.h"
#include "codegen/codegen.h"

std::vector<Escena> EscenasProyecto;
std::vector<ObjetoEscena> ObjetosProyecto;

void CompilarBloquesBhv(std::string init, std::string main, std::string identificador, std::vector<NodoBloque> &lista, ContextoCompilacion contexto) {
	// Iniciar contexto
	SetContextoCompilacion(contexto);

	// Alguien puede poner bloques en el editor los cuales estan desconectados de un evento. Estos compilarian como bloques normales, hay que evitar esto
	// y solo empezar a compilar desde un bloque de evento real.
	cc.Etiqueta(init + identificador);
	for(NodoBloque &bloque : lista) {
		if(bloque.TipoDeBloque == BLOQUE_EVENTO_INIT) {
			bloque.Compilar();
		}
	}
	cc.ReturnLong();

	cc.Etiqueta(main + identificador);
	int uid = 0;
	for(NodoBloque &bloque : lista) {
		if(bloque.TipoDeBloque == BLOQUE_EVENTO_FRAME) {
			// Parametro especial: Solo ejecutar si el estado del script es el que se espera.
			if(bloque.ParametroEspecial) {
				cc.SetearFlags(FLAG_M_8BIT);
				cc.CargarRegEnMemoria_IndY(REG_A, WRAM_OBJETOS + PARAMETRO_OBJ_BHV_SCRIPT_STATUS);
				cc.CompararRegConst8(REG_A, bloque.ParametroEspecial);
				cc.LimpiarFlags(FLAG_M_8BIT);
				cc.BranchLong(main + "PR_" + std::to_string(uid) + "_", BRANCH_ZERO_CLEAR);
			}
			bloque.Compilar();
			cc.Etiqueta(main + "PR_" + std::to_string(uid) + "_" + identificador);
		}
	}
	cc.ReturnLong();
}

void Escena::Compilar() {
	CompilarBloquesBhv("ESCENA_INIT", "ESCENA_MAIN", Nombre, Bloques, CTX_ESCENA);
}

void ObjetoEscena::Compilar() {
	CompilarBloquesBhv("OBJETO_INIT", "OBJETO_MAIN", Nombre, Bloques, CTX_OBJETO);
}