#include "codegen.h"

void Emit_CAPA(NodoBloque *blk) {
	EmitirNoImplementado(blk);
}

static void EmitirGetLayer(NodoBloque *blk, uint16_t addr) {
	EsperarEntradas(blk, 0);
	cc.CargarRegEnMemoria(REG_A, addr);
}

static void EmitirSetLayer(NodoBloque *blk, uint16_t addr) {
	EsperarEntradas(blk, 1);
	CompilarExpresion(&blk->Entradas[0]);
	cc.AlmacenarRegEnMemoria(REG_A, addr);
}

void Emit_CAPA_LAYER1_POSITION_X(NodoBloque *blk) {
	EmitirGetLayer(blk, WRAM_V_LAYER1_X);
}

void Emit_CAPA_LAYER1_POSITION_Y(NodoBloque *blk) {
	EmitirGetLayer(blk, WRAM_V_LAYER1_Y);
}

void Emit_CAPA_LAYER2_POSITION_X(NodoBloque *blk) {
	EmitirGetLayer(blk, WRAM_V_LAYER2_X);
}

void Emit_CAPA_LAYER2_POSITION_Y(NodoBloque *blk) {
	EmitirGetLayer(blk, WRAM_V_LAYER2_Y);
}

void Emit_CAPA_LAYER3_POSITION_X(NodoBloque *blk) {
	EmitirGetLayer(blk, WRAM_V_LAYER3_X);
}

void Emit_CAPA_LAYER3_POSITION_Y(NodoBloque *blk) {
	EmitirGetLayer(blk, WRAM_V_LAYER3_Y);
}

void Emit_CAPA_SET_LAYER1_POSITION_X(NodoBloque *blk) {
	EmitirSetLayer(blk, WRAM_V_LAYER1_X);
}

void Emit_CAPA_SET_LAYER1_POSITION_Y(NodoBloque *blk) {
	EmitirSetLayer(blk, WRAM_V_LAYER1_Y);
}

void Emit_CAPA_SET_LAYER2_POSITION_X(NodoBloque *blk) {
	EmitirSetLayer(blk, WRAM_V_LAYER2_X);
}

void Emit_CAPA_SET_LAYER2_POSITION_Y(NodoBloque *blk) {
	EmitirSetLayer(blk, WRAM_V_LAYER2_Y);
}

void Emit_CAPA_SET_LAYER3_POSITION_X(NodoBloque *blk) {
	EmitirSetLayer(blk, WRAM_V_LAYER3_X);
}

void Emit_CAPA_SET_LAYER3_POSITION_Y(NodoBloque *blk) {
	EmitirSetLayer(blk, WRAM_V_LAYER3_Y);
}

static void EmitirCampoTile(NodoBloque *entrada, uint16_t mask, int shift, uint16_t dest) {
	CompilarExpresion(entrada);
	cc.ANDAcumuladorConst16(mask);
	if(shift) {
		cc.ShiftALeft(shift);
	}
	cc.ORAcumuladorMemoria(dest);
	cc.AlmacenarRegEnMemoria(REG_A, dest);
}

