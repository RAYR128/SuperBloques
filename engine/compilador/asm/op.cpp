#include "rom.h"
#include "op.h"
#include <cstdio>
#include <iostream>

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
	if(flags == FLAG_CARRYF) {
		EmitirByte(OP_CLC_IMP);
		return;
	}
	if(flags == FLAG_DECIML) {
		EmitirByte(OP_CLD_IMP);
		return;
	}
	if(flags == FLAG_INTERR) {
		EmitirByte(OP_CLI_IMP);
		return;
	}
	if(flags == FLAG_OVERFL) {
		EmitirByte(OP_CLV_IMP);
		return;
	}
	EmitirByte(OP_REP_IMM8);
	EmitirByte(flags);
}

void Emitidor65816::SetearFlags(uint8_t flags) {
	// versiones mas cortas de 1 byte
	if(flags == FLAG_CARRYF) {
		EmitirByte(OP_SEC_IMP);
		return;
	}
	if(flags == FLAG_DECIML) {
		EmitirByte(OP_SED_IMP);
		return;
	}
	if(flags == FLAG_INTERR) {
		EmitirByte(OP_SEI_IMP);
		return;
	}
	EmitirByte(OP_SEP_IMM8);
	EmitirByte(flags);
}

void Emitidor65816::IntercambiarCarryConEmulacion() {
	EmitirByte(OP_XCE_IMP);
}

void Emitidor65816::IntercambiarBytesA() {
	EmitirByte(OP_XBA_IMP);
}

void Emitidor65816::EmitirInstMemoriaOptimizada(uint16_t opDp, uint16_t opAbs, uint16_t opLong, uint32_t addrHw) {
	if(opDp != 0xFFFF && (addrHw - WRAM_DIRECTPAGE) < 0x100) {
		EmitirByte((uint8_t)opDp);
		EmitirByte((uint8_t)(addrHw - WRAM_DIRECTPAGE));
		return;
	}
	if(opLong != 0xFFFF && (addrHw >> 16) > 0) {
		EmitirByte((uint8_t)opLong);
		Emitir24Bit(addrHw);
		return;
	}
	if(opAbs == 0xFFFF) {
		throw std::runtime_error("EmitirInstMemoriaOptimizada: no hay variante para la direccion");
	}
	EmitirByte((uint8_t)opAbs);
	EmitirPalabra((uint16_t)addrHw);
}

void Emitidor65816::AlmacenarCeroEnMemoria(uint32_t addrHw) {
	EmitirInstMemoriaOptimizada(OP_STZ_DP, OP_STZ_ABS, 0xFFFF, addrHw);
}

void Emitidor65816::AlmacenarCeroEnMemoria_IndX(uint32_t addrHw) {
	EmitirInstMemoriaOptimizada(OP_STZ_DPX, OP_STZ_ABSX, 0xFFFF, addrHw);
}

void Emitidor65816::ShiftALeft(int veces) {
	for(int i = 0; i < veces; i++) {
		EmitirByte(OP_ASL_ACC);
	}
}

void Emitidor65816::ShiftARight(int veces) {
	for(int i = 0; i < veces; i++) {
		EmitirByte(OP_LSR_ACC);
	}
}

void Emitidor65816::RotarARight(int veces) {
	for(int i = 0; i < veces; i++) {
		EmitirByte(OP_ROR_ACC);
	}
}

void Emitidor65816::IncrementarMemoria(uint32_t addrHw) {
	EmitirInstMemoriaOptimizada(OP_INC_DP, OP_INC_ABS, 0xFFFF, addrHw);
}

void Emitidor65816::DecrementarMemoria(uint32_t addrHw) {
	EmitirInstMemoriaOptimizada(OP_DEC_DP, OP_DEC_ABS, 0xFFFF, addrHw);
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
		throw std::runtime_error("CargarRegConst8: Register invalido");
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
		throw std::runtime_error("CargarRegConst16: Register invalido");
		break;
	}
}

