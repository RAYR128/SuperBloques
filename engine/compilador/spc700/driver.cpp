// Rutinas de audio para comunicacion entre la S-CPU y el SPC700
#include "asm/op.h"
#include "driverspc_fuente.h"
#include "spc700.h"

// Sube un bloque al SPC700 con el protocolo del IPL. D debe ser 0 (lo deja RutinaRESET). PHP/PLP devuelven P. A, X e Y quedan sucios.
// WRAM_SPC_PTR_BLOQUE ($00-$02): puntero de 24 bits al bloque
//   +0 uint16 tamaño, +2 uint16 direccion ARAM, +4 datos.
// WRAM_SPC_ARAM_SALTO ($03-$04): direccion ARAM a la que el IPL salta al terminar.
// WRAM_SPC_TAMANO ($05-$06): la rutina escribe el tamaño leído.
// Esos bytes coinciden con WRAM_SCRATCH. El caller no puede estar a mitad del evaluador de expresiones.
// Y indexa el bloque en 16 bits (tope 65532 bytes de datos).
// https://wiki.superfamicom.org/spc700-reference#ipl-rom-1567
void CrearCodigoDriverSPC700(std::vector<BloqueDato> &Datos) {
	// engine/datos/driverspc.asm, embebido para que el build WASM no necesite filesystem.
	cc.Etiqueta("DRIVER_AUDIO");
	std::string codigoDriver(reinterpret_cast<const char *>(kDriverSpcFuente), kDriverSpcFuenteLen);
	int DatosMusica = EnsamblarProgramaSPC700(codigoDriver);
	(void)DatosMusica;

	// Codigo de driver de audio
	cc.Etiqueta("PUNTEROS_AUDIO");
	cc.EscribirEtiqueta("DRIVER_AUDIO", true);

	// Rutina de subida de datos al SPC700
	cc.Etiqueta("SUBIR_DATOS_SPC700");
	cc.Empujar(REG_FLAGS);
	cc.LimpiarFlags(FLAG_M_8BIT | FLAG_X_8BIT);
	cc.CargarRegConst16(REG_Y, 0x0000);
	cc.CargarRegConst16(REG_A, 0xBBAA);
	cc.Etiqueta("SUBIR_DATOS_SPC700_ESPERAR_BBAA");
	cc.CompararAcumuladorMemoria(HW_APUIO0);
	cc.Branch("SUBIR_DATOS_SPC700_ESPERAR_BBAA", BRANCH_ZERO_CLEAR);

	cc.SetearFlags(FLAG_M_8BIT);
	cc.CargarRegConst8(REG_A, 0xCC);
	cc.Empujar(REG_A);
	cc.LimpiarFlags(FLAG_M_8BIT);
	cc.CargarAcumuladorIndirectoLargo_IndY(WRAM_SPC_PTR_BLOQUE);
	cc.AlmacenarRegEnMemoria(REG_A, WRAM_SPC_TAMANO);
	cc.IncrementarReg(REG_Y);
	cc.IncrementarReg(REG_Y);
	cc.Transferir(REG_A, REG_X);
	cc.CargarAcumuladorIndirectoLargo_IndY(WRAM_SPC_PTR_BLOQUE);
	cc.IncrementarReg(REG_Y);
	cc.IncrementarReg(REG_Y);
	cc.AlmacenarRegEnMemoria(REG_A, HW_APUIO2);

	cc.SetearFlags(FLAG_M_8BIT);
	// X sigue en 16 bits. El carry queda activo si el tamaño no es 0.
	cc.CompararRegConst16(REG_X, 0x0001);
	cc.CargarRegConst8(REG_A, 0x00);
	cc.RotarALeft();
	cc.AlmacenarRegEnMemoria(REG_A, HW_APUIO1);
	// Sin CLC: el carry del CPX hace que ADC #$7F levante V cuando hay datos.
	cc.SumaAcumuladorConst8(0x7F);
	cc.Sacar(REG_A);
	cc.AlmacenarRegEnMemoria(REG_A, HW_APUIO0);
	cc.Etiqueta("SUBIR_DATOS_SPC700_ESPERAR_CC");
	cc.CompararAcumuladorMemoria(HW_APUIO0);
	cc.Branch("SUBIR_DATOS_SPC700_ESPERAR_CC", BRANCH_ZERO_CLEAR);
	cc.Branch("SUBIR_DATOS_SPC700_BYTES", BRANCH_OVERFLOW_SET);
	cc.Sacar(REG_FLAGS);
	cc.ReturnLong();

	cc.Etiqueta("SUBIR_DATOS_SPC700_BYTES");
	cc.CargarAcumuladorIndirectoLargo_IndY(WRAM_SPC_PTR_BLOQUE);
	cc.IncrementarReg(REG_Y);
	cc.IntercambiarBytesA();
	cc.CargarRegConst8(REG_A, 0x00);
	cc.Branch("SUBIR_DATOS_SPC700_GUARDAR", BRANCH_ALWAYS);

	cc.Etiqueta("SUBIR_DATOS_SPC700_SIGUIENTE");
	cc.IntercambiarBytesA();
	cc.CargarAcumuladorIndirectoLargo_IndY(WRAM_SPC_PTR_BLOQUE);
	cc.IncrementarReg(REG_Y);
	cc.IntercambiarBytesA();
	cc.Etiqueta("SUBIR_DATOS_SPC700_ESPERAR_INDICE");
	cc.CompararAcumuladorMemoria(HW_APUIO0);
	cc.Branch("SUBIR_DATOS_SPC700_ESPERAR_INDICE", BRANCH_ZERO_CLEAR);
	cc.IncrementarReg(REG_A);

	cc.Etiqueta("SUBIR_DATOS_SPC700_GUARDAR");
	cc.LimpiarFlags(FLAG_M_8BIT);
	cc.AlmacenarRegEnMemoria(REG_A, HW_APUIO0);
	cc.SetearFlags(FLAG_M_8BIT);
	cc.DecrementarReg(REG_X);
	cc.Branch("SUBIR_DATOS_SPC700_SIGUIENTE", BRANCH_ZERO_CLEAR);

	cc.Etiqueta("SUBIR_DATOS_SPC700_ESPERAR_ULTIMO");
	cc.CompararAcumuladorMemoria(HW_APUIO0);
	cc.Branch("SUBIR_DATOS_SPC700_ESPERAR_ULTIMO", BRANCH_ZERO_CLEAR);
	// El CMP igual dejo el carry. ADC #$03 avanza el indice, si da 0, se repite
	cc.Etiqueta("SUBIR_DATOS_SPC700_CERRAR");
	cc.SumaAcumuladorConst8(0x03);
	cc.Branch("SUBIR_DATOS_SPC700_CERRAR", BRANCH_ZERO_SET);

	cc.Etiqueta("SUBIR_DATOS_SPC700_SALTAR");
	cc.LimpiarFlags(FLAG_M_8BIT);
	cc.CargarRegEnMemoria(REG_A, WRAM_SPC_ARAM_SALTO);
	cc.AlmacenarRegEnMemoria(REG_A, HW_APUIO2);
	cc.CargarRegEnMemoria(REG_A, WRAM_SPC_TAMANO);
	cc.IncrementarReg(REG_A);
	cc.ANDAcumuladorConst16(0x00FF);
	cc.AlmacenarRegEnMemoria(REG_A, HW_APUIO0);
	cc.SetearFlags(FLAG_M_8BIT);
	cc.Etiqueta("SUBIR_DATOS_SPC700_ESPERAR_SALTO");
	cc.CompararAcumuladorMemoria(HW_APUIO0);
	cc.Branch("SUBIR_DATOS_SPC700_ESPERAR_SALTO", BRANCH_ZERO_CLEAR);
	cc.Sacar(REG_FLAGS);
	cc.ReturnLong();
}

