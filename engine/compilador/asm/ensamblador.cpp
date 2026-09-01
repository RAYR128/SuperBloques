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

	// limpiar registros de control de interrupciones, dma de hardware, y puertos de audio
	cc.SetearFlag(SFLG_INTERRUPT);
	cc.AlmacenarCeroEnMemoriaW(HW_NMITIMEN);
	cc.AlmacenarCeroEnMemoriaW(HW_HDMAEN);
	cc.AlmacenarCeroEnMemoriaW(HW_APUIO0);
	cc.AlmacenarCeroEnMemoriaW(HW_APUIO1);
	cc.AlmacenarCeroEnMemoriaW(HW_APUIO2);
	cc.AlmacenarCeroEnMemoriaW(HW_APUIO3);

	// desactivar la pantalla y configurar el registro de control de video
	cc.CargarRegConst8(REG_A, 0x8F);
	cc.AlmacenarRegEnMemoriaW(REG_A, HW_INIDISP);

	// CLC : XCE, desactivar emulacion de 6502 y activar modo nativo de 65816
	cc.LimpiarFlag(SFLG_CARRY);
	cc.IntercambiarCarryConEmulacion();

	// REP #$38
	cc.ResetearFlags(FLAG_X | FLAG_M | FLAG_D);

	// SEP #$30
	cc.SetearFlags(FLAG_X | FLAG_M);
}