void Emitidor65816::SumaAcumuladorConst8(uint8_t valor) {
	EmitirByte(OP_ADC_IMMM);
	EmitirByte(valor);
}

void Emitidor65816::SumaAcumuladorConst16(uint16_t valor) {
	EmitirByte(OP_ADC_IMMM);
	EmitirPalabra(valor);
}

void Emitidor65816::SumaAcumuladorMemoria(uint32_t addrHw) {
	EmitirInstMemoriaOptimizada(OP_ADC_DP, OP_ADC_ABS, OP_ADC_LONG, addrHw);
}

void Emitidor65816::RestaAcumuladorConst16(uint16_t valor) {
	EmitirByte(OP_SBC_IMMM);
	EmitirPalabra(valor);
}

void Emitidor65816::RestaAcumuladorMemoria(uint32_t addrHw) {
	EmitirInstMemoriaOptimizada(OP_SBC_DP, OP_SBC_ABS, OP_SBC_LONG, addrHw);
}

void Emitidor65816::ANDAcumuladorMemoria(uint32_t addrHw) {
	EmitirInstMemoriaOptimizada(OP_AND_DP, OP_AND_ABS, OP_AND_LONG, addrHw);
}

void Emitidor65816::ORAcumuladorMemoria(uint32_t addrHw) {
	EmitirInstMemoriaOptimizada(OP_ORA_DP, OP_ORA_ABS, OP_ORA_LONG, addrHw);
}

void Emitidor65816::EORAcumuladorMemoria(uint32_t addrHw) {
	EmitirInstMemoriaOptimizada(OP_EOR_DP, OP_EOR_ABS, OP_EOR_LONG, addrHw);
}

void Emitidor65816::ANDAcumuladorConst8(uint8_t valor) {
	EmitirByte(OP_AND_IMMM);
	EmitirByte(valor);
}

void Emitidor65816::ANDAcumuladorConst16(uint16_t valor) {
	EmitirByte(OP_AND_IMMM);
	EmitirPalabra(valor);
}

void Emitidor65816::ORAcumuladorConst8(uint8_t valor) {
	EmitirByte(OP_ORA_IMMM);
	EmitirByte(valor);
}

void Emitidor65816::ORAcumuladorConst16(uint16_t valor) {
	EmitirByte(OP_ORA_IMMM);
	EmitirPalabra(valor);
}

void Emitidor65816::CompararRegConst8(Registers reg, uint8_t valor) {
	switch(reg) {
	case REG_A:
		EmitirByte(OP_CMP_IMMM);
		EmitirByte(valor);
		break;
	case REG_X:
		EmitirByte(OP_CPX_IMMX);
		EmitirByte(valor);
		break;
	case REG_Y:
		EmitirByte(OP_CPY_IMMX);
		EmitirByte(valor);
		break;
	default:
		throw std::runtime_error("CargarRegConst16: Register invalido");
		break;
	}
}

void Emitidor65816::CompararRegConst16(Registers reg, uint16_t valor) {
	switch(reg) {
	case REG_A:
		EmitirByte(OP_CMP_IMMM);
		EmitirPalabra(valor);
		break;
	case REG_X:
		EmitirByte(OP_CPX_IMMX);
		EmitirPalabra(valor);
		break;
	case REG_Y:
		EmitirByte(OP_CPY_IMMX);
		EmitirPalabra(valor);
		break;
	default:
		throw std::runtime_error("CargarRegConst16: Register invalido");
		break;
	}
}

void Emitidor65816::CargarRegEnMemoria(Registers reg, uint32_t addrHw) {
	switch(reg) {
	case REG_A:
		EmitirInstMemoriaOptimizada(OP_LDA_DP, OP_LDA_ABS, OP_LDA_LONG, addrHw);
		break;
	case REG_X:
		EmitirInstMemoriaOptimizada(OP_LDX_DP, OP_LDX_ABS, 0xFFFF, addrHw);
		break;
	case REG_Y:
		EmitirInstMemoriaOptimizada(OP_LDY_DP, OP_LDY_ABS, 0xFFFF, addrHw);
		break;
	default:
		throw std::runtime_error("CargarRegEnMemoriaW: Register invalido");
		break;
	}
}

