#include "codegen.h"
#include <stdexcept>
#include <string>

void Emit_OPERACION(NodoBloque *blk) {
	EmitirNoImplementado(blk);
}

enum class BinariaOp {
	Add,
	Sub,
	And,
	Or,
	Xor
};

static void EmitirBinaria(NodoBloque *blk, BinariaOp op) {
	EsperarEntradas(blk, 2);
	NodoBloque *izq = &blk->Entradas[0];
	NodoBloque *der = &blk->Entradas[1];
	bool resta = (op == BinariaOp::Sub);
	bool conmutativa = !resta;

	auto aplicarInmediato = [&](uint16_t imm) {
		switch(op) {
		case BinariaOp::Add:
			cc.LimpiarFlags(FLAG_CARRYF);
			cc.SumaAcumuladorConst16(imm);
			break;
		case BinariaOp::Sub:
			cc.SetearFlags(FLAG_CARRYF);
			cc.RestaAcumuladorConst16(imm);
			break;
		case BinariaOp::And:
			cc.ANDAcumuladorConst16(imm);
			break;
		case BinariaOp::Or:
			cc.ORAcumuladorConst16(imm);
			break;
		case BinariaOp::Xor:
			cc.EORAcumuladorConst16(imm);
			break;
		}
	};
	auto aplicarMemoria = [&](uint16_t addr) {
		switch(op) {
		case BinariaOp::Add:
			cc.LimpiarFlags(FLAG_CARRYF);
			cc.SumaAcumuladorMemoria(addr);
			break;
		case BinariaOp::Sub:
			cc.SetearFlags(FLAG_CARRYF);
			cc.RestaAcumuladorMemoria(addr);
			break;
		case BinariaOp::And:
			cc.ANDAcumuladorMemoria(addr);
			break;
		case BinariaOp::Or:
			cc.ORAcumuladorMemoria(addr);
			break;
		case BinariaOp::Xor:
			cc.EORAcumuladorMemoria(addr);
			break;
		}
	};

	if(der->TipoDeBloque == BLOQUE_NUMERO) {
		if(izq->TipoDeBloque == BLOQUE_NUMERO) {
			cc.CargarRegConst16(REG_A, (uint16_t)izq->ParametroEspecial);
		} else {
			CompilarExpresion(izq);
		}
		aplicarInmediato((uint16_t)der->ParametroEspecial);
		return;
	}

	if(conmutativa && izq->TipoDeBloque == BLOQUE_NUMERO) {
		CompilarExpresion(der);
		aplicarInmediato((uint16_t)izq->ParametroEspecial);
		return;
	}

	if(resta) {
		CompilarExpresion(der);
		uint16_t slot = ScratchPush();
		cc.AlmacenarRegEnMemoria(REG_A, slot);
		if(izq->TipoDeBloque == BLOQUE_NUMERO) {
			cc.CargarRegConst16(REG_A, (uint16_t)izq->ParametroEspecial);
		} else {
			CompilarExpresion(izq);
		}
		aplicarMemoria(slot);
		ScratchPop();
		return;
	}

	CompilarExpresion(izq);
	uint16_t slot = ScratchPush();
	cc.AlmacenarRegEnMemoria(REG_A, slot);
	CompilarExpresion(der);
	aplicarMemoria(slot);
	ScratchPop();
}

void Emit_OPERACION_SUMA(NodoBloque *blk) {
	EmitirBinaria(blk, BinariaOp::Add);
}

void Emit_OPERACION_RESTA(NodoBloque *blk) {
	EmitirBinaria(blk, BinariaOp::Sub);
}

void Emit_OPERACION_AND(NodoBloque *blk) {
	EmitirBinaria(blk, BinariaOp::And);
}

void Emit_OPERACION_OR(NodoBloque *blk) {
	EmitirBinaria(blk, BinariaOp::Or);
}

void Emit_OPERACION_XOR(NodoBloque *blk) {
	EmitirBinaria(blk, BinariaOp::Xor);
}

static bool EsOperando8Directo(NodoBloque *n) {
	return n->TipoDeBloque == BLOQUE_NUMERO || n->TipoDeBloque == BLOQUE_VARIABLE;
}

static void CargarOperando8Directo(NodoBloque *n) {
	if(n->TipoDeBloque == BLOQUE_NUMERO) {
		cc.CargarRegConst8(REG_A, (uint8_t)n->ParametroEspecial);
	} else {
		CargarVariableEnA(n->ParametroEspecial);
	}
}

static void EscribirMpyaDesdeA8() {
	cc.AlmacenarRegEnMemoria(REG_A, HW_MPYA);
	cc.IntercambiarBytesA();
	cc.AlmacenarRegEnMemoria(REG_A, HW_MPYA);
}

