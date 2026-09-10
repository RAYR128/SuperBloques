#include "asm/op.h"
#include "datos.h"
#include "ensamblador.h"
#include "instancia.h"
#include <cstring>
#include <stdexcept>

// Compartido
void ConRegALlamarJumpTable(std::string tabla) {
	// X = A * 3;
	cc.AlmacenarRegEnMemoria(REG_A, WRAM_SCRATCH);
	cc.ShiftALeft();
	cc.SumaAcumuladorMemoria(WRAM_SCRATCH);
	cc.Transferir(REG_A, REG_X);

	// Tenemos ahora el puntero a la tabla, usamos esto para conseguir el PC a ejecutar.
	// WRAM_POSICION_SALTO = tabla[X];
	cc.CargarRegEnMemoria_SymLX(REG_A, tabla);
	cc.AlmacenarRegEnMemoria(REG_A, WRAM_POSICION_SALTO);
	cc.IncrementarReg(REG_X);
	cc.CargarRegEnMemoria_SymLX(REG_A, tabla);
	cc.AlmacenarRegEnMemoria(REG_A, WRAM_POSICION_SALTO + 1);

	// Llamamos a la rutina dinamica.
	cc.LlamadaLong("CALL_DYNAMIC_POSITION");
}

// Rutinas
void RutinaLoopPrincipal() {
	// Activar modo 16-bit
	cc.LimpiarFlags(FLAG_X_8BIT | FLAG_M_8BIT);

	// TO-DO: Añadir latch (WRAM_CONTROL1_PRESIONADO)
	for(int i = 0; i < 2; i++) {
		int hwCntrl = HW_CNTRL1 + i * 2;
		int cntMask = WRAM_CONTROL1_MASK + i * 2;
		int cntIndex = WRAM_CONTROL1 + i * 4;
		cc.CargarRegEnMemoria(REG_A, hwCntrl);
		cc.AlmacenarRegEnMemoria(REG_A, cntIndex);
		cc.Transferir(REG_A, REG_Y);
		cc.EORAcumuladorMemoria(cntMask);
		cc.ANDAcumuladorMemoria(cntIndex);
		cc.AlmacenarRegEnMemoria(REG_A, cntIndex + 2);
		cc.AlmacenarRegEnMemoria(REG_Y, cntMask);
	}

	// Correr codigo main de escena
	cc.LlamadaLong("LLAMAR_ESCENA_ID");

	// Iterar por toda la memoria dedicada a objetos
	// Los objetos van a utilizar REG_Y de forma compartida!!
	cc.CargarRegConst16(REG_Y, 0);
	cc.Etiqueta("LOOP_CONTROL_OBJETOS");

	// Codigo para cada objeto
	// Primero chequeamos si existe
	cc.SetearFlags(FLAG_M_8BIT);
	cc.CargarRegEnMemoria_IndY(REG_A, WRAM_OBJETOS + PARAMETRO_OBJ_BHV_SCRIPT_STATUS);
	cc.LimpiarFlags(FLAG_M_8BIT);

	// No existe? Saltar este codigo.
	cc.Branch("LOOP_CONTROL_OBJETO_NO_EXISTE", BRANCH_ZERO_SET);

	// Tenemos que almacenar un puntero de 24-bit.
	cc.CargarRegEnMemoria_IndY(REG_A, WRAM_OBJETOS + PARAMETRO_OBJ_BHV_SCRIPT_POINTER);
	cc.AlmacenarRegEnMemoria(REG_A, WRAM_POSICION_SALTO);
	cc.CargarRegEnMemoria_IndY(REG_A, WRAM_OBJETOS + PARAMETRO_OBJ_BHV_SCRIPT_POINTER + 1);
	cc.AlmacenarRegEnMemoria(REG_A, WRAM_POSICION_SALTO + 1);

	// Llamamos al objeto ahora.
	cc.LlamadaLong("CALL_DYNAMIC_POSITION");
	cc.Etiqueta("LOOP_CONTROL_OBJETO_NO_EXISTE");

	// Iteramos hacia el siguiente objeto
	cc.Transferir(REG_Y, REG_A);
	cc.LimpiarFlags(FLAG_CARRYF);
	cc.SumaAcumuladorConst16(TAMANO_OBJETO);
	cc.Transferir(REG_A, REG_Y);
	cc.CompararRegConst16(REG_Y, CANTIDAD_DE_OBJETOS * TAMANO_OBJETO);
	cc.Branch("LOOP_CONTROL_OBJETOS", BRANCH_CARRY_CLEAR);

	// Incrementar WRAM_TIMER
	cc.IncrementarMemoria(WRAM_TIMER);

	// Desactivar modo 16-bit
	cc.SetearFlags(FLAG_X_8BIT | FLAG_M_8BIT);
}

