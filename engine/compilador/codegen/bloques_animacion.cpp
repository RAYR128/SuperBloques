#include "codegen.h"
#include <string>

void Emit_ANIMACION(NodoBloque *blk) {
	EmitirNoImplementado(blk);
}

static void ClampAUnsigned(uint16_t max, const char *tag) {
	std::string skip = EtiquetaLocal(tag);
	cc.CompararRegConst16(REG_A, max);
	cc.Branch(skip, BRANCH_CARRY_CLEAR);
	cc.CargarRegConst16(REG_A, max);
	cc.Etiqueta(skip);
}

static void StoreA8(uint16_t addr) {
	cc.SetearFlags(FLAG_X_8BIT | FLAG_M_8BIT);
	cc.AlmacenarRegEnMemoria(REG_A, addr);
	cc.LimpiarFlags(FLAG_X_8BIT | FLAG_M_8BIT);
}

void Emit_ANIMACION_OBJ_SET_SPRITE(NodoBloque *blk) {
	EsperarContexto(blk, CTX_OBJETO);
	EsperarEntradas(blk, 1);
	CompilarExpresion(&blk->Entradas[0]);
	cc.AlmacenarRegEnMemoria_IndY(REG_A, WRAM_OBJETOS + PARAMETRO_OBJ_SPRITE);
}

void Emit_ANIMACION_SCENE_SET_MOSAIC_FILTER(NodoBloque *blk) {
	EsperarEntradas(blk, 1);
	CompilarExpresion(&blk->Entradas[0]);
	cc.ORAcumuladorConst16(0x000F);
	ClampAUnsigned(0x00FF, "MOS");
	StoreA8(WRAM_V_FILTRO_MOSAICO);
}

void Emit_ANIMACION_SCENE_SET_BRIGHTNESS(NodoBloque *blk) {
	EsperarEntradas(blk, 1);
	CompilarExpresion(&blk->Entradas[0]);
	cc.ShiftARight(4);
	ClampAUnsigned(0x000F, "BRI");
	StoreA8(WRAM_V_BRILLO);
}

static void EmitirCanal5Bit(NodoBloque *entrada, int shift, const char *tag) {
	CompilarExpresion(entrada);
	cc.ShiftARight(3);
	ClampAUnsigned(0x001F, tag);
	if(shift) {
		cc.ShiftALeft(shift);
	}
}

void Emit_ANIMACION_SCENE_SET_COLOR(NodoBloque *blk) {
	EsperarEntradas(blk, 4);

	CompilarExpresion(&blk->Entradas[0]);
	cc.ANDAcumuladorConst16(0x00FF);
	cc.ShiftALeft(1);
	cc.Transferir(REG_A, REG_X);

	uint16_t color = ScratchPush();
	EmitirCanal5Bit(&blk->Entradas[1], 0, "CR");
	cc.AlmacenarRegEnMemoria(REG_A, color);

	EmitirCanal5Bit(&blk->Entradas[2], 5, "CG");
	cc.ORAcumuladorMemoria(color);
	cc.AlmacenarRegEnMemoria(REG_A, color);

	EmitirCanal5Bit(&blk->Entradas[3], 10, "CB");
	cc.ORAcumuladorMemoria(color);
	cc.AlmacenarRegEnMemoria_IndX(REG_A, WRAM_PALETA);
	ScratchPop();
}
