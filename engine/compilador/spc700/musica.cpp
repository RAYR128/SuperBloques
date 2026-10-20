// Generacion de datos de musica para el SPC700
#include "asm/op.h"
#include "spc700.h"
#include <algorithm>
#include <stdexcept>

std::vector<InstrumentoMusica> InstrumentosProyecto;
std::vector<SonidoProyecto> SonidosProyecto;
std::vector<MusicaProyecto> MusicasProyecto;

void EnsamblarDatosMusicaSPC700(int Posicion, std::vector<BloqueDato> &Datos) {
	/* stub temporal
	db $C0,$44,$44,$44,$44,$44,$44,$44,$44
	db $C0,$44,$44,$44,$44,$44,$44,$44,$44
	db $C0,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
	db $C3,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
	*/
	SonidosProyecto.clear();
	// 4 bloques de sonido, esto produce un square wave 50%
	SonidoProyecto s;
	s.Nombre = "SquareWave";
	s.Datos = {
		0xC0, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, // fase 1
		0xC0, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, // fase 2
		0xC0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // fase 3
		0xC3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF // fase 4
	};
	s.PuntoLoop = 0;
	SonidosProyecto.push_back(s);

	// ptrMusica es donde iran los datos de la musica cuando sea generada
	int ptrMusica = Posicion;

	// La tabla SRCN es una tabla conteniendo 2 pointers LE para cada sample
	std::vector<uint8_t> SRCN;
	std::vector<uint8_t> DatosSamples;
	for(size_t sample = 0; sample < SonidosProyecto.size(); sample++) {
		const SonidoProyecto &snd = SonidosProyecto[sample];
		if(snd.Datos.size() % 9 != 0) {
			throw std::runtime_error("Sonido " + snd.Nombre + " tiene datos BRR invalidos");
		}
		int comienzoSample = ptrMusica;
		int puntoLoop = ptrMusica + snd.PuntoLoop;
		SRCN.push_back(comienzoSample & 0xFF);
		SRCN.push_back((comienzoSample >> 8) & 0xFF);
		SRCN.push_back(puntoLoop & 0xFF);
		SRCN.push_back((puntoLoop >> 8) & 0xFF);
		DatosSamples.insert(DatosSamples.end(), snd.Datos.begin(), snd.Datos.end());
		ptrMusica += snd.Datos.size();
	}
	// El SRCN va en 0x100
	uint8_t cabeceraSRCN[4] = {
		(uint8_t)(SRCN.size() & 0xFF),
		(uint8_t)((SRCN.size() >> 8) & 0xFF),
		0x00,
		0x01,
	};
	// Los samples van en Posicion
	uint8_t cabeceraSamples[4] = {
		(uint8_t)(DatosSamples.size() & 0xFF),
		(uint8_t)((DatosSamples.size() >> 8) & 0xFF),
		(uint8_t)(Posicion & 0xFF),
		(uint8_t)((Posicion >> 8) & 0xFF),
	};
	SRCN.insert(SRCN.begin(), cabeceraSRCN, cabeceraSRCN + 4);
	DatosSamples.insert(DatosSamples.begin(), cabeceraSamples, cabeceraSamples + 4);
	Datos.push_back({"DRIVER_AUDIO_SRCN", SRCN.data(), (uint32_t)SRCN.size()});
	Datos.push_back({"DRIVER_AUDIO_SAMPLES", DatosSamples.data(), (uint32_t)DatosSamples.size()});
	if(ptrMusica > 0xFFFF) {
		throw std::runtime_error("Los datos de sonido superan el limite de 64KB");
	}

	// ptrMusica ahora tiene la ubicacion de la musica (despues de samples)
}