void RutinaControlObjetos() {
	// Rutina de control: Crear objeto
	// TO-DO: definir como se implementaria esto. Tiene que existir algun parametro (probablemente el script bhv del objeto en WRAM_POSICION_SALTO)
	// para la inicializacion de este, y el estado a activar (PARAMETRO_OBJ_BHV_SCRIPT_STATUS), normalmente Estado 1
	cc.Etiqueta("OBJC_CREAR");
	cc.ReturnLong();
}

void RutinaControlEscena() {
	// Rutina de control: Inicializar escena
	cc.Etiqueta("INICIALIZAR_ESCENA_ID");

	// Desactivar modo 16-bit
	cc.SetearFlags(FLAG_X_8BIT | FLAG_M_8BIT);

	// Desactivar pantalla
	cc.CargarRegConst8(REG_A, 0x8F);
	cc.AlmacenarRegEnMemoria(REG_A, HW_INIDISP);

	// Activar modo 16-bit
	cc.LimpiarFlags(FLAG_X_8BIT | FLAG_M_8BIT);

	// TO-DO: Aqui tenemos que implementar el DMA a la VRAM para los recursos utilizados.

	// Ahora llamamos a la rutina de init de la escena actual.
	cc.CargarRegEnMemoria(REG_A, WRAM_ESCENA_ACTUAL);
	cc.ANDAcumuladorConst16(0x00FF);
	ConRegALlamarJumpTable("TABLA_SALTO_ESCENA_INIT");
	cc.ReturnLong();

	// Rutina de control: Llamar a escena
	cc.Etiqueta("LLAMAR_ESCENA_ID");
	cc.CargarRegEnMemoria(REG_A, WRAM_ESCENA_ACTUAL);
	cc.ANDAcumuladorConst16(0x00FF);
	ConRegALlamarJumpTable("TABLA_SALTO_ESCENA_MAIN");
	cc.ReturnLong();
}

void RutinaConfiguracionVideo() {
	// Paleta WRAM -> CGRAM (DMA ch0) y conversion de WRAM_V_COLDATA a COLDATA.
	cc.LimpiarFlags(FLAG_X_8BIT | FLAG_M_8BIT);

	cc.CargarRegConst16(REG_A, 0x0200);
	cc.AlmacenarRegEnMemoria(REG_A, HW_DMACNT);
	cc.CargarRegConst16(REG_A, ((HW_CGDATA & 0xFF) << 8) | HW_DMA_1Byte1Addr);
	cc.AlmacenarRegEnMemoria(REG_A, HW_DMAPARAM);
	cc.CargarRegConst16(REG_A, WRAM_PALETA);
	cc.AlmacenarRegEnMemoria(REG_A, HW_DMAADDR);

	cc.CargarRegEnMemoria(REG_A, WRAM_V_COLDATA);
	cc.ShiftALeft(3);
	cc.SetearFlags(FLAG_X_8BIT | FLAG_M_8BIT | FLAG_CARRYF);
	cc.RotarARight(3);
	cc.IntercambiarBytesA();
	cc.ORAcumuladorConst8(0x40);
	cc.AlmacenarRegEnMemoria(REG_A, HW_COLDATA);
	cc.CargarRegEnMemoria(REG_A, WRAM_V_COLDATA + 1);
	cc.ShiftARight();
	cc.SetearFlags(FLAG_CARRYF);
	cc.RotarARight();
	cc.AlmacenarRegEnMemoria(REG_A, HW_COLDATA);
	cc.IntercambiarBytesA();
	cc.AlmacenarRegEnMemoria(REG_A, HW_COLDATA);

	cc.AlmacenarCeroEnMemoria(HW_CGADD);
	cc.AlmacenarCeroEnMemoria(HW_DMAADDR + 2);
	cc.CargarRegConst8(REG_A, 1);
	cc.AlmacenarRegEnMemoria(REG_A, HW_MDMAEN);
}

