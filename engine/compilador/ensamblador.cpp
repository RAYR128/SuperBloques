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
	memcpy(&DROM[0x7FC0], "SuperBloques", 12);
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

// La rutina de RESET se ejecuta cuando el hardware genera un reset, ya sea por presionar el boton de reset, encender la consola, o por un fallo de energia.
// En este caso, el hardware genera un reset al encender la consola, lo que permite al programa inicializar la logica y la pantalla.
void RutinaRESET() {
	// limpiar registros de control de interrupciones, dma de hardware, y puertos de audio
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

	// Loop de programa
	cc.Etiqueta("PROGRAM_LOOP");

	// Chequear lectura de joypad
	cc.CargarRegEnMemoriaW(REG_A, HW_HVBJOY);
	cc.ShiftARight();
	cc.Branch("PROGRAM_LOOP", BRANCH_CARRY_SET);

	// Esperar a que el hardware genere un VBlank, para sincronizar la logica con la pantalla
	cc.IncrementarMemoria(WRAM_FLAG_EJECUCION);
	cc.CargarRegEnMemoriaW(REG_A, HW_RDNMI); // Leer flag de NMI para evitar que el interrupt se ejecute de inmediato
	cc.CargarRegConst8(REG_A, 0x81);		 // Activar NMI + Auto joypad read
	cc.AlmacenarRegEnMemoriaW(REG_A, HW_NMITIMEN);
	cc.EsperarInterrupcion(); // Esperar una interrupcion

	// Podemos ejecutar un nuevo cuadro?
	cc.Etiqueta("ESPERAR_BLANK");
	cc.CargarRegEnMemoriaW(REG_A, WRAM_FLAG_EJECUCION);
	cc.Branch("ESPERAR_BLANK", BRANCH_ZERO_CLEAR);

	// Repetir
	cc.Saltar("PROGRAM_LOOP", REF_ABSOLUTE);
}

// La rutina de NMI se ejecuta cuando el hardware genera una interrupcion no enmascarable (NMI).
// En este caso, el hardware genera un NMI cada vez que se produce un cambio de fotograma (VBlank),
// lo que permite al programa actualizar la pantalla y procesar la logica.
// Usamos la logica de VBlank para mantener una tasa de refresco constante y sincronizada con la pantalla.
void RutinaNMI() {
	cc.Etiqueta("I_NMI");

	// preservar estado de CPU durante interrupcion
	cc.Empujar(REG_FLAGS);
	cc.LimpiarFlags(FLAG_X | FLAG_M | FLAG_D);
	cc.Empujar(REG_BANK);
	cc.Empujar(REG_A);
	cc.Empujar(REG_X);
	cc.Empujar(REG_Y);
	cc.SetearFlags(FLAG_X | FLAG_M);

	// desactivar el interrupt de vblank, dejar solo auto joypad read activado.
	// esto es para prevenir un bug en el cual si el NMI tarda demasiado en ejecutarse, otro vblank puede causar que se vuelva a ejecutar,
	// generando un bucle infinito y corrupcion de stack.
	cc.CargarRegConst8(REG_A, 0x1);
	cc.AlmacenarRegEnMemoriaW(REG_A, HW_NMITIMEN);

	// podemos ejecutar NMI?
	cc.CargarRegEnMemoriaW(REG_A, WRAM_FLAG_EJECUCION);
	cc.BranchLong("FINALIZAR_NMI", BRANCH_ZERO_SET);
	cc.AlmacenarCeroEnMemoriaW(WRAM_FLAG_EJECUCION);
	cc.Etiqueta("NMI_EJECUCION");

	// TO-DO: codigo de NMI (configuracion de video)

	// rescatar estado de CPU, volver a ejecucion normal
	cc.Etiqueta("FINALIZAR_NMI");

	cc.CargarRegEnMemoriaW(REG_A, HW_RDNMI); // Leer flag de NMI para evitar que el interrupt se ejecute de inmediato
	cc.CargarRegConst8(REG_A, 0x81);		 // Activar NMI + Auto joypad read
	cc.AlmacenarRegEnMemoriaW(REG_A, HW_NMITIMEN);

	cc.LimpiarFlags(FLAG_X | FLAG_M);
	cc.Sacar(REG_Y);
	cc.Sacar(REG_X);
	cc.Sacar(REG_A);
	cc.Sacar(REG_BANK);
	cc.Sacar(REG_FLAGS);
	cc.ReturnInterrupt();
}

// La rutina de IRQ se ejecuta cuando el hardware genera una interrupcion enmascarable (IRQ).
// No es usada ahora mismo.
void RutinaIRQ() {
	cc.Etiqueta("I_IRQ");
	cc.ReturnInterrupt();
}

void EnsamblarROM() {
	cc.SetearPC(0x000000);
	RutinaRESET();
	RutinaNMI();
	RutinaIRQ();

	// Finalizar ROM
	GenerarHeader();
	cc.ResolverReferencias();
	GenerarChecksum();
}