static void EscribirMpyaDesdeConst8(uint16_t v) {
	cc.CargarRegConst8(REG_A, (uint8_t)v);
	cc.AlmacenarRegEnMemoria(REG_A, HW_MPYA);
	cc.CargarRegConst8(REG_A, (uint8_t)(v >> 8));
	cc.AlmacenarRegEnMemoria(REG_A, HW_MPYA);
}

enum class HardwareAritmetica {
	Mul,
	Div,
	Mod
};

static void EmitirMulDivMod(NodoBloque *blk, HardwareAritmetica op) {
	EsperarEntradas(blk, 2);
	NodoBloque *izq = &blk->Entradas[0];
	NodoBloque *der = &blk->Entradas[1];
	bool mul = (op == HardwareAritmetica::Mul);

	auto dispararB = [&]() {
		cc.AlmacenarRegEnMemoria(REG_A, mul ? HW_MPYB : HW_WRDIVB);
	};
	auto terminarYLeer = [&]() {
		cc.LimpiarFlags(FLAG_M_8BIT);
		if(!mul) {
			cc.Nop(8);
		}
		if(op == HardwareAritmetica::Mul) {
			cc.CargarRegEnMemoria(REG_A, HW_MPY);
		} else if(op == HardwareAritmetica::Div) {
			cc.CargarRegEnMemoria(REG_A, HW_RDDIV);
		} else {
			cc.CargarRegEnMemoria(REG_A, HW_RDMPY);
		}
	};
	auto escribirA16 = [&]() {
		if(mul) {
			cc.SetearFlags(FLAG_M_8BIT);
			EscribirMpyaDesdeA8();
		} else {
			cc.AlmacenarRegEnMemoria(REG_A, HW_WRDIV);
			cc.SetearFlags(FLAG_M_8BIT);
		}
	};

	if(EsOperando8Directo(der)) {
		if(izq->TipoDeBloque == BLOQUE_NUMERO) {
			cc.CargarRegConst16(REG_A, (uint16_t)izq->ParametroEspecial);
		} else {
			CompilarExpresion(izq);
		}
		escribirA16();
		CargarOperando8Directo(der);
		dispararB();
		terminarYLeer();
		return;
	}

	if(izq->TipoDeBloque == BLOQUE_NUMERO) {
		uint16_t a = (uint16_t)izq->ParametroEspecial;
		CompilarExpresion(der);
		cc.SetearFlags(FLAG_M_8BIT);
		cc.Empujar(REG_A);
		if(mul) {
			EscribirMpyaDesdeConst8(a);
		} else {
			cc.LimpiarFlags(FLAG_M_8BIT);
			cc.CargarRegConst16(REG_A, a);
			cc.AlmacenarRegEnMemoria(REG_A, HW_WRDIV);
			cc.SetearFlags(FLAG_M_8BIT);
		}
		cc.Sacar(REG_A);
		dispararB();
		terminarYLeer();
		return;
	}

	CompilarExpresion(izq);
	uint16_t slotA = ScratchPush();
	cc.AlmacenarRegEnMemoria(REG_A, slotA);
	CompilarExpresion(der);
	cc.SetearFlags(FLAG_M_8BIT);
	cc.Empujar(REG_A);
	cc.LimpiarFlags(FLAG_M_8BIT);
	cc.CargarRegEnMemoria(REG_A, slotA);
	escribirA16();
	cc.Sacar(REG_A);
	dispararB();
	terminarYLeer();
	ScratchPop();
}

void Emit_OPERACION_MULTIPLICACION(NodoBloque *blk) {
	EmitirMulDivMod(blk, HardwareAritmetica::Mul);
}

void Emit_OPERACION_DIVISION(NodoBloque *blk) {
	EmitirMulDivMod(blk, HardwareAritmetica::Div);
}

void Emit_OPERACION_MOD(NodoBloque *blk) {
	EmitirMulDivMod(blk, HardwareAritmetica::Mod);
}

enum class ShiftOp {
	Asl,
	Lsr,
	Rol,
	Ror
};

static void EmitirShift(NodoBloque *blk, ShiftOp op, int minVeces, int maxVeces) {
	EsperarEntradas(blk, 1);
	int veces = blk->ParametroEspecial;
	if(veces < minVeces || veces > maxVeces) {
		throw std::runtime_error(ConvertirTipoDeBloqueAString(blk->TipoDeBloque) + " requiere ParametroEspecial en " + std::to_string(minVeces) + "-" + std::to_string(maxVeces));
	}
	CompilarExpresion(&blk->Entradas[0]);
	switch(op) {
	case ShiftOp::Asl:
		cc.ShiftALeft(veces);
		break;
	case ShiftOp::Lsr:
		cc.ShiftARight(veces);
		break;
	case ShiftOp::Rol:
		for(int i = 0; i < veces; i++) {
			cc.CompararRegConst16(REG_A, 0x8000);
			cc.RotarALeft(1);
		}
		break;
	case ShiftOp::Ror:
		for(int i = 0; i < veces; i++) {
			cc.Empujar(REG_A);
			cc.ShiftARight(1);
			cc.Sacar(REG_A);
			cc.RotarARight(1);
		}
		break;
	}
}