void Emitidor65816::CargarRegEnMemoria_IndX(Registers reg, uint32_t addrHw) {
	switch(reg) {
	case REG_A:
		EmitirInstMemoriaOptimizada(OP_LDA_DPX, OP_LDA_ABSX, OP_LDA_LONGX, addrHw);
		break;
	default:
		throw std::runtime_error("CargarRegEnMemoriaWX: Register invalido");
		break;
	}
}

void Emitidor65816::CargarRegEnMemoria_IndY(Registers reg, uint32_t addrHw) {
	switch(reg) {
	case REG_A:
		EmitirInstMemoriaOptimizada(0xFFFF, OP_LDA_ABSY, 0xFFFF, addrHw);
		break;
	default:
		throw std::runtime_error("CargarRegEnMemoriaWY: Register invalido");
		break;
	}
}

void Emitidor65816::CargarRegEnMemoria_SymLX(Registers reg, std::string label) {
	switch(reg) {
	case REG_A:
		EmitirByte(OP_LDA_LONGX);
		CrearReferencia(label, REF_LONG);
		Emitir24Bit(0x000000);
		break;
	default:
		throw std::runtime_error("CargarRegEnMemoria_SymLX: Register invalido");
		break;
	}
}

void Emitidor65816::SaltarLongIndirecto(uint32_t addrHw) {
	EmitirInstMemoriaOptimizada(0xFFFF, OP_JML_INDL, 0xFFFF, addrHw);
}

void Emitidor65816::AlmacenarRegEnMemoria(Registers reg, uint32_t addrHw) {
	switch(reg) {
	case REG_A:
		EmitirInstMemoriaOptimizada(OP_STA_DP, OP_STA_ABS, OP_STA_LONG, addrHw);
		break;
	case REG_X:
		EmitirInstMemoriaOptimizada(OP_STX_DP, OP_STX_ABS, 0xFFFF, addrHw);
		break;
	case REG_Y:
		EmitirInstMemoriaOptimizada(OP_STY_DP, OP_STY_ABS, 0xFFFF, addrHw);
		break;
	default:
		throw std::runtime_error("AlmacenarRegEnMemoriaW: Register invalido");
		break;
	}
}

void Emitidor65816::AlmacenarRegEnMemoria_IndX(Registers reg, uint32_t addrHw) {
	switch(reg) {
	case REG_A:
		EmitirInstMemoriaOptimizada(OP_STA_DPX, OP_STA_ABSX, OP_STA_LONGX, addrHw);
		break;
	default:
		throw std::runtime_error("AlmacenarRegEnMemoriaWX: Register invalido");
		break;
	}
}

void Emitidor65816::AlmacenarRegEnMemoria_IndY(Registers reg, uint32_t addrHw) {
	switch(reg) {
	case REG_A:
		EmitirInstMemoriaOptimizada(0xFFFF, OP_STA_ABSY, 0xFFFF, addrHw);
		break;
	default:
		throw std::runtime_error("AlmacenarRegEnMemoriaWY: Register invalido");
		break;
	}
}

void Emitidor65816::EsperarInterrupcion() {
	EmitirByte(OP_WAI_IMP);
}

void Emitidor65816::PararCPU() {
	EmitirByte(OP_STP_IMP);
}

void Emitidor65816::ReturnShort() {
	EmitirByte(OP_RTS_IMP);
}

void Emitidor65816::ReturnLong() {
	EmitirByte(OP_RTL_IMP);
}

void Emitidor65816::ReturnInterrupt() {
	EmitirByte(OP_RTI_IMP);
}