void RutinaInicializarAudioSPC700() {
    cc.Etiqueta("INICIALIZACION_SPC");
    cc.CargarRegConst8(REG_X, 0);

    // Poner puntero all audio
	cc.LimpiarFlags(FLAG_M_8BIT);
	cc.CargarRegEnMemoria_SymLX(REG_A, "PUNTEROS_AUDIO");
	cc.AlmacenarRegEnMemoria(REG_A, WRAM_SPC_PTR_BLOQUE);
	cc.IncrementarReg(REG_X);
	cc.CargarRegEnMemoria_SymLX(REG_A, "PUNTEROS_AUDIO");
	cc.AlmacenarRegEnMemoria(REG_A, WRAM_SPC_PTR_BLOQUE + 1);

    // Saltar a 0x100 (inicio del programa)
    // TO-DO: Deberiamos sacar esto del programa si mismo, enves de tenerlo aqui hardcodeado
    cc.CargarRegConst16(REG_A, 0x100);
    cc.AlmacenarRegEnMemoria(REG_A, WRAM_SPC_ARAM_SALTO);
	cc.SetearFlags(FLAG_M_8BIT);

    cc.LlamadaLong("SUBIR_DATOS_SPC700");
    cc.Etiqueta("INICIALIZACION_SPC_FINALIZADA");
}