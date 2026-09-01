#include "asm/op.h"
#include "ensamblador.h"
#include <cstring>

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

	// crear un header basico para el ROM
	memset(&DROM[0x7FC0], ' ', 21);
	memcpy(&DROM[0x7FC0], "SuperBloques ROM", 12);
	DROM[0x7FD5] = 0x20; // $FFD5: Modo de mapa (LoROM, Sin fastROM)
	DROM[0x7FD6] = 0x00; // $FFD6: Tipo de cartucho (ROM solamente)
	DROM[0x7FD7] = 0x0B; // $FFD7: Tamaño de ROM (2MB = 2^11 KB -> N=11)
	DROM[0x7FD8] = 0x00; // $FFD8: Tamaño de SRAM (ninguno)
	DROM[0x7FD9] = 0x01; // $FFD9: Codigo de region (1 = NTSC)
	DROM[0x7FDA] = 0x33; // $FFDA: Version de ROM
	DROM[0x7FDB] = 0x00; // $FFDB: Version de ROM

	// $FFDC-$FFDF: Checksum + complement
	// TO-DO: implementar calculo de checksum y complement, a la consola real le importa, pero a la mayoria de los emuladores no les importa
	// asi que por ahora lo dejamos en 0xFFFF
	DROM[0x7FDC] = 0xFF; DROM[0x7FDD] = 0xFF;
	DROM[0x7FDE] = 0x00; DROM[0x7FDF] = 0x00;

	// vectores de la CPU
	DROM[0x7FEA] = 0x00; DROM[0x7FEB] = 0x80; // NMI -> $8000
	DROM[0x7FEE] = 0x00; DROM[0x7FEF] = 0x80; // IRQ -> $8000
	DROM[0x7FFA] = 0x00; DROM[0x7FFB] = 0x80; // NMI -> $8000
	DROM[0x7FFC] = 0x00; DROM[0x7FFD] = 0x80; // RESET-> $8000
	DROM[0x7FFE] = 0x00; DROM[0x7FFF] = 0x80; // IRQ/BRK -> $8000

	// limpiar registros de control de interrupciones, dma de hardware, y puertos de audio
	cc.SetearPC(0x000000);
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

	cc.PararCPU();
}