void Emitidor65816::Transferir(Registers entrada, Registers destino) {
	if(entrada == REG_A) {
		switch(destino) {
		case REG_STACK: EmitirByte(OP_TCS_IMP); break;
		case REG_DP: EmitirByte(OP_TCD_IMP); break;
		case REG_X: EmitirByte(OP_TAX_IMP); break;
		case REG_Y: EmitirByte(OP_TAY_IMP); break;
		default: throw std::runtime_error("Transferir: A->Destino invalido"); break;
		}
		return;
	}
	if(entrada == REG_X) {
		switch(destino) {
		case REG_A: EmitirByte(OP_TXA_IMP); break;
		case REG_Y: EmitirByte(OP_TXY_IMP); break;
		default: throw std::runtime_error("Transferir: X->Destino invalido"); break;
		}
		return;
	}
	if(entrada == REG_Y) {
		switch(destino) {
		case REG_A: EmitirByte(OP_TYA_IMP); break;
		case REG_X: EmitirByte(OP_TYX_IMP); break;
		default: throw std::runtime_error("Transferir: X->Destino invalido"); break;
		}
		return;
	}
	throw std::runtime_error("Transferir: Entrada invalida");
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
	default: throw std::runtime_error("Empujar: Register invalido"); break;
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
	default: throw std::runtime_error("Empujar: Register invalido"); break;
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
		throw std::runtime_error("Saltar: Tipo de referencia invalido");
		break;
	}
}

void Emitidor65816::Llamada(std::string label) {
	EmitirByte(OP_JSR_ABSJ);
	CrearReferencia(label, REF_ABSOLUTE);
	EmitirPalabra(0x0000);
}

void Emitidor65816::LlamadaLong(std::string label) {
	EmitirByte(OP_JSL_LONGJ);
	CrearReferencia(label, REF_LONG);
	Emitir24Bit(0x000000);
}

void Emitidor65816::Branch(std::string label, TipoBranch tipo) {
	switch(tipo) {
	case BRANCH_CARRY_SET:
		EmitirByte(OP_BCS_REL);
		break;
	case BRANCH_CARRY_CLEAR:
		EmitirByte(OP_BCC_REL);
		break;
	case BRANCH_ZERO_SET:
		EmitirByte(OP_BEQ_REL);
		break;
	case BRANCH_ZERO_CLEAR:
		EmitirByte(OP_BNE_REL);
		break;
	case BRANCH_NEGATIVE_CLEAR:
		EmitirByte(OP_BPL_REL);
		break;
	case BRANCH_NEGATIVE_SET:
		EmitirByte(OP_BMI_REL);
		break;
	case BRANCH_OVERFLOW_CLEAR:
		EmitirByte(OP_BVC_REL);
		break;
	case BRANCH_OVERFLOW_SET:
		EmitirByte(OP_BVS_REL);
		break;
	case BRANCH_ALWAYS:
		EmitirByte(OP_BRA_REL);
		break;
	default:
		throw std::runtime_error("Branch: Tipo de branch invalido");
		break;
	}
	ReferenciaCodigo ref;
	ref.nombre = label;
	ref.tipo = REF_BRANCH;
	ref.direccion = PC;
	referencias.push_back(ref);
	EmitirByte(0x00); // placeholder para la direccion del label
}

// BranchLong es una version inversa de un branch que emite un JMP para hacer branches mas largos, ya que los branches normales solo permiten 1 byte.
void Emitidor65816::BranchLong(std::string label, TipoBranch tipo) {
	switch(tipo) {
	case BRANCH_CARRY_SET:
		EmitirByte(OP_BCC_REL);
		break;
	case BRANCH_CARRY_CLEAR:
		EmitirByte(OP_BCS_REL);
		break;
	case BRANCH_ZERO_SET:
		EmitirByte(OP_BNE_REL);
		break;
	case BRANCH_ZERO_CLEAR:
		EmitirByte(OP_BEQ_REL);
		break;
	case BRANCH_NEGATIVE_CLEAR:
		EmitirByte(OP_BMI_REL);
		break;
	case BRANCH_NEGATIVE_SET:
		EmitirByte(OP_BPL_REL);
		break;
	case BRANCH_OVERFLOW_CLEAR:
		EmitirByte(OP_BVS_REL);
		break;
	case BRANCH_OVERFLOW_SET:
		EmitirByte(OP_BVC_REL);
		break;
	default:
		throw std::runtime_error("Branch: Tipo de branch invalido");
		break;
	}
	EmitirByte(3);
	EmitirByte(OP_JMP_ABSJ);
	CrearReferencia(label, REF_ABSOLUTE);
	EmitirPalabra(0x0000);
}

