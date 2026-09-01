#include "rom.h"
#include "op.h"

// Generacion de codigo dinamico
Emitidor65816 cc;

Emitidor65816::Emitidor65816() {
	PC = 0;
}

void Emitidor65816::SetearPC(uint32_t direccion) {
	PC = direccion;
}

uint32_t Emitidor65816::ObtenerPC() {
	return PC;
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

void Emitidor65816::AlmacenarCeroEnMemoriaW(uint16_t addrHw) {
	EmitirByte(OP_STZ_ABS);
	EmitirPalabra(addrHw);
}

void Emitidor65816::AlmacenarCeroEnMemoriaWX(uint16_t addrHw) {
	EmitirByte(OP_STZ_ABSX);
	EmitirPalabra(addrHw);
}

void Emitidor65816::CargarRegConst8(Registers reg, uint8_t valor) {
	switch (reg) {
	case REG_A:
		EmitirByte(OP_LDA_IMMM);
		EmitirByte(valor);
		break;
	case REG_X:
		EmitirByte(OP_LDX_IMMX);
		EmitirByte(valor);
		break;
	case REG_Y:
		EmitirByte(OP_LDY_IMMX);
		EmitirByte(valor);
		break;
	default:
		break;
	}
}

void Emitidor65816::CargarRegConst16(Registers reg, uint16_t valor) {
	switch (reg) {
	case REG_A:
		EmitirByte(OP_LDA_IMMM);
		EmitirPalabra(valor);
		break;
	case REG_X:
		EmitirByte(OP_LDX_IMMX);
		EmitirPalabra(valor);
		break;
	case REG_Y:
		EmitirByte(OP_LDY_IMMX);
		EmitirPalabra(valor);
		break;
	default:
		break;
	}
}

void Emitidor65816::AlmacenarRegEnMemoriaW(Registers reg, uint16_t addrHw) {
	switch (reg) {
	case REG_A:
		EmitirByte(OP_STA_ABS);
		EmitirPalabra(addrHw);
		break;
	case REG_X:
		EmitirByte(OP_STX_ABS);
		EmitirPalabra(addrHw);
		break;
	case REG_Y:
		EmitirByte(OP_STY_ABS);
		EmitirPalabra(addrHw);
		break;
	default:
		break;
	}
}

void Emitidor65816::AlmacenarRegEnMemoriaWX(Registers reg, uint16_t addrHw) {
	switch (reg) {
	case REG_A:
		EmitirByte(OP_STA_ABSX);
		EmitirPalabra(addrHw);
		break;
	default:
		break;
	}
}

void Emitidor65816::AlmacenarRegEnMemoriaWY(Registers reg, uint16_t addrHw) {
	switch (reg) {
	case REG_A:
		EmitirByte(OP_STA_ABSY);
		EmitirPalabra(addrHw);
		break;
	default:
		break;
	}
}

void Emitidor65816::PararCPU() {
	EmitirByte(OP_STP_IMP);
}