// $FFDC-$FFDF (LoROM $7FDC-$7FDF): complemento + checksum de 16 bits.
// el checksum es la suma de todos los bytes de la ROM (se descarta overflow).
// Cualquier par checksum/complemento valido suma 0x1FE en esos 4 bytes, asi que
// se inicializan a $FFFF/$0000 antes de sumar para que el valor escrito coincida
// con la ROM final.
void GenerarChecksum() {
	DROM[0x7FDC] = 0xFF;
	DROM[0x7FDD] = 0xFF;
	DROM[0x7FDE] = 0x00;
	DROM[0x7FDF] = 0x00;

	uint32_t suma = 0;
	for(uint32_t i = 0; i < TAMANO_ROM; i++) {
		suma += DROM[i];
	}

	uint16_t checksum = (uint16_t)(suma & 0xFFFF);
	uint16_t complemento = checksum ^ 0xFFFF;

	DROM[0x7FDC] = (uint8_t)(complemento & 0xFF);
	DROM[0x7FDD] = (uint8_t)((complemento >> 8) & 0xFF);
	DROM[0x7FDE] = (uint8_t)(checksum & 0xFF);
	DROM[0x7FDF] = (uint8_t)((checksum >> 8) & 0xFF);
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
	cc.SetearFlags(FLAG_INTERR);
	cc.AlmacenarCeroEnMemoria(HW_NMITIMEN);
	cc.AlmacenarCeroEnMemoria(HW_HDMAEN);
	cc.AlmacenarCeroEnMemoria(HW_APUIO0);
	cc.AlmacenarCeroEnMemoria(HW_APUIO1);
	cc.AlmacenarCeroEnMemoria(HW_APUIO2);
	cc.AlmacenarCeroEnMemoria(HW_APUIO3);

	// desactivar la pantalla y configurar el registro de control de video
	cc.CargarRegConst8(REG_A, 0x8F);
	cc.AlmacenarRegEnMemoria(REG_A, HW_INIDISP);

	// CLC : XCE, desactivar emulacion de 6502 y activar modo nativo de 65816
	cc.LimpiarFlags(FLAG_CARRYF);
	cc.IntercambiarCarryConEmulacion();

	// REP #$38
	cc.LimpiarFlags(FLAG_X_8BIT | FLAG_M_8BIT | FLAG_DECIML);

	cc.CargarRegConst16(REG_A, WRAM_DIRECTPAGE);
	cc.Transferir(REG_A, REG_DP);
	cc.CargarRegConst16(REG_A, WRAM_STACK);
	cc.Transferir(REG_A, REG_STACK);

	// Limpiar toda la memoria
	cc.CargarRegConst16(REG_X, WRAM_SIZE - 2);
	cc.Etiqueta("LIMPIAR_MEMORIA");
	cc.AlmacenarCeroEnMemoria_IndX(0x0000);
	cc.DecrementarReg(REG_X);
	cc.DecrementarReg(REG_X);
	cc.Branch("LIMPIAR_MEMORIA", BRANCH_NEGATIVE_CLEAR);

	// Inicializar escena
	cc.LlamadaLong("INICIALIZAR_ESCENA_ID");

	// SEP #$30
	cc.SetearFlags(FLAG_X_8BIT | FLAG_M_8BIT);

	// Loop de programa
	cc.Etiqueta("PROGRAM_LOOP");

	// Chequear lectura de joypad
	cc.CargarRegEnMemoria(REG_A, HW_HVBJOY);
	cc.ShiftARight();
	cc.Branch("PROGRAM_LOOP", BRANCH_CARRY_SET);

	// Loop principal
	RutinaLoopPrincipal();

	// Esperar a que el hardware genere un VBlank, para sincronizar la logica con la pantalla
	cc.Etiqueta("PROGRAM_FINALIZAR_FRAME");
	cc.IncrementarMemoria(WRAM_FLAG_EJECUCION);
	cc.CargarRegEnMemoria(REG_A, HW_RDNMI); // Leer flag de NMI para evitar que el interrupt se ejecute de inmediato
	cc.CargarRegConst8(REG_A, 0x81);		// Activar NMI + Auto joypad read
	cc.AlmacenarRegEnMemoria(REG_A, HW_NMITIMEN);
	cc.EsperarInterrupcion(); // Esperar una interrupcion

	// Podemos ejecutar un nuevo cuadro?
	cc.Etiqueta("ESPERAR_BLANK");
	cc.CargarRegEnMemoria(REG_A, WRAM_FLAG_EJECUCION);
	cc.Branch("ESPERAR_BLANK", BRANCH_ZERO_CLEAR);

	// Repetir
	cc.Saltar("PROGRAM_LOOP", REF_ABSOLUTE);

	// Rutina de control: Llamar a codigo en WRAM_POSICION_SALTO
	cc.Etiqueta("CALL_DYNAMIC_POSITION");
	cc.SaltarLongIndirecto(WRAM_POSICION_SALTO);
}