void Emitidor65816::GuardarSimbolosArchivo(const char *nombreArchivo) {
	FILE *archivo = fopen(nombreArchivo, "w");
	if(archivo) {
		// los emuladores esperan esta signatura
		fprintf(archivo, "; wla symbolic information file\n; generated by SuperBloques\n\n");
		fprintf(archivo, "[labels]\n");
		for(const EtiquetaCodigo &l : etiquetas) {
			uint32_t direccion = ConvertirAddrPcAHw(l.direccion);
			if(direccion == INVALIDO) {
				continue;
			}
			fprintf(archivo, "%02X:%04X %s\n", (direccion >> 16) & 0xFF, direccion & 0xFFFF, l.nombre.c_str());
		}
		fprintf(archivo, "\n[source files]\n");
		fprintf(archivo, "\n[rom checksum]\n%08x\n", CalcularCRC32ROM());
		fprintf(archivo, "\n[addr-to-line mapping]");
		fclose(archivo);
	}
}

void Emitidor65816::ResolverReferencias() {
	for(ReferenciaCodigo ref : referencias) {
		uint32_t direccion = INVALIDO;
		for(EtiquetaCodigo l : etiquetas) {
			if(l.nombre == ref.nombre) {
				// El label fue encontrado, convertir direccion de PC a direccion de hardware
				direccion = ConvertirAddrPcAHw(l.direccion);
				break;
			}
		}
		if(direccion != INVALIDO) {
			switch(ref.tipo) {
			case REF_BRANCH: {
				int32_t offset = (int32_t)direccion - ((int32_t)ConvertirAddrPcAHw(ref.direccion) + 1);
				if(offset < -128 || offset > 127) {
					throw std::runtime_error("ResolverReferencias: branch fuera de rango para " + ref.nombre);
				}
				DROM[ref.direccion] = (uint8_t)offset;
				break;
			}
			case REF_ABSOLUTE: {
				DROM[ref.direccion] = direccion & 0xFF;
				DROM[ref.direccion + 1] = (direccion >> 8) & 0xFF;
				break;
			}
			case REF_LONG: {
				DROM[ref.direccion] = direccion & 0xFF;
				DROM[ref.direccion + 1] = (direccion >> 8) & 0xFF;
				DROM[ref.direccion + 2] = (direccion >> 16) & 0xFF;
				break;
			}
			}
		} else {
			throw std::runtime_error("ResolverReferencias: No se encontro la etiqueta " + ref.nombre);
		}
	}
}

void Emitidor65816::EscribirEtiqueta(std::string label, bool grande) {
	if(grande) {
		CrearReferencia(label, REF_LONG);
		Emitir24Bit(0x000000);
	} else {
		CrearReferencia(label, REF_ABSOLUTE);
		EmitirPalabra(0x0000);
	}
}

void Emitidor65816::IncrementarReg(Registers reg) {
	switch(reg) {
	case REG_A:
		EmitirByte(OP_INC_ACC);
		break;
	case REG_X:
		EmitirByte(OP_INX_IMP);
		break;
	case REG_Y:
		EmitirByte(OP_INY_IMP);
		break;
	default:
		throw std::runtime_error("IncrementarReg: Register invalido");
		break;
	}
}

void Emitidor65816::DecrementarReg(Registers reg) {
	switch(reg) {
	case REG_A:
		EmitirByte(OP_DEC_ACC);
		break;
	case REG_X:
		EmitirByte(OP_DEX_IMP);
		break;
	case REG_Y:
		EmitirByte(OP_DEY_IMP);
		break;
	default:
		throw std::runtime_error("DecrementarReg: Register invalido");
		break;
	}
}