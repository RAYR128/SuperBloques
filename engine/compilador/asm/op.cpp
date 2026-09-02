#include "rom.h"
#include "op.h"
#include <iostream>

#define ErrorSB(msg) std::cout << "Error: " << msg << std::endl; exit(1);

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

void Emitidor65816::LimpiarFlags(uint8_t flags) {
	// versiones mas cortas de 1 byte
	if(flags == FLAG_C) {
		EmitirByte(OP_CLC_IMP);
		return;
	}
	if(flags == FLAG_D) {
		EmitirByte(OP_CLD_IMP);
		return;
	}
	if(flags == FLAG_I) {
		EmitirByte(OP_CLI_IMP);
		return;
	}
	if(flags == FLAG_V) {
		EmitirByte(OP_CLV_IMP);
		return;
	}
	EmitirByte(OP_REP_IMM8);
	EmitirByte(flags);
}

void Emitidor65816::SetearFlags(uint8_t flags) {
	// versiones mas cortas de 1 byte
	if(flags == FLAG_C) {
		EmitirByte(OP_SEC_IMP);
		return;
	}
	if(flags == FLAG_D) {
		EmitirByte(OP_SED_IMP);
		return;
	}
	if(flags == FLAG_I) {
		EmitirByte(OP_SEI_IMP);
		return;
	}
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
	switch(reg) {
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
		ErrorSB("CargarRegConst8: Register invalido");
		break;
	}
}

void Emitidor65816::CargarRegConst16(Registers reg, uint16_t valor) {
	switch(reg) {
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
		ErrorSB("CargarRegConst16: Register invalido");
		break;
	}
}

void Emitidor65816::AlmacenarRegEnMemoriaW(Registers reg, uint16_t addrHw) {
	switch(reg) {
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
		ErrorSB("AlmacenarRegEnMemoriaW: Register invalido");
		break;
	}
}

void Emitidor65816::AlmacenarRegEnMemoriaWX(Registers reg, uint16_t addrHw) {
	switch(reg) {
	case REG_A:
		EmitirByte(OP_STA_ABSX);
		EmitirPalabra(addrHw);
		break;
	default:
		ErrorSB("AlmacenarRegEnMemoriaWX: Register invalido");
		break;
	}
}

void Emitidor65816::AlmacenarRegEnMemoriaWY(Registers reg, uint16_t addrHw) {
	switch(reg) {
	case REG_A:
		EmitirByte(OP_STA_ABSY);
		EmitirPalabra(addrHw);
		break;
	default:
		ErrorSB("AlmacenarRegEnMemoriaWY: Register invalido");
		break;
	}
}

void Emitidor65816::PararCPU() {
	EmitirByte(OP_STP_IMP);
}

void Emitidor65816::Transferir(Registers entrada, Registers destino) {
	if(entrada == REG_A) {
		switch(destino) {
		case REG_STACK: EmitirByte(OP_TCS_IMP); break;
		case REG_DP: EmitirByte(OP_TCD_IMP); break;
		case REG_X: EmitirByte(OP_TAX_IMP); break;
		case REG_Y: EmitirByte(OP_TAY_IMP); break;
		default: ErrorSB("Transferir: A->Destino invalido"); break;
		}
		return;
	}
	ErrorSB("Transferir: Entrada invalida");
}

void Emitidor65816::Empujar(Registers reg) {
	switch(reg) {
	case REG_A: EmitirByte(OP_PHA_IMP); break;
	case REG_X: EmitirByte(OP_PHX_IMP); break;
	case REG_Y: EmitirByte(OP_PHY_IMP); break;
	case REG_BANK: EmitirByte(OP_PHB_IMP); break;
	case REG_EXECBANK: EmitirByte(OP_PHK_IMP); break;
	case REG_FLAGS: EmitirByte(OP_PHP_IMP); break;
	case REG_DP: EmitirByte(OP_PHD_IMP); break;
	default: ErrorSB("Empujar: Register invalido"); break;
	}
}

void Emitidor65816::Sacar(Registers reg) {
	switch(reg) {
	case REG_A: EmitirByte(OP_PLA_IMP); break;
	case REG_X: EmitirByte(OP_PLX_IMP); break;
	case REG_Y: EmitirByte(OP_PLY_IMP); break;
	case REG_BANK: EmitirByte(OP_PLB_IMP); break;
	// no existe REG_EXECBANK, en todo caso cuenta como program counter.
	case REG_FLAGS: EmitirByte(OP_PLP_IMP); break;
	case REG_DP: EmitirByte(OP_PLD_IMP); break;
	default: ErrorSB("Empujar: Register invalido"); break;
	}
}

void Emitidor65816::Etiqueta(std::string nombre) {
	EtiquetaCodigo l;
	l.nombre = nombre;
	l.direccion = PC;
	etiquetas.push_back(l);
}

void Emitidor65816::CrearReferencia(std::string label, TipoReferencia tipo) {
	ReferenciaCodigo ref;
	ref.nombre = label;
	ref.tipo = tipo;
	ref.direccion = PC;
	referencias.push_back(ref);
}

void Emitidor65816::Saltar(std::string label, TipoReferencia tipo) {
	ReferenciaCodigo ref;
	ref.nombre = label;
	ref.tipo = tipo;
	ref.direccion = PC;
	referencias.push_back(ref);
	switch(tipo) {
	case REF_BRANCH:
		EmitirByte(OP_BRA_REL);
		CrearReferencia(label, REF_BRANCH);
		EmitirByte(0x00);
		break;
	case REF_ABSOLUTE:
		EmitirByte(OP_JMP_ABSJ);
		CrearReferencia(label, REF_ABSOLUTE);
		EmitirPalabra(0x0000);
		break;
	case REF_LONG:
		EmitirByte(OP_JML_LONGJ);
		CrearReferencia(label, REF_LONG);
		Emitir24Bit(0x000000);
		break;
	default:
		ErrorSB("Saltar: Tipo de referencia invalido");
		break;
	}
}

void Emitidor65816::ResolverReferencias() {
	for(ReferenciaCodigo ref : referencias) {
		uint32_t direccion = INVALIDO;
		for(EtiquetaCodigo l : etiquetas) {
			if(l.nombre == ref.nombre) {
				direccion = l.direccion;
				break;
			}
		}
		if(direccion != INVALIDO) {
			switch(ref.tipo) {
			case REF_BRANCH:
				DROM[ref.direccion] = direccion & 0xFF;
				break;
			case REF_ABSOLUTE:
				DROM[ref.direccion] = direccion & 0xFF;
				DROM[ref.direccion + 1] = (direccion >> 8) & 0xFF;
				break;
			case REF_LONG:
				DROM[ref.direccion] = direccion & 0xFF;
				DROM[ref.direccion + 1] = (direccion >> 8) & 0xFF;
				DROM[ref.direccion + 2] = (direccion >> 16) & 0xFF;
				break;
			}
		} else {
			ErrorSB("ResolverReferencias: No se encontro la etiqueta " + ref.nombre);
		}
	}
}