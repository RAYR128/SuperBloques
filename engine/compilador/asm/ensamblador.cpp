#include "asm/op.h"
#include "ensamblador.h"

void EnsamblarROM() {
	/*
		TO-DO: Tenemos que lograr lo siguiente para iniciar el hardware de la consola:
		SEI
		STZ.W HW_NMITIMEN
		STZ.W HW_HDMAEN
		STZ.W HW_MDMAEN
		STZ.W HW_APUIO0
		STZ.W HW_APUIO1
		STZ.W HW_APUIO2
		STZ.W HW_APUIO3
		LDA.B #$8F
		STA.W HW_INIDISP
		CLC
		XCE
		REP #$38
			LDA.W #$0000 : TCD
			LDA.W #$1FFF : TCS
		SEP #$30
	*/
	cc.SetearFlag(SFLG_INTERRUPT);

	// CLC : XCE
	cc.LimpiarFlag(SFLG_CARRY);
	cc.IntercambiarCarryConEmulacion();

	// REP #$38
	cc.ResetearFlags(FLAG_X | FLAG_M | FLAG_D);

	// SEP #$30
	cc.SetearFlags(FLAG_X | FLAG_M);
}