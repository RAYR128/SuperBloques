#include "asm/op.h"
#include "ensamblador.h"
#include <cstring>

// $FFDC-$FFDF: Checksum + complement
// TO-DO: implementar calculo de checksum y complement, a la consola real le importa, pero a la mayoria de los emuladores no les importa
// asi que por ahora lo dejamos en 0xFFFF
void GenerarChecksum() {
	DROM[0x7FDC] = 0xFF;
	DROM[0x7FDD] = 0xFF;
	DROM[0x7FDE] = 0x00;
	DROM[0x7FDF] = 0x00;
}

void GenerarHeader() {
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

	// vectores de la CPU
	cc.SetearPC(0x7FEA);
	cc.CrearReferencia("I_NMI", REF_ABSOLUTE);
	cc.SetearPC(0x7FEE);
	cc.CrearReferencia("I_IRQ", REF_ABSOLUTE);
	cc.SetearPC(0x7FFA);
	cc.CrearReferencia("I_NMI", REF_ABSOLUTE);
	cc.SetearPC(0x7FFC);
	cc.CrearReferencia("I_RESET", REF_ABSOLUTE);
	cc.SetearPC(0x7FFE);
	cc.CrearReferencia("I_IRQ", REF_ABSOLUTE);
}

void EnsamblarROM() {
	// limpiar registros de control de interrupciones, dma de hardware, y puertos de audio
	cc.SetearPC(0x000000);
	cc.Etiqueta("I_RESET");
	cc.SetearFlags(FLAG_I);
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
	cc.LimpiarFlags(FLAG_C);
	cc.IntercambiarCarryConEmulacion();

	// REP #$38
	cc.LimpiarFlags(FLAG_X | FLAG_M | FLAG_D);

	cc.CargarRegConst16(REG_A, WRAM_DIRECTPAGE);
	cc.Transferir(REG_A, REG_DP);
	cc.CargarRegConst16(REG_A, WRAM_STACK);
	cc.Transferir(REG_A, REG_STACK);

	// SEP #$30
	cc.SetearFlags(FLAG_X | FLAG_M);

	// TO-DO: Crear loop, añadir NMI
	cc.PararCPU();

	// Finalizar ROM
	GenerarHeader();
	cc.ResolverReferencias();
	GenerarChecksum();
}