static void EmitirSetTile(NodoBloque *blk, ParametroBGSize bgSize, uint16_t vramBase) {
	EsperarEntradas(blk, 7);

	uint16_t sx = ScratchPush();
	CompilarExpresion(&blk->Entradas[0]);
	cc.AlmacenarRegEnMemoria(REG_A, sx);

	uint16_t sy = ScratchPush();
	CompilarExpresion(&blk->Entradas[1]);
	cc.AlmacenarRegEnMemoria(REG_A, sy);

	uint16_t tlp = ScratchPush();
	cc.CargarRegEnMemoria(REG_A, sx);
	cc.ANDAcumuladorConst16(31);
	cc.AlmacenarRegEnMemoria(REG_A, tlp);

	cc.CargarRegEnMemoria(REG_A, sy);
	cc.ANDAcumuladorConst16(31);
	cc.ShiftALeft(5);
	cc.LimpiarFlags(FLAG_CARRYF);
	cc.SumaAcumuladorMemoria(tlp);
	cc.AlmacenarRegEnMemoria(REG_A, tlp);

	switch(bgSize) {
	case HW_BGSC_Size_64x32:
		cc.CargarRegEnMemoria(REG_A, sx);
		cc.ANDAcumuladorConst16(0x0020);
		cc.ShiftALeft(5);
		cc.LimpiarFlags(FLAG_CARRYF);
		cc.SumaAcumuladorMemoria(tlp);
		cc.AlmacenarRegEnMemoria(REG_A, tlp);
		break;
	case HW_BGSC_Size_32x64:
		cc.CargarRegEnMemoria(REG_A, sy);
		cc.ANDAcumuladorConst16(0x0020);
		cc.ShiftALeft(5);
		cc.LimpiarFlags(FLAG_CARRYF);
		cc.SumaAcumuladorMemoria(tlp);
		cc.AlmacenarRegEnMemoria(REG_A, tlp);
		break;
	case HW_BGSC_Size_64x64:
		cc.CargarRegEnMemoria(REG_A, sx);
		cc.ANDAcumuladorConst16(0x0020);
		cc.ShiftALeft(5);
		cc.LimpiarFlags(FLAG_CARRYF);
		cc.SumaAcumuladorMemoria(tlp);
		cc.AlmacenarRegEnMemoria(REG_A, tlp);
		cc.CargarRegEnMemoria(REG_A, sy);
		cc.ANDAcumuladorConst16(0x0020);
		cc.ShiftALeft(6);
		cc.LimpiarFlags(FLAG_CARRYF);
		cc.SumaAcumuladorMemoria(tlp);
		cc.AlmacenarRegEnMemoria(REG_A, tlp);
		break;
	case HW_BGSC_Size_32x32:
		break;
	}

	cc.LimpiarFlags(FLAG_CARRYF);
	cc.SumaAcumuladorConst16(vramBase);
	cc.AlmacenarRegEnMemoria(REG_A, tlp);

	uint16_t tile = ScratchPush();
	CompilarExpresion(&blk->Entradas[2]);
	cc.ANDAcumuladorConst16(0x03FF);
	cc.AlmacenarRegEnMemoria(REG_A, tile);
	EmitirCampoTile(&blk->Entradas[3], 0x0007, 10, tile);
	EmitirCampoTile(&blk->Entradas[6], 0x0001, 13, tile);
	EmitirCampoTile(&blk->Entradas[4], 0x0001, 14, tile);
	EmitirCampoTile(&blk->Entradas[5], 0x0001, 15, tile);

	cc.CargarRegEnMemoria(REG_X, WRAM_V_QUEUESIZE);
	cc.CargarRegEnMemoria(REG_A, tlp);
	cc.AlmacenarRegEnMemoria_IndX(REG_A, WRAM_QUEUE_TILEMAP);
	cc.CargarRegEnMemoria(REG_A, tile);
	cc.AlmacenarRegEnMemoria_IndX(REG_A, WRAM_QUEUE_TILEMAP + 2);
	cc.IncrementarReg(REG_X);
	cc.IncrementarReg(REG_X);
	cc.IncrementarReg(REG_X);
	cc.IncrementarReg(REG_X);
	cc.AlmacenarRegEnMemoria(REG_X, WRAM_V_QUEUESIZE);

	ScratchPop();
	ScratchPop();
	ScratchPop();
	ScratchPop();
}

void Emit_CAPA_SET_TILE_LAYER1(NodoBloque *blk) {
	EmitirSetTile(blk, HW_BGSC_Size_64x64, ADD_VRAM_TILEMAP_LAYER1);
}

void Emit_CAPA_SET_TILE_LAYER2(NodoBloque *blk) {
	EmitirSetTile(blk, HW_BGSC_Size_32x32, ADD_VRAM_TILEMAP_LAYER2);
}

void Emit_CAPA_SET_TILE_LAYER3(NodoBloque *blk) {
	EmitirSetTile(blk, HW_BGSC_Size_32x32, ADD_VRAM_TILEMAP_LAYER3);
}
