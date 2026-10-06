#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "datos.h"

// Ensambla codigo SPC700 y lo escribe en el PC actual de la ROM. Cada base/org emite un bloque little-endian:
//  0x00-0x01: tamaño de los datos (0 para indicar fin)
//  0x02-0x03: direccion de carga en ARAM
//  0x04-...: datos
// Devuelve la direccion ARAM final del ultimo bloque (direccion de carga + tamaño).
// Si no hay datos, devuelve 0.
int EnsamblarProgramaSPC700(std::string &Codigo);

// Ensamblar codigo de driver para subida de audio
void CrearCodigoDriverSPC700(std::vector<BloqueDato> &Datos);

// Rutina de inicializacion de audio
void RutinaInicializarAudioSPC700();

// Comandos para SFX / Musica
enum ComandosAudio : uint8_t {
    SB_ACMD_FINAL_DE_CANAL = 0x00,
    SB_ACMD_DURACION = 0x01, // 0x01-0x7F
    SB_ACMD_NOTA = 0x80, // 0x80-0xEF, se le resta 0x80
    
    // Notas de percusion, es el equivalente de SB_ACMD_INSTRUMENTO - 0xF0 + C-4
    SB_ACMD_PERC_1 = 0xF0,
    SB_ACMD_PERC_2 = 0xF1,
    SB_ACMD_PERC_3 = 0xF2,
    SB_ACMD_PERC_4 = 0xF3,
    SB_ACMD_PERC_5 = 0xF4,
    SB_ACMD_PERC_6 = 0xF5,
    SB_ACMD_PERC_7 = 0xF6,
    SB_ACMD_PERC_8 = 0xF7,
    SB_ACMD_PERC_9 = 0xF8,
    SB_ACMD_PERC_10 = 0xF9,
    SB_ACMD_PERC_11 = 0xFA,
    SB_ACMD_PERC_12 = 0xFB,

    // Comandos de control de audio
    SB_ACMD_INSTRUMENTO = 0xFC, // 2 bytes, 0xFC + 0x00-0xFF = instrumento
    SB_ACMD_CONTROL = 0xFD, // 2...+ bytes, 0xFD + 0x00-0xFF + ....
    SB_ACMD_SILENCIO = 0xFE,
    SB_ACMD_REST = 0xFF
};

// Clases para almacenar los datos de audio del proyecto
class SonidoProyecto {
public:
    std::string Nombre;
    std::vector<uint8_t> Datos;
};

class MusicaProyecto {
public:
    std::string Nombre;
    std::vector<uint8_t> DatosCanal[8];
};

extern std::vector<SonidoProyecto> SonidosProyecto;
extern std::vector<MusicaProyecto> MusicasProyecto;