void Emit_OPERACION_ASL(NodoBloque *blk) {
	EmitirShift(blk, ShiftOp::Asl, 1, 15);
}

void Emit_OPERACION_LSR(NodoBloque *blk) {
	EmitirShift(blk, ShiftOp::Lsr, 1, 15);
}

void Emit_OPERACION_ROL(NodoBloque *blk) {
	EmitirShift(blk, ShiftOp::Rol, 1, 32);
}

void Emit_OPERACION_ROR(NodoBloque *blk) {
	EmitirShift(blk, ShiftOp::Ror, 1, 32);
}

enum class ComparacionOp {
	Eq,
	Neq,
	Gt,
	Gte,
	Lt,
	Lte
};

static void EmitirResultado01(const std::string &si) {
	std::string fin = EtiquetaLocal("CMPF");
	cc.CargarRegConst16(REG_A, 0);
	cc.Branch(fin, BRANCH_ALWAYS);
	cc.Etiqueta(si);
	cc.CargarRegConst16(REG_A, 1);
	cc.Etiqueta(fin);
}

static void EmitirComparacion(NodoBloque *blk, ComparacionOp op) {
	EsperarEntradas(blk, 2);
	NodoBloque *izq = &blk->Entradas[0];
	NodoBloque *der = &blk->Entradas[1];
	bool signedOrder = (op != ComparacionOp::Eq && op != ComparacionOp::Neq);
	std::string si = EtiquetaLocal("CMPS");

	if(der->TipoDeBloque == BLOQUE_NUMERO) {
		uint16_t imm = (uint16_t)der->ParametroEspecial;
		if(izq->TipoDeBloque == BLOQUE_NUMERO) {
			cc.CargarRegConst16(REG_A, (uint16_t)izq->ParametroEspecial);
		} else {
			CompilarExpresion(izq);
		}
		if(signedOrder) {
			cc.EORAcumuladorConst16(0x8000);
			imm ^= 0x8000;
		}
		cc.CompararRegConst16(REG_A, imm);
	} else {
		CompilarExpresion(der);
		if(signedOrder) {
			cc.EORAcumuladorConst16(0x8000);
		}
		uint16_t slot = ScratchPush();
		cc.AlmacenarRegEnMemoria(REG_A, slot);
		if(izq->TipoDeBloque == BLOQUE_NUMERO) {
			cc.CargarRegConst16(REG_A, (uint16_t)izq->ParametroEspecial);
		} else {
			CompilarExpresion(izq);
		}
		if(signedOrder) {
			cc.EORAcumuladorConst16(0x8000);
		}
		cc.CompararAcumuladorMemoria(slot);
		ScratchPop();
	}

	switch(op) {
	case ComparacionOp::Eq:
		cc.Branch(si, BRANCH_ZERO_SET);
		break;
	case ComparacionOp::Neq:
		cc.Branch(si, BRANCH_ZERO_CLEAR);
		break;
	case ComparacionOp::Lt:
		cc.Branch(si, BRANCH_CARRY_CLEAR);
		break;
	case ComparacionOp::Gte:
		cc.Branch(si, BRANCH_CARRY_SET);
		break;
	case ComparacionOp::Gt: {
		std::string no = EtiquetaLocal("CMPN");
		cc.Branch(no, BRANCH_ZERO_SET);
		cc.Branch(si, BRANCH_CARRY_SET);
		cc.Etiqueta(no);
		break;
	}
	case ComparacionOp::Lte:
		cc.Branch(si, BRANCH_CARRY_CLEAR);
		cc.Branch(si, BRANCH_ZERO_SET);
		break;
	}
	EmitirResultado01(si);
}

void Emit_OPERACION_EQ(NodoBloque *blk) {
	EmitirComparacion(blk, ComparacionOp::Eq);
}

void Emit_OPERACION_NEQ(NodoBloque *blk) {
	EmitirComparacion(blk, ComparacionOp::Neq);
}

void Emit_OPERACION_GT(NodoBloque *blk) {
	EmitirComparacion(blk, ComparacionOp::Gt);
}

void Emit_OPERACION_GTE(NodoBloque *blk) {
	EmitirComparacion(blk, ComparacionOp::Gte);
}

void Emit_OPERACION_LT(NodoBloque *blk) {
	EmitirComparacion(blk, ComparacionOp::Lt);
}

void Emit_OPERACION_LTE(NodoBloque *blk) {
	EmitirComparacion(blk, ComparacionOp::Lte);
}