// La rutina de NMI se ejecuta cuando el hardware genera una interrupcion no enmascarable (NMI).
// En este caso, el hardware genera un NMI cada vez que se produce un cambio de fotograma (VBlank),
// lo que permite al programa actualizar la pantalla y procesar la logica.
// Usamos la logica de VBlank para mantener una tasa de refresco constante y sincronizada con la pantalla.
void RutinaNMI() {
	cc.Etiqueta("I_NMI");

	// preservar estado de CPU durante interrupcion
	cc.Empujar(REG_FLAGS);
	cc.LimpiarFlags(FLAG_X_8BIT | FLAG_M_8BIT | FLAG_DECIML);
	cc.Empujar(REG_BANK);
	cc.Empujar(REG_A);
	cc.Empujar(REG_X);
	cc.Empujar(REG_Y);
	cc.SetearFlags(FLAG_X_8BIT | FLAG_M_8BIT);

	// desactivar el interrupt de vblank, dejar solo auto joypad read activado.
	// esto es para prevenir un bug en el cual si el NMI tarda demasiado en ejecutarse, otro vblank puede causar que se vuelva a ejecutar,
	// generando un bucle infinito y corrupcion de stack.
	cc.CargarRegConst8(REG_A, 0x1);
	cc.AlmacenarRegEnMemoria(REG_A, HW_NMITIMEN);

	// podemos ejecutar NMI?
	cc.CargarRegEnMemoria(REG_A, WRAM_FLAG_EJECUCION);
	cc.BranchLong("FINALIZAR_NMI", BRANCH_ZERO_SET);
	cc.AlmacenarCeroEnMemoria(WRAM_FLAG_EJECUCION);
	cc.Etiqueta("NMI_EJECUCION");

	// Ejecucion de codigo de NMI, configuracion de video
	cc.CargarRegConst8(REG_A, 0x8F);
	cc.AlmacenarRegEnMemoria(REG_A, HW_INIDISP);
	RutinaConfiguracionVideo();
	cc.CargarRegEnMemoria(REG_A, WRAM_V_BRILLO);
	cc.AlmacenarRegEnMemoria(REG_A, HW_INIDISP);

	// rescatar estado de CPU, volver a ejecucion normal
	cc.Etiqueta("FINALIZAR_NMI");

	cc.CargarRegEnMemoria(REG_A, HW_RDNMI); // Leer flag de NMI para evitar que el interrupt se ejecute de inmediato
	cc.CargarRegConst8(REG_A, 0x81);		// Activar NMI + Auto joypad read
	cc.AlmacenarRegEnMemoria(REG_A, HW_NMITIMEN);

	cc.LimpiarFlags(FLAG_X_8BIT | FLAG_M_8BIT);
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

void EmitirTablaEscena(string tabla, string nombre) {
	cc.Etiqueta(tabla);
	for(size_t i = 0; i < EscenasProyecto.size(); i++) {
		cc.EscribirEtiqueta(nombre + EscenasProyecto[i].Nombre, true);
	}
}

void EnsamblarROM() {
	cc.SetearPC(0x000000);
	RutinaRESET();
	RutinaControlObjetos();
	RutinaControlEscena();
	RutinaNMI();
	RutinaIRQ();

	// Tabla de salto de escena
	EmitirTablaEscena("TABLA_SALTO_ESCENA_INIT", "ESCENA_INIT");
	EmitirTablaEscena("TABLA_SALTO_ESCENA_MAIN", "ESCENA_MAIN");
	EmitirTablaEscena("TABLA_DATOS_GraficosPrincipales", "ESCENA_DATO_GraficosPrincipales");
	EmitirTablaEscena("TABLA_DATOS_GraficosHud", "ESCENA_DATO_GraficosHud");
	EmitirTablaEscena("TABLA_DATOS_Paleta", "ESCENA_DATO_Paleta");
	EmitirTablaEscena("TABLA_DATOS_Tilemap1", "ESCENA_DATO_Tilemap1");
	EmitirTablaEscena("TABLA_DATOS_Tilemap2", "ESCENA_DATO_Tilemap2");
	EmitirTablaEscena("TABLA_DATOS_Tilemap3", "ESCENA_DATO_Tilemap3");

	// Generar codigo de objetos
	for(size_t i = 0; i < ObjetosProyecto.size(); i++) {
		ObjetosProyecto[i].Compilar();
	}
	for(size_t i = 0; i < EscenasProyecto.size(); i++) {
		EscenasProyecto[i].Compilar();
	}

	if(cc.ObtenerPC() >= ((uint32_t)BANCO_DATOS_PRIMERO << 15)) {
		throw std::runtime_error("el codigo ocupa bancos de datos $10-$3F");
	}

	std::vector<BloqueDato> datos;
	datos.reserve(EscenasProyecto.size() * 6);
	for(const auto &escena : EscenasProyecto) {
		datos.push_back({"ESCENA_DATO_GraficosPrincipales" + escena.Nombre, escena.GraficosPrincipales, (uint32_t)sizeof(escena.GraficosPrincipales)});
		datos.push_back({"ESCENA_DATO_GraficosHud" + escena.Nombre, escena.GraficosHud, (uint32_t)sizeof(escena.GraficosHud)});
		datos.push_back({"ESCENA_DATO_Paleta" + escena.Nombre, escena.Paleta, (uint32_t)sizeof(escena.Paleta)});
		datos.push_back({"ESCENA_DATO_Tilemap1" + escena.Nombre, escena.Tilemap1, (uint32_t)sizeof(escena.Tilemap1)});
		datos.push_back({"ESCENA_DATO_Tilemap2" + escena.Nombre, escena.Tilemap2, (uint32_t)sizeof(escena.Tilemap2)});
		datos.push_back({"ESCENA_DATO_Tilemap3" + escena.Nombre, escena.Tilemap3, (uint32_t)sizeof(escena.Tilemap3)});
	}
	EmpaquetarDatosROM(std::move(datos));

	// Finalizar ROM
	GenerarHeader();
	cc.ResolverReferencias();
	GenerarChecksum();
	cc.GuardarSimbolosArchivo("salida.sym");
	cc.GuardarSimbolosArchivo("salida.cpu.sym");
}