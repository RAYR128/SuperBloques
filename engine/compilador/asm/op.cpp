#include "rom.h"
#include "op.h"

// Generacion de codigo dinamico
Emitidor65816 cc;

Emitidor65816::Emitidor65816() {
	PC = 0;
}

// Escritura little endian
void Emitidor65816::EmitirByte(uint8_t byte) {
	DROM[PC++] = byte;
}

void Emitidor65816::EmitirPalabra(uint16_t byte) {
	DROM[PC++] = byte & 0xFF;
	DROM[PC++] = byte >> 8;
}

void Emitidor65816::Emitir24Bit(uint32_t doblePalabra) {
	DROM[PC++] = doblePalabra & 0xFF;
	DROM[PC++] = (doblePalabra >> 8) & 0xFF;
	DROM[PC++] = (doblePalabra >> 16) & 0xFF;
}

void Emitidor65816::EmitirDoblePalabra(uint32_t doblePalabra) {
	DROM[PC++] = doblePalabra & 0xFF;
	DROM[PC++] = (doblePalabra >> 8) & 0xFF;
	DROM[PC++] = (doblePalabra >> 16) & 0xFF;
	DROM[PC++] = (doblePalabra >> 24) & 0xFF;
}

// Opcodes rapidos
void Emitidor65816::LimpiarFlag(FlagOpcodes flg) {
	switch (flg) {
	case SFLG_CARRY:
		EmitirByte(OP_CLC_IMP);
		break;
	case SFLG_DECIMAL:
		EmitirByte(OP_CLD_IMP);
		break;
	case SFLG_INTERRUPT:
		EmitirByte(OP_CLI_IMP);
		break;
	case SFLG_OVERFLOW:
		EmitirByte(OP_CLV_IMP);
		break;
	}
}

void Emitidor65816::SetearFlag(FlagOpcodes flg) {
	switch (flg) {
	case SFLG_CARRY:
		EmitirByte(OP_SEC_IMP);
		break;
	case SFLG_DECIMAL:
		EmitirByte(OP_SED_IMP);
		break;
	case SFLG_INTERRUPT:
		EmitirByte(OP_SEI_IMP);
		break;
	case SFLG_OVERFLOW:
		// No hay opcode para setear el flag de overflow
		break;
	}
}

void Emitidor65816::ResetearFlags(uint8_t flags) {
	EmitirByte(OP_REP_IMM8);
	EmitirByte(flags);
}

void Emitidor65816::SetearFlags(uint8_t flags) {
	EmitirByte(OP_SEP_IMM8);
	EmitirByte(flags);
}

void Emitidor65816::IntercambiarCarryConEmulacion() {
	EmitirByte(OP_XCE_IMP);
}